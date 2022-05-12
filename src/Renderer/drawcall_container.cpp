#include <OE/Renderer/renderer_utils.h>
#include <OE/math_oe.h>


NRE_DrawCallContainer::NRE_DrawCallContainer() {
    this->data_ = std::set<NRE_RenderGroup>();
}

NRE_DrawCallContainer::~NRE_DrawCallContainer() {
}

NRE_DrawCallContainer::Iterator::Iterator(set_iter_t beginning) : iter(beginning) {
}

NRE_DrawCallContainer::Iterator& NRE_DrawCallContainer::Iterator::operator++() {
    iter++;
    return *this;
}
NRE_DrawCallContainer::Iterator NRE_DrawCallContainer::Iterator::operator++(int) {
    NRE_DrawCallContainer::Iterator tmp = *this;
    ++(*this);
    return tmp;
}
NRE_RenderGroup NRE_DrawCallContainer::Iterator::operator*() {
    return *iter;
}

bool operator==(const NRE_DrawCallContainer::Iterator& a, const NRE_DrawCallContainer::Iterator& b) {
    return a.iter == b.iter;
};
bool operator!=(const NRE_DrawCallContainer::Iterator& a, const NRE_DrawCallContainer::Iterator& b) {
    return a.iter != b.iter;
};

NRE_DrawCallContainer::Iterator NRE_DrawCallContainer::begin() {
    return Iterator(this->data_.begin());
}

NRE_DrawCallContainer::Iterator NRE_DrawCallContainer::end() {
    return Iterator(this->data_.end());
}

bool NRE_DrawCallContainer::contains(const NRE_RenderGroup& ren_group) {
    return this->data_.contains(ren_group);
}

void NRE_DrawCallContainer::insert(NRE_RenderGroup ren_group) {
    this->data_.insert(ren_group);
}

void NRE_DrawCallContainer::replace(NRE_RenderGroup ren_group) {
    this->pending_rengroups_.insert(ren_group);
}

void NRE_DrawCallContainer::update() {

    for (auto ren_group : this->pending_rengroups_) {
        if (this->contains(ren_group)) {

            this->data_.erase(ren_group);
            this->data_.insert(ren_group);
        }
    }
    this->pending_rengroups_.clear();
}

const std::set<NRE_RenderGroup> NRE_DrawCallContainer::deleted() {
    return to_be_deleted_;
}

void NRE_DrawCallContainer::cleanupPrograms() {
    for (auto ren_group : to_be_deleted_) {
        this->data_.erase(ren_group);
    }
    this->to_be_deleted_.clear();
}

void NRE_DrawCallContainer::removeCamera(std::size_t cam) {

    for (auto ren_group : this->data_) {
        if (ren_group.camera == cam) {
            to_be_deleted_.insert(ren_group);
        }
    }
}

void NRE_DrawCallContainer::removeMaterial(std::size_t mat) {

    for (auto ren_group : this->data_) {
        if (ren_group.material == mat) {
            to_be_deleted_.insert(ren_group);
        }
    }
}
void NRE_DrawCallContainer::removeMesh(std::size_t mesh) {

    for (auto ren_group : this->data_) {
        if (ren_group.mesh == mesh) {
            to_be_deleted_.insert(ren_group);
        }
    }
}
void NRE_DrawCallContainer::removeVertexGroup(std::size_t mesh, std::size_t vgroup) {

    for (auto ren_group : this->data_) {
        if (ren_group.vgroup == vgroup) {
            to_be_deleted_.insert(ren_group);
        }
    }
}
