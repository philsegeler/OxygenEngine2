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


/** New General class intended to optimize and properly parallelize accesing of
 * individual meshes/materials/etc. Stores ids and names. Only stores one name per element and one id per element.
 * Supports input iterators. Everything apart from iterators is 100% thread-safe.
 * [Enhancement TODO for Andreas]: Add a custom container in place of std::set in Changed/Deleted
 */

template <typename T>
class OE_SharedIndexMap : public OE_THREAD_SAFETY_OBJECT {
public:
    friend class Changed;
    friend class Element;
    friend class OE_TaskManager;

    OE_SharedIndexMap() : changed_(*this), deleted_(*this){};

    OE_SharedIndexMap(const OE_SharedIndexMap&) = delete;


    //*******************************************/
    // interfacing class

    class Element {
        friend class OE_SharedIndexMap;

    public:
        Element() {
        }
        Element(OE_SharedIndexMap<T>* db, std::size_t index, std::shared_ptr<T> element) : p_(element), db_(db) {
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

        void flag_as_changed() {
            db_->change(this->id());
        }

        bool is_valid() {
            return p_ != nullptr;
        }

        std::string get_name() {
            return db_->get_name(this->id());
        }

    private:
        std::shared_ptr<T>    p_{nullptr};
        OE_SharedIndexMap<T>* db_;
    };

    //*******************************************/
    // methods

    std::size_t size() {
        lockMutex();
        std::size_t output = this->elements_container_.size();
        unlockMutex();
        return output;
    }

    void extend(OE_SharedIndexMap<T>& other, bool override_names) {
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
        if (this->elements_container_.count(index) == 1) {
            output = 1;
        }
        unlockMutex();
        return output;
    }

    bool is_pending(std::size_t index) {
        int output = 0;
        lockMutex();
        if (this->pending_elements_container_.count(index) == 1) {
            output = 1;
        }
        unlockMutex();
        return output;
    }

    // TODO: Add referemce count
    int count(const std::string& name) {
        int output = 0;
        lockMutex();
        if (this->name2id_container_.count(name) == 1) {
            output = 1;
        }
        unlockMutex();
        return output;
    }

    bool is_pending(const std::string& name) {
        int output = 0;
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

    std::string get_name(const std::size_t& index) {
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

    Element operator[](const std::size_t& index) noexcept {

        auto output = Element();

        lockMutex();
        if (elements_container_.count(index) != 0)
            output = Element(this, index, elements_container_[index]);
        else
            output = Element(this, index, nullptr);
        unlockMutex();

        if (!output.is_valid()) {
            OE_Warn("Element with ID: '" + std::to_string(output.id()) + "' does not exist in SharedIndexMap<" +
                    typeid(T).name() + ">.");
        }

        return output;
    }

    Element operator[](const std::string& name) noexcept {

        auto output = Element();

        lockMutex();
        if (this->name2id_container_.contains(name)) {
            size_t elem_id = name2id_container_[name];
            output         = Element(this, elem_id, elements_container_[elem_id]);
        }
        else
            output = Element(this, 0, nullptr);
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

        Iterator(OE_SharedIndexMap<T>& db, map_iter_t beginning) : iter(beginning), db_(db) {
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
        OE_SharedIndexMap<T>& db_{nullptr};
    };

    Iterator begin() {
        return Iterator(*this, this->elements_container_.begin());
    }

    Iterator end() {
        return Iterator(*this, this->elements_container_.end());
    }

    //*******************************************/
    // Changed class for storing all element indices that changed the previous frame

    class Changed {
    public:
        // Changed(){}
        Changed(OE_SharedIndexMap<T>& inputa) : db_(inputa) {
        }

        void add(const std::size_t& index) {
            if ((db_.elements_container_.count(index) != 0) && (db_.deleted_.count(index) == 0))
                indices_container_.insert(index);
        }

        void remove(const std::size_t& index) {
            if (indices_container_.count(index) != 0) {
                indices_container_.erase(index);
            }
        }

        void clear() {
            this->indices_container_.clear();
        }

        //*******************************************/
        // Changed iterator for getting only the objects that changed
        class ChangedIter {
        public:
            typedef std::set<std::size_t, std::greater<std::size_t>>::iterator set_iter_t;

            using iterator_category = std::input_iterator_tag;
            using difference_type   = int;

            ChangedIter(OE_SharedIndexMap<T>& db, set_iter_t beginning) : iter(beginning), db_(db) {
            }

            ChangedIter& operator++() {
                iter++;
                return *this;
            }
            ChangedIter operator++(int) {
                ChangedIter tmp = *this;
                ++(*this);
                return tmp;
            }

            // This needs robust error handling in multiple threads
            Element operator*() {
                return db_[*iter];
            }

            friend bool operator==(const ChangedIter& a, const ChangedIter& b) {
                return a.iter == b.iter;
            };
            friend bool operator!=(const ChangedIter& a, const ChangedIter& b) {
                return a.iter != b.iter;
            };

        protected:
            set_iter_t            iter;
            OE_SharedIndexMap<T>& db_;
        };

        ChangedIter begin() const {
            return ChangedIter(this->db_, this->indices_container_.begin());
        }

        ChangedIter end() const {
            return ChangedIter(this->db_, this->indices_container_.end());
        }

    private:
        OE_SharedIndexMap<T>&                            db_;
        std::set<std::size_t, std::greater<std::size_t>> indices_container_;
    };

    const Changed& changed() {
        return this->changed_;
    }

    //*******************************************/
    // Deleted class for storing all element indices that changed the previous frame

    class Deleted {
        friend class OE_SharedIndexMap;

    public:
        // Deleted(){}
        Deleted(OE_SharedIndexMap<T>& inputa) : db_(inputa) {
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

        void clear() {
            this->indices_container_.clear();
        }

    private:
        OE_SharedIndexMap<T>&                            db_;
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

    Changed changed_;
    Deleted deleted_;

    void change(const std::size_t& index) {
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
    }

    void reset_changed() {
        this->changed_.clear();
    }

    void synchronize(bool clear_all) noexcept {

        lockMutex();

        this->reset_changed();

        for (auto x : deleted_.indices_container_) {
            if (elements_container_.count(x) == 0) continue;
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
                changed_.remove(this->name2id_container_[pending_id2name_container_[x.first]]);
                deleted_.add(this->name2id_container_[pending_id2name_container_[x.first]]);
                this->elements_container_.erase(this->name2id_container_[pending_id2name_container_[x.first]]);
                this->id2name_container_.erase(this->name2id_container_[pending_id2name_container_[x.first]]);
            }

            // std::cout << "Pending element: " << x.first << " " << pending_id2name_container_[x.first] << (x.second ==
            // nullptr) << std::endl;
            elements_container_[x.first]                            = x.second;
            id2name_container_[x.first]                             = pending_id2name_container_[x.first];
            name2id_container_[pending_id2name_container_[x.first]] = x.first;
            changed_.add(x.first);
        }
        pending_elements_container_.clear();
        pending_id2name_container_.clear();
        unlockMutex();
    }

    void appendUNSAFE(const std::string& name, std::shared_ptr<T> element) {
        if ((this->count(element->id) == 0) && (this->pending_elements_container_.count(element->id) == 0)) {
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
    }
};

#endif
