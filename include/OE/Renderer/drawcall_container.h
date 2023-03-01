#ifndef DRAWCALLCONTAINER_H
#define DRAWCALLCONTAINER_H

#include <OE/Renderer/exceptions_nre.h>
#include <iterator>
#include <set>

namespace nre {
    struct render_group;

    class drawcall_container_t {
    public:
        drawcall_container_t();
        ~drawcall_container_t();



        class Iterator {
        public:
            typedef std::set<render_group>::iterator set_iter_t;

            Iterator(set_iter_t);

            Iterator&    operator++();
            Iterator     operator++(int);
            render_group operator*();


            using iterator_category = std::input_iterator_tag;
            using difference_type   = int;

            friend bool operator==(const Iterator& a, const Iterator& b);
            friend bool operator!=(const Iterator& a, const Iterator& b);

        private:
            set_iter_t iter;
        };

        Iterator begin();

        Iterator end();

        bool contains(const render_group&);
        void insert(render_group);
        void replace(render_group);
        void update();
        void cleanupPrograms();

        void removeCamera(std::size_t);
        void removeMaterial(std::size_t);
        void removeMesh(std::size_t);
        void removeVertexGroup(std::size_t, std::size_t);

        const std::set<render_group> deleted();

    private:
        std::set<render_group> data_;
        std::set<render_group> pending_rengroups_;
        std::set<render_group> to_be_deleted_;
    };

}; // namespace nre

#endif
