#pragma once
#include "shadowDelve/enemies/enemy.hpp"
#include "shadowDelve/enemies/vampire/fireball/fireball.hpp"
#include "shadowDelve/player/player.hpp"
#include <memory>



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
    void playerAttackedHandler(const PlayerAttackedEvent& e);
    void attack();
    void roamToGoal(double dt);
    double getDist(vec2 start,vec2 end);
    int currFrame=0;
    vec2 goal{0,0};
    MODE mode=MODE::ATTACK;
    bool locked=false;
    bool decending=false;
    TaskId animationTask=UINT32_MAX;
    static constexpr int attackRange = 400;
    static constexpr int maxHealth = 100;
    static constexpr int aggroRange = 800;
    static constexpr int roamSpeed = 30;
    static constexpr int chaseSpeed = 100;
    int health=maxHealth;
    void setMode(MODE mode);
    bool canSeePlayer();
    void update(double dt) override;
    std::vector<std::unique_ptr<FireBall>> fireballs;
    void updateFireballs(double dt);

  public:
    Vampire(vec2 pos,Engine& e);

};
