#pragma once
#include "engine/engine.hpp"
#include "engine/entityManager/component/components.hpp"
#include "engine/scheduleManager/task/task.hpp"



class FireBall{
  static constexpr float projectileSpeed = 100; 
  EntityId light=UINT32_MAX;
  EntityId id=UINT32_MAX;
  Engine& engine;
  vec2 velocity = {0,0};
  int currFrame=0;
  TaskId animationTask=UINT32_MAX;
  void startAnimation();
  void updatePositionFromVelocity(double dt);
  void checkCollisions();


  public:
    bool destroyed=false;
    FireBall(Engine& e,vec2 pos,vec2 moveDir);
    FireBall(FireBall&& other) noexcept;
    FireBall& operator=(FireBall&& other) noexcept;
    ~FireBall();
    void update(double dt);
};
