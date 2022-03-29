#ifndef COLISSION_ENGINE_H_
#define COLISSION_ENGINE_H_

#include "FE_World.h"
#include "HE_Types.h"

class Collision_Engine {
public:
    ~Collision_Engine();

private:
    AABB generateAABB(FE_Mesh* mesh);
};

#endif
