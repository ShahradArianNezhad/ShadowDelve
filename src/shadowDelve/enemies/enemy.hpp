#pragma once
#include "engine/engine.hpp"
#include "engine/scheduleManager/task/task.hpp"
#include "utilities/types.hpp"
#include "utils/types.hpp"

class EnemyEntity{
  protected:
    int health=50;
    EntityId id=UINT32_MAX,collider=UINT32_MAX;
    TaskId moveTask;
    Engine& engine;
    vec2 velocity{0,0};
    void applyVelocity(vec2 v);
    void updateFacingDirection();
    vec2 getRandomMove();
    bool isTileWalkable(vec2 tileCords);
    std::vector<vec2> findPath(vec2 position);
  public:
    EnemyEntity(Engine& e);
    virtual void init()=0;
    virtual void update(double dt)=0;
};
