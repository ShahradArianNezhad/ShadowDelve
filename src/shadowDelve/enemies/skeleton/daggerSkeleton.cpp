#include "./daggerSkeleton.hpp"
#include "engine/eventManager/eventManager.hpp"
#include "engine/scheduleManager/scheduleManager.hpp"
#include "shadowDelve/enemies/enemy.hpp"
#include "shadowDelve/player/player.hpp"
#include "utilities/consts.hpp"
#include <cmath>
#include <cstdint>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/geometric.hpp>


DaggerSkeleton::DaggerSkeleton(vec2 pos,Engine& e):Skeleton(e){
  id = engine.makeSprite({pos.x,pos.y,ENTITY_LAYER},"assets/skeleton/skeleton1_idle2.png",{0,0},{1.0f/6.0f,1});
  engine.componentManager.setComponent(id, Component::RECTCOLLIDER{{-3,1},{12,18}});
  if(showCollider)collider = engine.makeRect({pos.x,pos.y,9}, {12,10});
  setMode(MODE::IDLE);
  ScheduleManager::do_every(random()%6 + 3,[this](){
      if(mode==MODE::IDLE)makeRandomMove();
  });
}




void DaggerSkeleton::setMode(MODE mode){
  if(this->mode == mode)return;
  locked=false;

  this->mode = mode;
  if(animationTask!=UINT32_MAX)ScheduleManager::cancel_task(animationTask);
  currFrame=0;
  auto old_trans = engine.componentManager.getComponent<Component::TRANSFORM>(id);
  switch(mode){
    case MODE::IDLE:
      engine.changeSprite(id,"assets/skeleton/skeleton1_idle2.png",{0,0},{1.0f/6.0f,1});
      animationTask = ScheduleManager::do_every(0.2,[this](){
          if(currFrame==5)currFrame=0;
          engine.componentManager.setComponent(id, Component::UVRECT{{currFrame/6.0f,0},{(currFrame+1)/6.0f,1}});
          currFrame++;
      });
      break;
    case MODE::WALK:
    case MODE::CHASE:
      engine.changeSprite(id,"assets/skeleton/skeleton1_movement2.png",{0,0},{1.0f/10.0f,1});
      animationTask = ScheduleManager::do_every(0.2,[this](){
          if(currFrame==9)currFrame=0;
          engine.componentManager.setComponent(id, Component::UVRECT{{currFrame/10.0f,0},{(currFrame+1)/10.0f,1}});
          currFrame++;
      });
      break;
    case MODE::ATTACK:
      engine.changeSprite(id,"assets/skeleton/skeleton1_attack2.png",{0,0},{1.0f/9.0f,1});
      currFrame=3;
      animationTask = ScheduleManager::do_every(0.075,[this](){
          if(currFrame==8){
            currFrame=3;
            locked=false;
          }
          engine.componentManager.setComponent(id, Component::UVRECT{{currFrame/9.0f,0},{(currFrame+1)/9.0f,1}});
          currFrame++;
      });
      break;
    case MODE::DAMAGED:
      engine.changeSprite(id,"assets/skeleton/skeleton1_take_damage2.png",{0,0},{1.0f/5.0f,1});
      animationTask = ScheduleManager::do_every(0.075,[this](){
          if(currFrame==5){setMode(MODE::CHASE);return;}
          engine.componentManager.setComponent(id, Component::UVRECT{{currFrame/5.0f,0},{(currFrame+1)/5.0f,1}});
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



void DaggerSkeleton::update(double dt){
  vec2 pos = engine.componentManager.getComponent<Component::TRANSFORM>(id).position;
  vec2 playerPos = engine.componentManager.getComponent<Component::TRANSFORM>(Player::id).position;
  auto d = getDist(pos, playerPos);
  if(d<300 && (mode==MODE::WALK || mode==MODE::IDLE))setMode(MODE::CHASE);
  switch(mode){
    case MODE::IDLE:break;
    case MODE::WALK:
      roamToGoal(dt);
      if(pos==goal)setMode(MODE::IDLE);
      break;
    case MODE::CHASE:
      chasePlayer(dt);
      if(d>600)setMode(MODE::IDLE);
      else if(d<=attackRange)attack();
      break;
    case MODE::ATTACK:
      if(currFrame==4 && d>attackRange){
        setMode(MODE::CHASE);
        engine.componentManager.setComponent(id, Component::RECTCOLLIDER{{-3,1},{12,18}});
      }
      if(engine.rectIsColliding(Player::id, id)&&!locked&&currFrame>=6){
        EventManager::emit(PlayerDamagedEvent{id,10});
        locked=true;
      }
      break;
    case MODE::DAMAGED:
      break;
  }
}

