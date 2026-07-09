#include "./vampire.hpp"
#include "engine/scheduleManager/scheduleManager.hpp"
#include "shadowDelve/player/player.hpp"
#include "shadowDelve/tileMap/tileMap.hpp"
#include "utilities/consts.hpp"
#include <cmath>
#include <glm/ext/quaternion_geometric.hpp>


Vampire::Vampire(vec2 pos,Engine& e):EnemyEntity(e){
  id = engine.makeSprite({pos.x,pos.y,ENTITY_LAYER}, "./assets/vampire/vampire_idle2.png",{0,0},{1.0f/6.0f,1});
  setMode(MODE::IDLE);
}





void Vampire::setMode(MODE mode){
  if(this->mode==mode)return;
  this->mode=mode;
  if(animationTask!=UINT32_MAX)ScheduleManager::cancel_task(animationTask);
  currFrame=0;
  auto old_trans = engine.componentManager.getComponent<Component::TRANSFORM>(id);

  switch(mode){
    case MODE::IDLE:
      engine.changeSprite(id,"./assets/vampire/vampire_idle2.png",{0,0},{1.0f/6.0f,1});
      animationTask=ScheduleManager::do_every(0.15, [this](){
          if(currFrame==6)currFrame=0;
          engine.componentManager.setComponent(id, Component::UVRECT{{(currFrame)/6.0f,0},{(currFrame+1)/6.0f,1}});
          currFrame++;
      });
      break;
    case MODE::ATTACK:
      engine.changeSprite(id,"./assets/vampire/vampire_attack2.png",{0,0},{1.0f/16.0f,1});
      decending=false;
      animationTask=ScheduleManager::do_every(0.1, [this](){
          if(currFrame==6)decending=true;
          else if(currFrame==0&&decending){
            decending=false;
            locked=false;
            setMode(MODE::IDLE);
            return;
          }
          engine.componentManager.setComponent(id, Component::UVRECT{{(currFrame)/16.0f,0},{(currFrame+1)/16.0f,1}});
          if(!decending)currFrame++;
          else currFrame--;
      });
      break;
    case MODE::CHASE:
      engine.changeSprite(id,"./assets/vampire/vampire_movement2.png",{0,0},{1.0f/8.0f,1});
      animationTask=ScheduleManager::do_every(0.15, [this](){
          if(currFrame==7)currFrame=0;
          engine.componentManager.setComponent(id, Component::UVRECT{{(currFrame)/8.0f,0},{(currFrame+1)/8.0f,1}});
          currFrame++;
      });
      break;
  }
  if(old_trans.scale.x<0){
    auto new_trans = engine.componentManager.getComponent<Component::TRANSFORM>(id);
    new_trans.scale.x*=-1;
    engine.componentManager.setComponent(id,new_trans);
  }
}

double Vampire::getDist(vec2 start,vec2 end){
  auto deltaX =start.x-end.x;
  auto deltaY =start.y-end.y;
  return sqrt((deltaX*deltaX) + (deltaY*deltaY));
}


void Vampire::update(double dt){
  if(locked)return;
  vec2 player = engine.componentManager.getComponent<Component::TRANSFORM>(Player::id).position;
  vec2 vampire = engine.componentManager.getComponent<Component::TRANSFORM>(id).position;
  if(canAttack && getDist(player,vampire)<=attackRange && canSeePlayer())attack();
  else if(getDist(player,vampire)<=aggroRange)chasePlayer(dt);

}

bool Vampire::canSeePlayer(){
  vec2 player = engine.componentManager.getComponent<Component::TRANSFORM>(Player::id).position;
  vec2 vampire = engine.componentManager.getComponent<Component::TRANSFORM>(id).position;
  vec2 dir = glm::normalize(vampire-player);
  float step =  BLOCKSIZE * 0.25f;
  while(TileMap::positionToGridCords(player)!=TileMap::positionToGridCords(vampire)){
    auto [gridX,gridY]=TileMap::positionToGridCords(player);
    for(auto& tile:TileMap::tileMap[gridX][gridY]){
      auto uv = engine.componentManager.getComponent<Component::UVRECT>(tile.id);
      if(TileMap::isWall(uv.uvMin))return false;
    }
    player+=dir*step;
  }
  return true;

}

void Vampire::attack(){
  setMode(MODE::ATTACK);
  canAttack=false;
  locked=true;
  ScheduleManager::do_every(attackCooldown, [this](){canAttack=true;});
}



void Vampire::chasePlayer(double dt){
  setMode(MODE::CHASE);
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


void Vampire::go(vec2 pos){
  vec2 startPos = engine.componentManager.getComponent<Component::TRANSFORM>(id).position;
  velocity = glm::normalize(vec2{pos.x-startPos.x,pos.y-startPos.y});
  goal = pos;
  updateFacingDirection();
}
