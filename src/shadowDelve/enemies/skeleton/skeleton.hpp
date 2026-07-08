#pragma once
#include "shadowDelve/enemies/enemy.hpp"



class Skeleton : public EnemyEntity{
  protected:
    enum class MODE{
      IDLE,
      WALK,
      CHASE,
      DAMAGED,
      ATTACK,
      DEATH
    };
    float attackCooldown = 1.5;
    bool canAttack = true;
    void init(){};
    void chasePlayer(double dt);
    void go(vec2 pos);
    void makeRandomMove();
    void attack();
    void roamToGoal(double dt);
    double getDist(vec2 start,vec2 end);
    int currFrame=0;
    vec2 goal{0,0};
    MODE mode=MODE::ATTACK;
    bool locked=false;
    TaskId animationTask=UINT32_MAX;
    static constexpr int attackRange = 34;
    static constexpr int roamSpeed = 30;
    static constexpr int chaseSpeed = 60;
    virtual void setMode(MODE mode)=0;
    virtual void update(double dt)=0;

  public:
    Skeleton(Engine& e);

};

