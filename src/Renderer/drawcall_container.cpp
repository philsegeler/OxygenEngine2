#include <OE/Renderer/renderer_utils.h>
#include <OE/math_oe.h>


nre::drawcall_container_t::drawcall_container_t() {
    this->data_ = std::set<nre::render_group>();
}

nre::drawcall_container_t::~drawcall_container_t() {
}

nre::drawcall_container_t::Iterator::Iterator(set_iter_t beginning) : iter(beginning) {
}

nre::drawcall_container_t::Iterator& nre::drawcall_container_t::Iterator::operator++() {
    iter++;
    return *this;
}
nre::drawcall_container_t::Iterator nre::drawcall_container_t::Iterator::operator++(int) {
    nre::drawcall_container_t::Iterator tmp = *this;
    ++(*this);
    return tmp;
}
nre::render_group nre::drawcall_container_t::Iterator::operator*() {
    return *iter;
}
namespace nre {
    bool operator==(const nre::drawcall_container_t::Iterator& a, const nre::drawcall_container_t::Iterator& b) {
        return a.iter == b.iter;
    };
    bool operator!=(const nre::drawcall_container_t::Iterator& a, const nre::drawcall_container_t::Iterator& b) {
        return a.iter != b.iter;
    };
}; // namespace nre
nre::drawcall_container_t::Iterator nre::drawcall_container_t::begin() {
    return Iterator(this->data_.begin());
}

nre::drawcall_container_t::Iterator nre::drawcall_container_t::end() {
    return Iterator(this->data_.end());
}

bool nre::drawcall_container_t::contains(const nre::render_group& ren_group) {
    return this->data_.contains(ren_group);
}

void nre::drawcall_container_t::insert(nre::render_group ren_group) {
    this->data_.insert(ren_group);
}

void nre::drawcall_container_t::replace(nre::render_group ren_group) {
    this->pending_rengroups_.insert(ren_group);
}

void nre::drawcall_container_t::update() {

    for (auto ren_group : this->pending_rengroups_) {
        if (this->contains(ren_group)) {

            this->data_.erase(ren_group);
            this->data_.insert(ren_group);
        }
    }
    this->pending_rengroups_.clear();
}

const std::set<nre::render_group> nre::drawcall_container_t::deleted() {
    return to_be_deleted_;
}

void nre::drawcall_container_t::cleanupPrograms() {
    for (auto ren_group : to_be_deleted_) {
        this->data_.erase(ren_group);
    }
    this->to_be_deleted_.clear();
}

void nre::drawcall_container_t::removeCamera(std::size_t cam) {

    for (auto ren_group : this->data_) {
        if (ren_group.camera == cam) {
            to_be_deleted_.insert(ren_group);
        }
    }
}

void nre::drawcall_container_t::removeMaterial(std::size_t mat) {

    for (auto ren_group : this->data_) {
        if (ren_group.material == mat) {
            to_be_deleted_.insert(ren_group);
        }
    }
}
void nre::drawcall_container_t::removeMesh(std::size_t mesh) {

    for (auto ren_group : this->data_) {
        if (ren_group.mesh == mesh) {
            to_be_deleted_.insert(ren_group);
        }
    }
}
void nre::drawcall_container_t::removeVertexGroup(std::size_t mesh, std::size_t vgroup) {

    for (auto ren_group : this->data_) {
        if (ren_group.vgroup == vgroup) {
            to_be_deleted_.insert(ren_group);
        }
    }
}
