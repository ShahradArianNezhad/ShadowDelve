#include "fireball.hpp"
#include "engine/entityManager/component/components.hpp"
#include "engine/scheduleManager/scheduleManager.hpp"
#include "shadowDelve/player/player.hpp"
#include "utilities/consts.hpp"
#include <glm/geometric.hpp>


#define Xindex 0
#define Yindex 2


FireBall::FireBall(Engine& e,vec2 pos,vec2 moveDir):engine(e){
  id = engine.makeSprite({pos.x,pos.y,ENTITY_LAYER+1},"./assets/effects.png",{Xindex/40.0f,Yindex/25.0f},{(Xindex+1)/40.0f,(Yindex+1)/25.0f});
  light = engine.makeLight(pos,{240,84,42} , 200, 0.003);
  velocity = glm::normalize(moveDir)*projectileSpeed;

  auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(id);
  engine.componentManager.setComponent(id, Component::CIRCLECOLLIDER{trans.scale.x,{0,0}});
  startAnimation();
}

FireBall::FireBall(FireBall&& other) noexcept:engine(other.engine){
  if(other.animationTask!=UINT32_MAX)ScheduleManager::cancel_task(other.animationTask);
  id = other.id;
  light = other.light;
  currFrame=other.currFrame;
  velocity=other.velocity;
  startAnimation();
  other.id=UINT32_MAX;
  other.light=UINT32_MAX;
  other.animationTask=UINT32_MAX;
}

FireBall& FireBall::operator=(FireBall&& other) noexcept{
    if (this == &other)return *this;

    if(other.animationTask != UINT32_MAX)ScheduleManager::cancel_task(other.animationTask);
    id = other.id;
    light = other.light;
    destroyed=other.destroyed;
    velocity = other.velocity;
    currFrame = other.currFrame;
    startAnimation();
    other.id = UINT32_MAX;
    other.light = UINT32_MAX;
    other.animationTask = UINT32_MAX;

    return *this;
}



void FireBall::update(double dt){
  auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(id);
  auto lightTrans = engine.componentManager.getComponent<Component::TRANSFORM>(light);
  trans.position+=vec3{velocity*(float)dt,0};
  lightTrans.position+=vec3{velocity*(float)dt,0};
  engine.componentManager.setComponent(id, trans);
  engine.componentManager.setComponent(light, lightTrans);



  if(engine.rectCircleIsColliding(Player::id, id))destroyed=true;
  else{
    for(auto& tile:TileMap::getNearbyTiles(trans.position)){
      auto uv = engine.componentManager.getComponent<Component::UVRECT>(tile.id);
      if(TileMap::isWall(uv.uvMin)&&engine.isColliding(tile.id, id))destroyed=true;
    }
  }
}


FireBall::~FireBall(){
  if(id!=UINT32_MAX)engine.entityManager.deleteEntity(id);
  if(light!=UINT32_MAX)engine.entityManager.deleteEntity(light);
  if(animationTask!=UINT32_MAX)ScheduleManager::cancel_task(animationTask);
}


void FireBall::startAnimation(){
  animationTask = ScheduleManager::do_every(0.1, [this](){
      if(currFrame==5)currFrame=0;
      engine.componentManager.setComponent(id,Component::UVRECT{{(currFrame+Xindex)/40.0f,Yindex/25.0f},{(currFrame+Xindex+1)/40.0f,(Yindex+1)/25.0f}});
      currFrame++;
      });
}
