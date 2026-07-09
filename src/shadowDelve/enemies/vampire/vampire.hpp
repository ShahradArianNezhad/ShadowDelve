#pragma once
#include "shadowDelve/enemies/enemy.hpp"



class Vampire : public EnemyEntity{
  protected:
    enum class MODE{
      IDLE,
      WALK,
      CHASE,
      DAMAGED,
      ATTACK,
      DEATH,
      FALL
    };
    float attackCooldown = 5;
    bool canAttack = true;
    void init()override{};
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
    bool decending=false;
    TaskId animationTask=UINT32_MAX;
    static constexpr int attackRange = 200;
    static constexpr int aggroRange = 800;
    static constexpr int roamSpeed = 30;
    static constexpr int chaseSpeed = 100;
    void setMode(MODE mode);
    bool canSeePlayer();
    void update(double dt) override;

  public:
    Vampire(vec2 pos,Engine& e);

};
