#include "./vampire.hpp"
#include "engine/entityManager/component/components.hpp"
#include "engine/scheduleManager/scheduleManager.hpp"
#include "shadowDelve/enemies/vampire/fireball/fireball.hpp"
#include "shadowDelve/player/player.hpp"
#include "shadowDelve/tileMap/tileMap.hpp"
#include "utilities/consts.hpp"
#include <cmath>
#include <glm/ext/quaternion_geometric.hpp>
#include <memory>


Vampire::Vampire(vec2 pos,Engine& e):EnemyEntity(e){
  id = engine.makeSprite({pos.x,pos.y,ENTITY_LAYER}, "./assets/vampire/vampire_idle2.png",{0,0},{1.0f/6.0f,1});
  engine.componentManager.setComponent(id, Component::RECTCOLLIDER{{0,0},{12,18}});
  EventManager::subscribe<PlayerAttackedEvent>([this](const PlayerAttackedEvent& e){playerAttackedHandler(e);});
  setMode(MODE::IDLE);
};

void Vampire::playerAttackedHandler(const PlayerAttackedEvent& e){
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
    case MODE::DAMAGED:
      locked=true;
      engine.changeSprite(id,"./assets/vampire/vampire_take_damage2.png",{0,0},{1.0f/5.0f,1});
      animationTask=ScheduleManager::do_every(0.075, [this](){
          if(currFrame==5){
            setMode(MODE::IDLE);
            locked=false;
          }
          engine.componentManager.setComponent(id, Component::UVRECT{{(currFrame)/5.0f,0},{(currFrame+1)/5.0f,1}});
          currFrame++;
      });
      break;
    case MODE::DEATH:
      engine.changeSprite(id,"./assets/vampire/vampire_death2.png",{0,0},{1.0f/14.0f,1});
      animationTask=ScheduleManager::do_every(0.15, [this](){
          if(currFrame==14){
            ScheduleManager::cancel_task(animationTask);
            animationTask=UINT32_MAX;
            locked=true;
            return;
          }
          engine.componentManager.setComponent(id, Component::UVRECT{{(currFrame)/14.0f,0},{(currFrame+1)/14.0f,1}});
          currFrame++;
      });
      break;
    case MODE::FALL:
      animationTask = ScheduleManager::do_every(0.01, [this](){
          auto render = engine.componentManager.getComponent<Component::RENDER>(id);
          if((render.color&0x000000FF) > 10)render.color-=10;
          else {
          render.color&=0xFFFFFF00;
          engine.componentManager.setComponent(id, render);
          ScheduleManager::cancel_task(animationTask);
          animationTask=UINT32_MAX;
          }
          engine.componentManager.setComponent(id, render);
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

void Vampire::updateFireballs(double dt){
  for(size_t i{0};i<fireballs.size();i++){
    auto& fireball = fireballs[i];
    if(fireball->destroyed){fireballs.erase(fireballs.begin()+i);i--;}
  }
  for(size_t i{0};i<fireballs.size();i++){
    auto& fireball = fireballs[i];
    fireball->update(dt);
  }
}

void Vampire::update(double dt){
  vec2 vampire = engine.componentManager.getComponent<Component::TRANSFORM>(id).position;
  auto [gridX,gridY] = TileMap::positionToGridCords(vampire);
  updateFireballs(dt);
  if(TileMap::isGridEmpty(gridX, gridY)){setMode(MODE::FALL);return;}
  if(locked)return;
  vec2 player = engine.componentManager.getComponent<Component::TRANSFORM>(Player::id).position;
  if(canAttack && getDist(player,vampire)<=attackRange && canSeePlayer())attack();
  else if(getDist(player,vampire)<=aggroRange && getDist(player,vampire)>=attackRange/2.0)chasePlayer(dt);
  else setMode(MODE::IDLE);
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
  ScheduleManager::do_after(attackCooldown, [this](){canAttack=true;});
  vec2 pos = engine.componentManager.getComponent<Component::TRANSFORM>(id).position;
  vec2 playerPos = engine.componentManager.getComponent<Component::TRANSFORM>(Player::id).position;
  fireballs.emplace_back(std::make_unique<FireBall>(engine,pos,playerPos-pos));
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
