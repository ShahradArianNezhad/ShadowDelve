#include "skeleton.hpp"
#include "engine/eventManager/eventManager.hpp"
#include "engine/scheduleManager/scheduleManager.hpp"
#include "shadowDelve/enemies/enemy.hpp"
#include "shadowDelve/player/player.hpp"
#include "utilities/consts.hpp"
#include <cmath>
#include <cstdint>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/geometric.hpp>


Skeleton::Skeleton(Engine& e):EnemyEntity(e){
  EventManager::subscribe<PlayerAttackedEvent>([this](const PlayerAttackedEvent& e){playerAttackedHandler(e);});
};

void Skeleton::playerAttackedHandler(const PlayerAttackedEvent& e){
  if(!engine.rectIsColliding(id, Player::id))return;
  if(mode==MODE::DEATH)return;
  auto playerPos = engine.componentManager.getComponent<Component::TRANSFORM>(Player::id).position;
  auto enemyPos = engine.componentManager.getComponent<Component::TRANSFORM>(id).position;
  vec2 dir = glm::normalize(vec2{playerPos.x-enemyPos.x,playerPos.y-enemyPos.y});
  dir.x*=-5;
  dir.y*=-5;
  setMode(MODE::DAMAGED);
  auto knockbackTask = ScheduleManager::do_every(0.01,[this, dir](){
      applyVelocity(dir);
      });
  ScheduleManager::do_after(0.1, [this, knockbackTask](){ScheduleManager::cancel_task(knockbackTask);});
  health-=e.damage;
  if(health<=0)setMode(MODE::DEATH);

}



void Skeleton::makeRandomMove(){
  setMode(MODE::WALK);
  auto dest = getRandomMove();
  go(dest);
}



void Skeleton::go(vec2 pos){
  vec2 startPos = engine.componentManager.getComponent<Component::TRANSFORM>(id).position;
  velocity = glm::normalize(vec2{pos.x-startPos.x,pos.y-startPos.y});
  goal = pos;
  updateFacingDirection();
}

double Skeleton::getDist(vec2 start,vec2 end){
  auto deltaX =start.x-end.x;
  auto deltaY =start.y-end.y;
  return sqrt((deltaX*deltaX) + (deltaY*deltaY));
}


void Skeleton::chasePlayer(double dt){
  vec2 pos = engine.componentManager.getComponent<Component::TRANSFORM>(id).position;
  vec2 playerPos = engine.componentManager.getComponent<Component::TRANSFORM>(Player::id).position;
  auto path = findPath(playerPos);
  if(path.size()==0){
    setMode(MODE::IDLE);
    return;
  }
  go(vec2{path[0].x*BLOCKSIZE- BLOCKSIZE/2.0,path[0].y*BLOCKSIZE- BLOCKSIZE/2.0});
  auto v = velocity;
  v.x*=dt*chaseSpeed;
  v.y*=dt*chaseSpeed;
  if(getDist(pos,goal)>glm::length(v))applyVelocity(v);
  else{
    auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(id);
    trans.position.x=goal.x;
    trans.position.y=goal.y;
    engine.componentManager.setComponent(id, trans);
    velocity={0,0};
  };

}
 

void Skeleton::roamToGoal(double dt){
  vec2 pos = engine.componentManager.getComponent<Component::TRANSFORM>(id).position;
  auto v = velocity;
  v.x*=dt*roamSpeed;
  v.y*=dt*roamSpeed;
  if(getDist(pos,goal)>glm::length(v))applyVelocity(v);
  else{
    auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(id);
    trans.position.x=goal.x;
    trans.position.y=goal.y;
    engine.componentManager.setComponent(id, trans);
    velocity={0,0};
  };
}


void Skeleton::attack(){
  if(!canAttack)return;
  auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(id);
  vec2 pos = trans.position;
  vec2 playerPos = engine.componentManager.getComponent<Component::TRANSFORM>(Player::id).position;
  if(trans.position.x>playerPos.x)trans.position.x=-std::abs(trans.position.x);
  else trans.position.x=std::abs(trans.position.x);

  if(trans.scale.x>0)engine.componentManager.setComponent(id, Component::RECTCOLLIDER{{8,1},{23,20}});
  else engine.componentManager.setComponent(id, Component::RECTCOLLIDER{{-8,1},{23,20}});
  setMode(MODE::ATTACK);
  canAttack=false;
  ScheduleManager::do_after(attackCooldown, [this](){canAttack=true;});
}
