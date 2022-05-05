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
 *  - Changed is renamed to Registered and an event can come more than once
 *  - No mutexes, this is totally thread-unsafe, but it is guarded by the event handler mutex
 */

namespace oe {
    template <typename T>
    class event_container_t : public OE_THREAD_SAFETY_OBJECT {
    public:
        friend class Registered;
        friend class Element;

        event_container_t()
            : registered_(*this), deleted_(*this){

                                  };

        event_container_t(const event_container_t&) = delete;

        ~event_container_t() {
        }

        std::size_t size() {

            std::size_t output = this->elements_container_.size();

            return output;
        }

        //*******************************************/
        // interfacing class

        class Element {
        public:
            Element() {
            }
            Element(event_container_t<T>* db, std::size_t index, std::shared_ptr<T> element) : p_(element), db_(db) {
            }

            auto operator->() {
                return p_.operator->();
            }

            std::shared_ptr<T> pointer() {
                return p_;
            }

            std::size_t id() {
                return p_->id;
            }

            void flag_as_registered() {
                db_->register_event(this->id());
            }

            bool is_valid() {
                return p_ != nullptr;
            }

            std::string get_name() {
                return db_->get_name(p_->id);
            }

        private:
            std::shared_ptr<T>    p_{nullptr};
            event_container_t<T>* db_;
        };

        //*******************************************/
        // methods

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
        int count(std::size_t index) {
            int output = 0;

            if (this->elements_container_.count(index) == 1) {
                output = 1;
            }

            return output;
        }


        // TODO: Add referemce count
        int count(const std::string& name) {
            int output = 0;

            if (this->name2id_container_.count(name) == 1) {
                output = 1;
            }

            return output;
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
            if ((this->count(element->id) == 0)) {
                this->elements_container_[element->id] = element;
                this->id2name_container_[element->id]  = name;
                this->name2id_container_[name]         = element->id;
            }
            else {
                OE_Warn("Element with ID: '" + std::to_string(element->id) + "' and name: '" + name +
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
            this->registered_.add(element->id);
            this->name2id_container_[name] = element->id;
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

        void reset_registered() {
            this->registered_.clear();
        }

        void synchronize(bool clear_all) noexcept {



            this->reset_registered();

            for (auto x : deleted_.indices_) {
                if (elements_container_.count(x) == 0) continue;
                elements_container_.erase(x);
                name2id_container_.erase(id2name_container_[x]);
                id2name_container_.erase(x);
            }

            deleted_.clear();

            if (clear_all) this->clear_internally();
        }

        Element operator[](const std::size_t& index) noexcept {

            auto output = Element();


            if (elements_container_.contains(index))
                output = Element(this, index, elements_container_[index]);
            else
                output = Element(this, index, nullptr);


            if (!output.is_valid()) {
                OE_Warn("Element with ID: '" + std::to_string(output.id()) + "' does not exist in SharedIndexMap<" +
                        typeid(T).name() + ">.");
            }

            return output;
        }

        Element operator[](const std::string& name) noexcept {

            auto output = Element();


            if (this->name2id_container_.contains(name)) {
                size_t elem_id = name2id_container_[name];
                output         = Element(this, elem_id, elements_container_[elem_id]);
            }
            else
                output = Element(this, 0, nullptr);


            if (!output.is_valid()) {
                OE_Warn("Element with name: '" + name + "' does not exist in SharedIndexMap<" + typeid(T).name() + ">.");
            }

            return output;
        }

        Element at(const std::size_t& index) {

            auto output = this[0][index];

            if (!output.is_valid()) {
                throw oe::invalid_element_id("SharedIndexMap<" + std::string(typeid(T).name()) + ">", index);
            }

            return output;
        }

        Element at(const std::string& name) {

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

        class Iterator {
        public:
            typedef typename std::unordered_map<std::size_t, std::shared_ptr<T>>::iterator map_iter_t;
            typedef typename std::pair<std::size_t, std::shared_ptr<T>>                    map_iter_element_t;

            using iterator_category = std::input_iterator_tag;
            using difference_type   = int;

            Iterator(event_container_t<T>& db, map_iter_t beginning) : iter(beginning), db_(db) {
            }

            Iterator& operator++() {
                iter++;
                return *this;
            }
            Iterator operator++(int) {
                Iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            Element operator*() {
                return db_[(*iter).first];
            }

            friend bool operator==(const Iterator& a, const Iterator& b) {
                return a.iter == b.iter;
            };
            friend bool operator!=(const Iterator& a, const Iterator& b) {
                return a.iter != b.iter;
            };

        private:
            map_iter_t            iter;
            event_container_t<T>& db_{nullptr};
        };

        Iterator begin() {
            return Iterator(*this, this->elements_container_.begin());
        }

        Iterator end() {
            return Iterator(*this, this->elements_container_.end());
        }

        //*******************************************/
        // Registered class for storing all element indices that registered the previous frame

        class Registered {
        public:
            // Registered(){}
            Registered(event_container_t<T>& inputa) : db_(inputa) {
            }

            void add(const std::size_t& index) {
                if ((db_.elements_container_.count(index) != 0) && (db_.deleted_.count(index) == 0)) indices_.push_back(index);
            }

            void remove(const std::size_t& index) {
                if (std::count(indices_.begin(), indices_.end(), index) != 0) {
                    std::erase(indices_, index);
                }
            }

            bool empty() {
                return this->indices_.empty();
            }

            void clear() {
                this->indices_.clear();
            }

            typedef std::vector<std::size_t>::iterator vector_iter_t;

            vector_iter_t begin() {
                return this->indices_.begin();
            }

            vector_iter_t end() {
                return this->indices_.end();
            }

            event_container_t<T>&    db_;
            std::vector<std::size_t> indices_;
        };



        Registered& registered() {
            return this->registered_;
        }

        const std::vector<std::size_t>& get_all_registered() {
            return this->registered_.indices_;
        }

        void register_event(const std::size_t& index) {

            this->registered_.add(index);
        }

        bool has_registered_events() {

            bool output = not this->registered_.empty();

            return output;
        }

        //*******************************************/
        // Deleted class for storing all element indices that registered the previous frame

        class Deleted {
        public:
            // Deleted(){}
            Deleted(event_container_t<T>& inputa) : db_(inputa) {
            }

            void add(const std::size_t& index) {
                indices_.insert(index);
                if (db_.elements_container_.count(index) != 0) {
                    db_.registered_.remove(index);
                }
            }

            void remove(const std::size_t& index) {
                if (indices_.count(index) != 0) {
                    indices_.erase(index);
                }
            }

            int count(const std::size_t& index) {
                return this->indices_.count(index);
            }

            void clear() {
                this->indices_.clear();
            }

            typedef std::set<std::size_t, std::greater<std::size_t>>::iterator set_iter_t;

            set_iter_t begin() {
                return this->indices_.begin();
            }

            set_iter_t end() {
                return this->indices_.end();
            }

            event_container_t<T>&                            db_;
            std::set<std::size_t, std::greater<std::size_t>> indices_;
        };

        Deleted& deleted() {
            return this->deleted_;
        }

        std::vector<std::size_t> get_all_deleted() {
            return this->deleted_.indices_;
        }

    protected:
        std::unordered_map<std::size_t, std::shared_ptr<T>> elements_container_;
        std::unordered_map<std::string, std::size_t>        name2id_container_;

        std::unordered_map<std::size_t, std::string> id2name_container_;


        Registered registered_;

        Deleted deleted_;


        void clear_internally() {

            for (auto x : elements_container_) {
                this->deleted_.add(x.first);
            }
            elements_container_.clear();
            registered_.clear();

            name2id_container_.clear();
            id2name_container_.clear();
        }
    };
}; // namespace oe
#endif
