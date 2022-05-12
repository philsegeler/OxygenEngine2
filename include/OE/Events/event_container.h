#ifndef OE_EVENT_CONTAINER_H
#define OE_EVENT_CONTAINER_H


#include <OE/error_oe.h>
#include <OE/types/base_types.h>
#include <algorithm>
#include <iterator>
#include <map>
#include <memory>
#include <set>



/** New General class based on oe::shared_index_map_t intended to optimize and properly parallelize accesing of
 * individual events. Stores ids and names. Only stores one name per element and one id per element.
 * The difference between this and oe::shared_index_map is that
 *  - No pending elements, elements are added directly
 *  - Changed is renamed to registered_t and an event can come more than once
 *  - No mutexes, this is totally thread-unsafe, but it is guarded by the event handler mutex
 */

namespace oe {
    template <typename T>
    class event_container_t : public OE_THREAD_SAFETY_OBJECT {
    public:
        friend class registered_t;
        friend class element_t;
        friend class event_handler_t;

        event_container_t()
            : registered_(*this), deleted_(*this){

                                  };

        event_container_t(const event_container_t&) = delete;


        //*******************************************/
        // interfacing class

        class element_t {
        public:
            element_t() {
            }
            element_t(event_container_t<T>* db, std::shared_ptr<T> element) : p_(element), db_(db) {
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

            void flag_as_registered() const {
                db_->register_event(this->get_id());
            }

            bool is_valid() const {
                return p_ != nullptr;
            }

            std::string get_name() const {
                return db_->get_name(this->get_id());
            }

        private:
            std::shared_ptr<T>    p_{nullptr};
            event_container_t<T>* db_;
        };

        //*******************************************/
        // methods

        std::size_t size() {

            std::size_t output = this->elements_container_.size();

            return output;
        }

        void extend(event_container_t<T>& other, bool override_names) {


            other.synchronize(false);

            if (!override_names) {
                for (auto x : other) {
                    this->append(x.get_name(), x.p_);
                }
            }
            else {
                for (auto x : other) {
                    this->force_append(x.get_name(), x.p_);
                }
            }
        }

        // TODO: Add referemce count
        bool contains(std::size_t index) {
            return this->elements_container_.contains(index);
        }


        // TODO: Add referemce count
        bool contains(const std::string& name) {
            return this->name2id_container_.contains(name);
        }

        std::string get_name(const std::size_t& index) {
            if (id2name_container_.count(index) != 0) return id2name_container_[index];
            return "";
        }

        std::size_t get_id(const std::string& name) {
            if (this->name2id_container_.count(name) == 1) {
                return this->name2id_container_[name];
            }
            else {
                return 0;
            }
        }

        void append(const std::string& name, std::shared_ptr<T> element) {
            if ((this->contains(element->id) == 0) and not this->name2id_container_.contains(name)) {
                this->elements_container_[element->id] = element;
                this->id2name_container_[element->id]  = name;
                this->name2id_container_[name]         = element->id;
            }
            else {
                OE_Warn("element_t with ID: '" + std::to_string(element->id) + "' and name: '" + name +
                        "' already exists in SharedIndexMap<" + typeid(T).name() + ">.");
            }
        }

        void force_append(const std::string& name, std::shared_ptr<T> element) {
            if (name2id_container_.count(name) == 1) {
                size_t prev_id = this->name2id[name];
                this->elements_container_.erase(prev_id);
                this->id2name_container_.erase(prev_id);
                registered_.remove(prev_id);
                deleted_.add(prev_id);
            }
            this->elements_container_[element->id] = element;
            this->id2name_container_[element->id]  = name;
            this->name2id_container_[name]         = element->id;
        }

        std::string to_str() {

            std::string output = "[\n";
            for (auto x : elements_container_) {
                output.append(id2name_container_[x.first] + " ; " + std::to_string(x.first));
                output.append("\n");
            }
            output.append("]");

            return output;
        }

        const element_t operator[](const std::size_t& index) noexcept {

            auto output = element_t();


            if (elements_container_.contains(index))
                output = element_t(this, elements_container_[index]);
            else
                output = element_t(this, nullptr);


            if (!output.is_valid()) {
                OE_Warn("element_t with ID: '" + std::to_string(index) + "' does not exist in SharedIndexMap<" +
                        typeid(T).name() + ">.");
            }

            return output;
        }

        const element_t operator[](const std::string& name) noexcept {

            auto output = element_t();


            if (this->name2id_container_.contains(name)) {
                size_t elem_id = name2id_container_[name];
                output         = element_t(this, elements_container_[elem_id]);
            }
            else
                output = element_t(this, nullptr);


            if (!output.is_valid()) {
                OE_Warn("element_t with name: '" + name + "' does not exist in SharedIndexMap<" + typeid(T).name() + ">.");
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

            this->deleted_.add(index);
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

            iterator_t(event_container_t<T>& db, map_iter_t beginning) : iter(beginning), db_(db) {
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
                return element_t(&db_, (*iter).second);
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
            map_iter_t            iter;
            event_container_t<T>& db_{nullptr};
        };

        iterator_t begin() {
            return iterator_t(*this, this->elements_container_.begin());
        }

        iterator_t end() {
            return iterator_t(*this, this->elements_container_.end());
        }

        //*******************************************/
        // registered_t class for storing all element indices that registered the previous frame

        class registered_t {
            friend class event_container_t;

        public:
            // registered_t(){}
            registered_t(event_container_t<T>& inputa) : db_(inputa) {
            }

            void add(const std::size_t& index) {
                if ((db_.elements_container_.count(index) != 0) && (db_.deleted_.count(index) == 0))
                    indices_container_.push_back(index);
            }

            void remove(const std::size_t& index) {
                if (std::count(indices_container_.begin(), indices_container_.end(), index) != 0) {
                    std::erase(indices_container_, index);
                }
            }

            bool empty() {
                return this->indices_container_.empty();
            }

            void clear() {
                this->indices_container_.clear();
            }

            typedef std::vector<std::size_t>::iterator vector_iter_t;

            vector_iter_t begin() {
                return this->indices_container_.begin();
            }

            vector_iter_t end() {
                return this->indices_container_.end();
            }

        private:
            event_container_t<T>&    db_;
            std::vector<std::size_t> indices_container_;
        };



        const std::vector<std::size_t> registered() {
            return this->registered_.indices_container_;
        }

        void register_event(const std::size_t& index) {

            this->registered_.add(index);
        }

        bool has_registered_events() {

            bool output = not this->registered_.empty();

            return output;
        }

        //*******************************************/
        // deleted_t class for storing all element indices that registered the previous frame

        class deleted_t {
        public:
            // deleted_t(){}
            deleted_t(event_container_t<T>& inputa) : db_(inputa) {
            }

            void add(const std::size_t& index) {
                indices_container_.insert(index);
                if (db_.elements_container_.count(index) != 0) {
                    db_.registered_.remove(index);
                }
            }

            void remove(const std::size_t& index) {
                if (indices_container_.count(index) != 0) {
                    indices_container_.erase(index);
                }
            }

            int count(const std::size_t& index) {
                return this->indices_container_.count(index);
            }

            void clear() {
                this->indices_container_.clear();
            }

            event_container_t<T>&                            db_;
            std::set<std::size_t, std::greater<std::size_t>> indices_container_;
        };

        const std::set<std::size_t, std::greater<std::size_t>> deleted() {
            return this->deleted_.indices_container_;
        }


    private:
        std::unordered_map<std::size_t, std::shared_ptr<T>> elements_container_;
        std::unordered_map<std::string, std::size_t>        name2id_container_;

        std::unordered_map<std::size_t, std::string> id2name_container_;

        registered_t registered_;

        deleted_t deleted_;


        void clear_internally() {

            for (auto x : elements_container_) {
                this->deleted_.add(x.first);
            }
            elements_container_.clear();
            registered_.clear();

            name2id_container_.clear();
            id2name_container_.clear();
        }

        void reset_registered() {
            this->registered_.clear();
        }

        void synchronize(bool clear_all) noexcept {

            this->reset_registered();

            for (auto x : deleted_.indices_container_) {
                if (elements_container_.count(x) == 0) continue;
                elements_container_.erase(x);
                name2id_container_.erase(id2name_container_[x]);
                id2name_container_.erase(x);
            }

            deleted_.clear();

            if (clear_all) this->clear_internally();
        }
    };
}; // namespace oe
#endif
