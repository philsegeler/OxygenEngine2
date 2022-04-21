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
 * Supports input iterators. Everything apart from iterators is 100% thread-safe.
 * The difference between this and oe::shared_index_map is that
 *  - No pending elements, elements are added directly
 *  - Registered is renamed to Registered and an event can come more than once
 */

namespace oe {
    template <typename T>
    class event_container_t : public OE_THREAD_SAFETY_OBJECT {
    protected:
        std::unordered_map<std::size_t, std::shared_ptr<T>> elements_;
        std::set<std::string>                               names_;

        void register_event(const std::size_t& index) {
            lockMutex();
            this->registered_.add(index);
            unlockMutex();
        }

        void clear_internally() {

            for (auto x : elements_) {
                this->deleted_.add(x.first);
            }
            elements_.clear();
            registered_.clear();

            names_.clear();
            id2name_.clear();
        }

    public:
        std::unordered_map<std::size_t, std::string> id2name_;
        OE_Name2ID                                   name2id;

        friend class Registered;
        friend class Element;

        event_container_t() : registered_(*this), deleted_(*this) {
            lockMutex();
            this->name2id = OE_Name2ID(&this->id2name_);
            unlockMutex();
        };

        event_container_t(const event_container_t&) = delete;

        ~event_container_t() {
        }

        std::size_t size() {
            lockMutex();
            std::size_t output = this->elements_.size();
            unlockMutex();
            return output;
        }

        //*******************************************/
        // interfacing class

        class Element {
        public:
            Element() {
            }
            Element(event_container_t<T>* db, std::size_t index, std::shared_ptr<T> element)
                : id_(index), p_(element), db_(db) {
            }

            std::size_t        id_{0};
            std::shared_ptr<T> p_{nullptr};

            void flag_as_registered() {
                db_->register_event(this->id_);
            }

            bool is_valid() {
                return p_ != nullptr;
            }

            std::string get_name() {
                return db_->get_name(id_);
            }

        protected:
            event_container_t<T>* db_;
        };

        //*******************************************/
        // methods

        void extend(event_container_t<T>& other, bool override_names) {
            lockMutex();

            other.synchronize(false);

            if (!override_names) {
                for (auto x : other) {
                    this->appendUNSAFE(x.get_name(), x.p_);
                }
            }
            else {
                for (auto x : other) {
                    this->force_appendUNSAFE(x.get_name(), x.p_);
                }
            }

            unlockMutex();
        }

        // TODO: Add referemce count
        int count(std::size_t index) {
            int output = 0;
            lockMutex();
            if (this->elements_.count(index) == 1) {
                output = 1;
            }
            unlockMutex();
            return output;
        }


        // TODO: Add referemce count
        int count(const std::string& name) {
            int output = 0;
            lockMutex();
            if (this->names_.count(name) == 1) {
                output = 1;
            }
            unlockMutex();
            return output;
        }

        std::string get_name(const std::size_t& index) {
            if (id2name_.count(index) != 0) return id2name_[index];
            return "";
        }

        void append(const std::string& name, std::shared_ptr<T> element) {
            lockMutex();
            this->appendUNSAFE(name, element);
            unlockMutex();
        }

        void appendUNSAFE(const std::string& name, std::shared_ptr<T> element) {
            if ((this->count(element->id) == 0)) {
                this->elements_[element->id] = element;
                this->id2name_[element->id]  = name;
                this->names_.insert(name);
            }
            else {
                OE_Warn("Element with ID: '" + std::to_string(element->id) + "' and name: '" + name +
                        "' already exists in SharedIndexMap<" + typeid(T).name() + ">.");
            }
        }

        void force_append(const std::string& name, std::shared_ptr<T> element) {
            lockMutex();
            this->force_appendUNSAFE(name, element);
            unlockMutex();
        }

        void force_appendUNSAFE(const std::string& name, std::shared_ptr<T> element) {
            if (names_.count(name) == 1) {
                auto prev_id = this->name2id[name];
                this->elements_.erase(prev_id);
                this->id2name_.erase(prev_id);
                registered_.remove(prev_id);
                deleted_.add(prev_id);
            }
            this->elements_[element->id] = element;
            this->id2name_[element->id]  = name;
            this->registered_.add(element->id);
            this->names_.insert(name);
        }

        std::string to_str() {
            lockMutex();
            std::string output = "[\n";
            for (auto x : elements_) {
                output.append(id2name_[x.first] + " ; " + std::to_string(x.first));
                output.append("\n");
            }
            output.append("]");
            unlockMutex();
            return output;
        }

        void reset_registered() {
            lockMutex();
            this->reset_registeredUNSAFE();
            unlockMutex();
        }

        void reset_registeredUNSAFE() {
            this->registered_.clear();
        }

        void synchronize(bool clear_all) noexcept {

            lockMutex();

            this->reset_registeredUNSAFE();

            for (auto x : deleted_.indices_) {
                if (elements_.count(x) == 0) continue;
                elements_.erase(x);
                names_.erase(id2name_[x]);
                id2name_.erase(x);
            }

            deleted_.clear();

            if (clear_all) this->clear_internally();
            unlockMutex();
        }

        Element operator[](const std::size_t& index) noexcept {

            auto output = Element();

            lockMutex();
            if (elements_.count(index) != 0)
                output = Element(this, index, elements_[index]);
            else
                output = Element(this, index, nullptr);
            unlockMutex();

            if (!output.is_valid()) {
                OE_Warn("Element with ID: '" + std::to_string(output.id_) + "' does not exist in SharedIndexMap<" +
                        typeid(T).name() + ">.");
            }

            return output;
        }

        Element operator[](const std::string& name) noexcept {

            auto output = Element();

            lockMutex();
            if (this->names_.count(name) != 0) {
                size_t elem_id = name2id[name];
                output         = Element(this, elem_id, elements_[elem_id]);
            }
            else
                output = Element(this, name2id[name], nullptr);
            unlockMutex();

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
            lockMutex();
            this->deleted_.add(index);
            unlockMutex();
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
            return Iterator(*this, this->elements_.begin());
        }

        Iterator end() {
            return Iterator(*this, this->elements_.end());
        }

        //*******************************************/
        // Registered class for storing all element indices that registered the previous frame

        class Registered {
        public:
            // Registered(){}
            Registered(event_container_t<T>& inputa) : db_(inputa) {
            }

            void add(const std::size_t& index) {
                if ((db_.elements_.count(index) != 0) && (db_.deleted_.count(index) == 0)) indices_.push_back(index);
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

            //*******************************************/
            // Registered iterator for getting the registered events in the right direction
            class RegisteredIter {
            public:
                typedef std::vector<std::size_t>::iterator vector_iter_t;

                using iterator_category = std::input_iterator_tag;
                using difference_type   = int;

                RegisteredIter(event_container_t<T>& db, vector_iter_t beginning) : iter(beginning), db_(db) {
                }

                RegisteredIter& operator++() {
                    iter++;
                    return *this;
                }
                RegisteredIter operator++(int) {
                    RegisteredIter tmp = *this;
                    ++(*this);
                    return tmp;
                }

                // This needs robust error handling in multiple threads
                Element operator*() {
                    return db_[*iter];
                }

                friend bool operator==(const RegisteredIter& a, const RegisteredIter& b) {
                    return a.iter == b.iter;
                };
                friend bool operator!=(const RegisteredIter& a, const RegisteredIter& b) {
                    return a.iter != b.iter;
                };

            protected:
                vector_iter_t         iter;
                event_container_t<T>& db_;
            };

            RegisteredIter begin() {
                return RegisteredIter(this->db_, this->indices_.begin());
            }

            RegisteredIter end() {
                return RegisteredIter(this->db_, this->indices_.end());
            }

            event_container_t<T>&    db_;
            std::vector<std::size_t> indices_;
        };

        Registered registered_;

        Registered registered() {
            return this->registered_;
        }

        bool has_registered_events() {
            lockMutex();
            bool output = not this->registered_.empty();
            unlockMutex();
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
                if (db_.elements_.count(index) != 0) {
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

            //*******************************************/
            // Deleted iterator for getting only the objects that registered
            class DeletedIter {
            public:
                typedef std::set<std::size_t, std::greater<std::size_t>>::iterator set_iter_t;

                using iterator_category = std::input_iterator_tag;
                using difference_type   = int;

                DeletedIter(event_container_t<T>& db, set_iter_t beginning) : iter(beginning), db_(db) {
                }

                DeletedIter& operator++() {
                    iter++;
                    return *this;
                }
                DeletedIter operator++(int) {
                    DeletedIter tmp = *this;
                    ++(*this);
                    return tmp;
                }

                Element operator*() {
                    return db_[*iter];
                }

                friend bool operator==(const DeletedIter& a, const DeletedIter& b) {
                    return a.iter == b.iter;
                };
                friend bool operator!=(const DeletedIter& a, const DeletedIter& b) {
                    return a.iter != b.iter;
                };

            protected:
                set_iter_t            iter;
                event_container_t<T>& db_;
            };

            DeletedIter begin() {
                return DeletedIter(this->db_, this->indices_.begin());
            }

            DeletedIter end() {
                return DeletedIter(this->db_, this->indices_.end());
            }

            event_container_t<T>&                            db_;
            std::set<std::size_t, std::greater<std::size_t>> indices_;
        };

        Deleted deleted_;

        Deleted deleted() {
            return this->deleted_;
        }
    };
}; // namespace oe
#endif
