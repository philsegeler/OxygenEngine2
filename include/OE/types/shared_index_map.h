#ifndef OE_SHARED_INDEX_MAP_H
#define OE_SHARED_INDEX_MAP_H


#include <OE/error_oe.h>
#include <OE/types/base_types.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <type_traits>


/** New General class intended to optimize and properly parallelize accesing of
 * individual meshes/materials/etc. Stores ids and names. Only stores one name per element and one id per element.
 * Supports input iterators. Everything apart from iterators is 100% thread-safe.
 * [Enhancement TODO for Andreas]: Add a custom container in place of std::set in changed_t/deleted_t
 */

class OE_TaskManager;

namespace oe {

    enum class index_map_sort_type { greater_than, less_than, unsorted };

    // Base Element class. Only contains element
    template <typename T>
    class shared_index_map_base_element_t {
        friend class element_t;

    public:
        virtual ~shared_index_map_base_element_t() {
        }

        //*******************************************/
        // interfacing class

        class element_t {

        public:
            element_t() {
            }
            element_t(shared_index_map_base_element_t<T>* db, std::shared_ptr<T> element) : p_(element), db_(db) {
            }

            auto operator->() const {
                return p_.operator->();
            }

            std::shared_ptr<T> get_pointer() const {
                return p_;
            }

            std::size_t get_id() const {
                if (p_ == nullptr)
                    return 0;
                else
                    return p_->id;
            }

            void flag_as_changed() const {
                db_->change(this->get_id());
            }

            bool is_valid() const {
                return p_ != nullptr;
            }

            std::string get_name() const {
                return db_->get_name(this->get_id());
            }

        private:
            std::shared_ptr<T>                  p_{nullptr};
            shared_index_map_base_element_t<T>* db_;
        };

    protected:
        virtual void        change(std::size_t)   = 0;
        virtual std::string get_name(std::size_t) = 0;
    };

    // Base class; Overload for sorted
    template <typename T, index_map_sort_type IndexMapType>
    class shared_index_map_base_t : public shared_index_map_base_element_t<T> {
    public:
        shared_index_map_base_t() : sorted_(this) {
        }

        class sorted_t {
        public:
            using element_t = typename shared_index_map_base_element_t<T>::element_t;

            sorted_t(shared_index_map_base_element_t<T>* db) : db_(db) {
                if constexpr (IndexMapType == index_map_sort_type::greater_than) {
                    auto lambda_cmp = [](std::shared_ptr<T> a, std::shared_ptr<T> b) { return *a > *b; };
                    set_ =
                        std::set<std::shared_ptr<T>, std::function<bool(std::shared_ptr<T>, std::shared_ptr<T>)>>(lambda_cmp);
                }
                else {
                    auto lambda_cmp = [](std::shared_ptr<T> a, std::shared_ptr<T> b) { return *a < *b; };
                    set_ =
                        std::set<std::shared_ptr<T>, std::function<bool(std::shared_ptr<T>, std::shared_ptr<T>)>>(lambda_cmp);
                }
            }

            void add(std::shared_ptr<T> element) {
                set_.insert(element);
            }

            void remove(std::shared_ptr<T> element) {
                if (set_.contains(element)) set_.erase(element);
            }

            void clear() {
                set_.clear();
            }

            class sorted_iter_t {
            public:
                using iterator_category = std::input_iterator_tag;
                using difference_type   = std::ptrdiff_t;
                using value_type        = element_t;
                using reference         = value_type&;
                using pointer           = value_type*;

                typedef
                    typename std::set<std::shared_ptr<T>, std::function<bool(std::shared_ptr<T>, std::shared_ptr<T>)>>::iterator
                        set_iter_t;

                sorted_iter_t(set_iter_t beginning, shared_index_map_base_element_t<T>* db) : iter(beginning), db_(db) {
                }

                sorted_iter_t& operator++() {
                    iter++;
                    return *this;
                }
                sorted_iter_t operator++(int) {
                    sorted_iter_t tmp = *this;
                    ++(*this);
                    return tmp;
                }

                // This needs robust error handling in multiple threads
                const element_t operator*() {
                    return element_t(db_, *iter);
                }

                friend bool operator==(const sorted_iter_t& a, const sorted_iter_t b) {
                    return a.iter == b.iter;
                };
                friend bool operator!=(const sorted_iter_t& a, const sorted_iter_t& b) {
                    return a.iter != b.iter;
                };

            private:
                set_iter_t                          iter;
                shared_index_map_base_element_t<T>* db_;
            };

            sorted_iter_t begin() const {
                return sorted_iter_t(this->set_.begin(), db_);
            }

            sorted_iter_t end() const {
                return sorted_iter_t(this->set_.end(), db_);
            }

        private:
            std::set<std::shared_ptr<T>, std::function<bool(std::shared_ptr<T>, std::shared_ptr<T>)>> set_;
            shared_index_map_base_element_t<T>*                                                       db_;
        };

        const sorted_t& sorted() {
            return sorted_;
        }

    protected:
        sorted_t sorted_;
    };

    // Base class; Overload for unsorted
    template <typename T>
    class shared_index_map_base_t<T, index_map_sort_type::unsorted> : public shared_index_map_base_element_t<T> {
    protected:
    };


    //*************************************************************************************/
    // Final class
    template <typename T, index_map_sort_type IndexMapType = index_map_sort_type::unsorted>
    class shared_index_map_t : public shared_index_map_base_t<T, IndexMapType>, public OE_THREAD_SAFETY_OBJECT {

        constexpr static bool is_sorted = (IndexMapType != index_map_sort_type::unsorted);

        friend class ::OE_TaskManager;
        friend class changed_t;
        friend class deleted_t;

    public:
        using element_t = typename shared_index_map_base_element_t<T>::element_t;

        shared_index_map_t() : changed_(*this), deleted_(*this){};

        shared_index_map_t(const shared_index_map_t&) = delete;


        //*******************************************/
        // methods

        std::size_t size() {
            lockMutex();
            std::size_t output = this->elements_container_.size();
            unlockMutex();
            return output;
        }

        void extend(shared_index_map_t<T>& other, bool override_names) {
            lockMutex();

            other.synchronize(false);

            if (!override_names) {
                for (auto x : other) {
                    this->appendUNSAFE(x.get_name(), x.get_pointer());
                }
            }
            else {
                for (auto x : other) {
                    this->force_appendUNSAFE(x.get_name(), x.get_pointer());
                }
            }

            unlockMutex();
        }

        // TODO: Add referemce count
        bool contains(std::size_t index) {
            bool output = 0;
            lockMutex();
            if (this->elements_container_.contains(index)) {
                output = 1;
            }
            unlockMutex();
            return output;
        }

        bool is_pending(std::size_t index) {
            bool output = 0;
            lockMutex();
            if (this->pending_elements_container_.contains(index)) {
                output = 1;
            }
            unlockMutex();
            return output;
        }

        // TODO: Add referemce count
        bool contains(const std::string& name) {
            bool output = 0;
            lockMutex();
            if (this->name2id_container_.contains(name)) {
                output = 1;
            }
            unlockMutex();
            return output;
        }

        // VERY slow, do not recommend
        bool is_pending(const std::string& name) {
            bool output = 0;
            lockMutex();
            for (auto x : this->pending_id2name_container_) {
                if (x.second == name) {
                    output = 1;
                    break;
                }
            }
            unlockMutex();
            return output;
        }

        std::string get_name(std::size_t index) {
            std::string output = "";
            lockMutex();
            if (id2name_container_.contains(index)) output = id2name_container_[index];
            unlockMutex();
            return output;
        }

        std::size_t get_id(const std::string& name) {
            std::size_t output;
            lockMutex();
            if (this->name2id_container_.contains(name)) {
                output = this->name2id_container_[name];
            }
            else {
                output = 0;
            }
            unlockMutex();
            return output;
        }

        void append(const std::string& name, std::shared_ptr<T> element) {
            lockMutex();
            this->appendUNSAFE(name, element);
            unlockMutex();
        }

        void force_append(const std::string& name, std::shared_ptr<T> element) {
            lockMutex();
            this->force_appendUNSAFE(name, element);
            unlockMutex();
        }

        void append_now(const std::string& name, std::shared_ptr<T> element) {
            lockMutex();
            this->appendUNSAFE_now(name, element);
            unlockMutex();
        }

        void force_append_now(const std::string& name, std::shared_ptr<T> element) {
            lockMutex();
            this->force_appendUNSAFE_now(name, element);
            unlockMutex();
        }

        std::string to_str() {
            lockMutex();
            std::string output = "[\n";
            for (auto x : elements_container_) {
                output.append(id2name_container_[x.first] + " ; " + std::to_string(x.first));
                output.append("\n");
            }
            output.append("]");
            unlockMutex();
            return output;
        }

        const element_t operator[](const std::size_t& index) noexcept {

            auto output = element_t();

            lockMutex();
            if (elements_container_.count(index) != 0)
                output = element_t(this, elements_container_[index]);
            else
                output = element_t(this, nullptr);
            unlockMutex();

            if (!output.is_valid()) {
                OE_Warn("Element with ID: '" + std::to_string(index) + "' does not exist in SharedIndexMap<" +
                        typeid(T).name() + ">.");
            }

            return output;
        }

        const element_t operator[](const std::string& name) noexcept {

            auto output = element_t();

            lockMutex();
            if (this->name2id_container_.contains(name)) {
                size_t elem_id = name2id_container_[name];
                output         = element_t(this, elements_container_[elem_id]);
            }
            else
                output = element_t(this, nullptr);
            unlockMutex();

            if (!output.is_valid()) {
                OE_Warn("Element with name: '" + name + "' does not exist in SharedIndexMap<" + typeid(T).name() + ">.");
            }

            return output;
        }

        const element_t at(const std::size_t& index) {

            auto output = this[0][index];

            if (!output.is_valid()) {
                throw oe::invalid_element_id("SharedIndexMap<" + std::string(typeid(T).name()) + ">", index);
            }

            return output;
        }

        const element_t at(const std::string& name) {

            auto output = this[0][name];

            if (!output.is_valid()) {
                throw oe::invalid_element_name("SharedIndexMap<" + std::string(typeid(T).name()) + ">", name);
            }

            return output;
        }

        void remove(const std::size_t& index) {
            lockMutex();
            this->deleted_.add(index);
            unlockMutex();
        }

        //*******************************************/
        // Regular iterator for interfacing ALL elements

        class iterator_t {
        public:
            typedef typename std::unordered_map<std::size_t, std::shared_ptr<T>>::iterator map_iter_t;
            typedef typename std::pair<std::size_t, std::shared_ptr<T>>                    map_iter_element_t;

            using iterator_category = std::input_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = element_t;
            using reference         = value_type&;
            using pointer           = value_type*;

            iterator_t(shared_index_map_t<T, IndexMapType>* db, map_iter_t beginning) : iter(beginning), db_(db) {
            }

            iterator_t& operator++() {
                iter++;
                return *this;
            }
            iterator_t operator++(int) {
                iterator_t tmp = *this;
                ++(*this);
                return tmp;
            }

            const element_t operator*() {
                return element_t(db_, (*iter).second);
            }

            const element_t* operator->() const {
                return &element_t(db_, (*iter).second);
            }

            friend bool operator==(const iterator_t& a, const iterator_t& b) {
                return a.iter == b.iter;
            };
            friend bool operator!=(const iterator_t& a, const iterator_t& b) {
                return a.iter != b.iter;
            };

        private:
            map_iter_t                           iter;
            shared_index_map_t<T, IndexMapType>* db_{nullptr};
        };

        iterator_t begin() {
            return iterator_t(this, this->elements_container_.begin());
        }

        iterator_t end() {
            return iterator_t(this, this->elements_container_.end());
        }

        //*******************************************/
        // changed_t class for storing all element indices that changed the previous frame

        class changed_t {
        public:
            // changed_t(){}
            changed_t(shared_index_map_t<T, IndexMapType>& inputa) : db_(inputa) {
            }

            void add(const std::size_t& index) {
                if ((db_.elements_container_.contains(index)) and (not db_.deleted_.contains(index)))
                    indices_container_.insert(index);
            }

            void remove(const std::size_t& index) {
                if (indices_container_.contains(index)) {
                    indices_container_.erase(index);
                }
            }

            void clear() {
                this->indices_container_.clear();
            }

            //*******************************************/
            // changed_t iterator for getting only the objects that changed
            class changed_iter_t {
            public:
                typedef std::set<std::size_t, std::greater<std::size_t>>::iterator set_iter_t;

                using iterator_category = std::input_iterator_tag;
                using difference_type   = std::ptrdiff_t;
                using value_type        = element_t;
                using reference         = value_type&;
                using pointer           = value_type*;

                changed_iter_t(shared_index_map_t<T>& db, set_iter_t beginning) : iter(beginning), db_(db) {
                }

                changed_iter_t& operator++() {
                    iter++;
                    return *this;
                }
                changed_iter_t operator++(int) {
                    changed_iter_t tmp = *this;
                    ++(*this);
                    return tmp;
                }

                // This needs robust error handling in multiple threads
                const element_t operator*() {
                    return db_[*iter];
                }

                const element_t* operator->() {
                    return &db_[*iter];
                }

                friend bool operator==(const changed_iter_t& a, const changed_iter_t& b) {
                    return a.iter == b.iter;
                };
                friend bool operator!=(const changed_iter_t& a, const changed_iter_t& b) {
                    return a.iter != b.iter;
                };

            protected:
                set_iter_t                           iter;
                shared_index_map_t<T, IndexMapType>& db_;
            };

            changed_iter_t begin() const {
                return changed_iter_t(this->db_, this->indices_container_.begin());
            }

            changed_iter_t end() const {
                return changed_iter_t(this->db_, this->indices_container_.end());
            }

        private:
            shared_index_map_t<T, IndexMapType>&             db_;
            std::set<std::size_t, std::greater<std::size_t>> indices_container_;
        };

        const changed_t& changed() {
            return this->changed_;
        }

        //*******************************************/
        // deleted_t class for storing all element indices that changed the previous frame

        class deleted_t {
            friend class shared_index_map_t;

        public:
            // deleted_t(){}
            deleted_t(shared_index_map_t<T, IndexMapType>& inputa) : db_(inputa) {
            }

            void add(const std::size_t& index) {
                indices_container_.insert(index);
                db_.changed_.remove(index);
            }

            void remove(const std::size_t& index) {
                if (indices_container_.count(index) != 0) {
                    indices_container_.erase(index);
                }
            }

            int count(const std::size_t& index) const {
                return this->indices_container_.count(index);
            }

            bool contains(std::size_t index) const {
                return this->indices_container_.contains(index);
            }

            void clear() {
                this->indices_container_.clear();
            }

        private:
            shared_index_map_t<T, IndexMapType>&             db_;
            std::set<std::size_t, std::greater<std::size_t>> indices_container_;
        };



        const std::set<std::size_t, std::greater<std::size_t>>& deleted() {
            return this->deleted_.indices_container_;
        }

    private:
        std::unordered_map<std::size_t, std::shared_ptr<T>> elements_container_;

        std::map<std::size_t, std::shared_ptr<T>>    pending_elements_container_;
        std::unordered_map<std::size_t, std::string> pending_id2name_container_;

        std::unordered_map<std::size_t, std::string> id2name_container_;
        std::unordered_map<std::string, std::size_t> name2id_container_;

        changed_t changed_;
        deleted_t deleted_;

        void change(std::size_t index) {
            lockMutex();
            this->changed_.add(index);
            unlockMutex();
        }

        void clear_internally() {

            for (auto x : elements_container_) {
                this->deleted_.add(x.first);
            }
            elements_container_.clear();
            changed_.clear();

            name2id_container_.clear();
            id2name_container_.clear();

            if constexpr (is_sorted) {
                this->sorted_.clear();
            }
        }

        void reset_changed() {
            this->changed_.clear();
        }

        void synchronize(bool clear_all) noexcept {

            lockMutex();

            this->reset_changed();

            for (auto x : deleted_.indices_container_) {
                if (elements_container_.count(x) == 0) continue;
                if constexpr (is_sorted) {
                    this->sorted_.remove(elements_container_[x]);
                }
                elements_container_.erase(x);
                name2id_container_.erase(id2name_container_[x]);
                id2name_container_.erase(x);
            }

            deleted_.clear();

            if (clear_all) this->clear_internally();

            elements_container_.reserve(elements_container_.size() + pending_elements_container_.size());
            id2name_container_.reserve(id2name_container_.size() + pending_elements_container_.size());

            for (auto x : pending_elements_container_) {

                // if the element already exists, that means that it was added with force_append*
                // That means that the old element should be deleted.
                // There can never be two elements with the same name
                if (name2id_container_.contains(pending_id2name_container_[x.first])) {
                    std::size_t existing_id = this->name2id_container_[pending_id2name_container_[x.first]];
                    changed_.remove(existing_id);
                    deleted_.add(existing_id);
                    if constexpr (is_sorted) {
                        this->sorted_.remove(this->elements_container_[existing_id]);
                    }
                    this->elements_container_.erase(existing_id);
                    this->id2name_container_.erase(existing_id);
                }

                // std::cout << "Pending element: " << x.first << " " << pending_id2name_container_[x.first] << (x.second ==
                // nullptr) << std::endl;
                elements_container_[x.first] = x.second;
                if constexpr (is_sorted) {
                    this->sorted_.add(x.second);
                }
                id2name_container_[x.first]                             = pending_id2name_container_[x.first];
                name2id_container_[pending_id2name_container_[x.first]] = x.first;
                changed_.add(x.first);
            }
            pending_elements_container_.clear();
            pending_id2name_container_.clear();
            unlockMutex();
        }

        void appendUNSAFE(const std::string& name, std::shared_ptr<T> element) {
            if ((!this->contains(element->id)) && (this->pending_elements_container_.count(element->id) == 0)) {
                this->pending_elements_container_[element->id] = element;
                this->pending_id2name_container_[element->id]  = name;
            }
            else {
                OE_Warn("Element with ID: '" + std::to_string(element->id) + "' and name: '" + name +
                        "' already exists in SharedIndexMap<" + typeid(T).name() + ">.");
            }
        }

        void appendUNSAFE_now(const std::string& name, std::shared_ptr<T> element) {
            if ((this->elements_container_.count(element->id) == 0) &&
                (this->pending_elements_container_.count(element->id) == 0) && (this->name2id_container_.count(name) == 0)) {
                this->elements_container_[element->id] = element;
                this->id2name_container_[element->id]  = name;
                this->name2id_container_[name]         = element->id;
                if constexpr (is_sorted) {
                    this->sorted_.add(element);
                }
                this->changed_.add(element->id);
            }
            else {
                OE_Warn("Element with ID: '" + std::to_string(element->id) + "' and name: '" + name +
                        "' already exists in SharedIndexMap<" + typeid(T).name() + ">.");
            }
        }

        void force_appendUNSAFE(const std::string& name, std::shared_ptr<T> element) {
            this->pending_elements_container_[element->id] = element;
            this->pending_id2name_container_[element->id]  = name;
        }

        void force_appendUNSAFE_now(const std::string& name, std::shared_ptr<T> element) {
            if (name2id_container_.contains(name)) {
                size_t prev_id = this->name2id_container_[name];
                if constexpr (is_sorted) {
                    this->sorted_.remove(this->elements_container_[prev_id]);
                }
                this->elements_container_.erase(prev_id);
                this->id2name_container_.erase(prev_id);
                name2id_container_.erase(name);
                changed_.remove(prev_id);
                deleted_.add(prev_id);
            }
            this->elements_container_[element->id] = element;
            this->id2name_container_[element->id]  = name;
            this->changed_.add(element->id);
            this->name2id_container_[name] = element->id;
            if constexpr (is_sorted) {
                this->sorted_.add(element);
            }
        }
    };
}; // namespace oe
#endif
