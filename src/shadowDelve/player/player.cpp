#include "player.hpp"
#include "engine/entityManager/component/components.hpp"
#include "engine/scheduleManager/scheduleManager.hpp"
#include "shadowDelve/tileMap/tileMap.hpp"
#include "utilities/consts.hpp"
#include "utilities/keyPopUp/keyPopUp.hpp"
#include <cstdlib>
#include <glm/geometric.hpp>


void Player::init(){
  id = engine.makeSprite({0,0,ENTITY_LAYER}, "./assets/Soldier/Soldier.png",{0,0},{1.0/uvSegmentsX,1.0/uvSegmentsY});
  engine.componentManager.setComponent(id, Component::RECTCOLLIDER{{0,-3},{12,18},0});
  if(showCollider){
    auto comp = engine.componentManager.getComponent<Component::TRANSFORM>(id);
    collider = engine.makeRect(comp.position, {12,18});
  }
  setMode(MODE::IDLE);
  tileMap.setPlayer(id);
}

void Player::animationFunction(const AnimationData& data){
  auto comp = Component::UVRECT{{animationFrame/uvSegmentsX,data.yIndex/uvSegmentsY},{(animationFrame+1)/uvSegmentsX,(data.yIndex+1)/uvSegmentsY}};
  engine.componentManager.setComponent(id, comp);
  if(animationFrame!=data.maxFrames) animationFrame+=1;
  else animationFrame=0;
};


void Player::setMode(MODE mode){
  if(this->mode==mode)return;
  this->mode = mode;
  AnimationData data; 

  switch(mode){
    case MODE::IDLE:
      data=idleAnimationData;
      break;

    case MODE::MOVE:
      data=moveAnimationData;
      break;
    case MODE::ATTACK:
      data=basicMelleAttackAnimationData;
      break;
  }

  animationFrame=0;
  if(animationJob!=UINT32_MAX)ScheduleManager::cancel_task(animationJob);
  animationJob = ScheduleManager::do_every(data.secsPerFrame,[this, data](){animationFunction(data);});
}


void Player::applyVelocity(double dt){
    auto comp = engine.componentManager.getComponent<Component::TRANSFORM>(id);
    auto nearbyTiles = TileMap::getNearbyTiles(comp.position);
    auto normalVelocity = glm::normalize(velocity);
    if(dashing){
      normalVelocity.x*=3;
      normalVelocity.y*=3;
    }


    comp.position.x+=normalVelocity.x*maxSpeed*dt;
    engine.componentManager.setComponent(id, comp);
    for(auto& tile:nearbyTiles){
      if(tile.type==TileType::Wall && engine.rectIsColliding(tile.id, id)){
        comp.position.x-=normalVelocity.x*maxSpeed*dt;
        engine.componentManager.setComponent(id, comp);
      }
    }

    comp.position.y+=normalVelocity.y*maxSpeed*dt;
    engine.componentManager.setComponent(id, comp);
    for(auto& tile:nearbyTiles){
      if(tile.type==TileType::Wall && engine.rectIsColliding(tile.id, id)){
        comp.position.y-=normalVelocity.y*maxSpeed*dt;
        engine.componentManager.setComponent(id, comp);
      }
    }


    if(showCollider){
      auto colTrans = engine.componentManager.getComponent<Component::TRANSFORM>(collider);
      auto colRect = engine.componentManager.getComponent<Component::RECTCOLLIDER>(id);
      colTrans.position = {comp.position.x+colRect.offset.x,comp.position.y+colRect.offset.y,comp.position.z-1};
      colTrans.scale = vec3{colRect.scale,0};
      engine.componentManager.setComponent(collider, colTrans);
    }

}

void Player::updatePosition(double dt){
  if(glm::length(velocity)>0.0f){
    applyVelocity(dt);
    setMode(MODE::MOVE);
  }else if(mode==MODE::MOVE)setMode(MODE::IDLE);   
}

void Player::updateVelocity(){
  vec2 v;
  if(engine.inputHandler.checkKeyPress(Key::W))v.y=-1;
  else if(engine.inputHandler.checkKeyPress(Key::S))v.y=1;
  if(engine.inputHandler.checkKeyPress(Key::A))v.x=-1;
  else if(engine.inputHandler.checkKeyPress(Key::D))v.x=1;
  velocity=v;
}

void Player::updateFacingDirection(){
  auto comp = engine.componentManager.getComponent<Component::TRANSFORM>(id);
  if(velocity.x < 0) comp.scale.x = -std::abs(comp.scale.x);
  else if(velocity.x > 0) comp.scale.x = std::abs(comp.scale.x);
  engine.componentManager.setComponent(id, comp);
}

void Player::centerCameraOnPlayer(){
  auto cam = engine.getActiveCamera();
  auto camComp = engine.componentManager.getComponent<Component::CAMERA2D>(cam);
  auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(id);
  camComp.position = {trans.position.x,trans.position.y};
  engine.componentManager.setComponent(cam, camComp);
}


void Player::handleInteract(){
  auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(id);
  auto nearby = TileMap::getNearbyTiles(trans.position);
  for(auto& tile:nearby){
    auto uv = engine.componentManager.getComponent<Component::UVRECT>(tile.id);
    if(TileMap::isDoor(uv.uvMin)){
      auto pair = tileMap.getDoorPair(tile.id);
      if(pair.second!=UINT32_MAX)tileMap.toggleDoor(pair,trans.position);
      if(popUps.contains("door"))popUps.erase("door");
      break;
    };
  }
}

void Player::dash(){
  if(!locked && canDash){
    dashing=true;
    canDash=false;
    auto pos = engine.componentManager.getComponent<Component::TRANSFORM>(id).position;
    trail1 = engine.makeSprite({pos.x,pos.y,ENTITY_LAYER-1}, "./assets/Soldier/Soldier.png",{0,0},{1.0/uvSegmentsX,1.0/uvSegmentsY});
    trail2 = engine.makeSprite({pos.x,pos.y,ENTITY_LAYER-1}, "./assets/Soldier/Soldier.png",{0,0},{1.0/uvSegmentsX,1.0/uvSegmentsY});
    trail3 = engine.makeSprite({pos.x,pos.y,ENTITY_LAYER-1}, "./assets/Soldier/Soldier.png",{0,0},{1.0/uvSegmentsX,1.0/uvSegmentsY});
    auto render = engine.componentManager.getComponent<Component::RENDER>(trail1); 
    render.color &= render.color & 0xFFFFFFBB;
    engine.componentManager.setComponent(trail1,render);
    render.color &= render.color & 0xFFFFFF88;
    engine.componentManager.setComponent(trail2,render);
    render.color &= render.color & 0xFFFFFF22;
    engine.componentManager.setComponent(trail3,render);
    ScheduleManager::do_after(0.2, [this](){
        dashTimer=0;
        dashing=false;
        engine.entityManager.deleteEntity(trail1);
        engine.entityManager.deleteEntity(trail2);
        engine.entityManager.deleteEntity(trail3);
        trail1=UINT32_MAX;
        trail2=UINT32_MAX;
        trail3=UINT32_MAX;
        
    });
    ScheduleManager::do_after(1, [this](){canDash=true;});
    ScheduleManager::do_after(0.01, [this](){
    });
  }
}


void Player::handleInput(){
  if(!dashing)updateVelocity();
  if(engine.inputHandler.checkKeyPress(Key::LeftShift)&&mode!=MODE::ATTACK)dash();
  if(engine.inputHandler.checkKeyPress(Key::E))handleInteract();
  if(engine.inputHandler.checkMousePress(Mouse::LEFT)&&!dashing)attack();
  else if(mode==MODE::ATTACK && !locked)setMode(MODE::IDLE);
}

void Player::attack(){
  setMode(MODE::ATTACK);
  locked=true;
  ScheduleManager::do_after(basicMelleAttackAnimationData.secsPerFrame*(basicMelleAttackAnimationData.maxFrames+1),[this](){locked=false;});
}

void Player::AddDoorPopUpOnNearbyDoor(){
    auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(id);
    auto nearby = TileMap::getNearbyTiles(trans.position);
    for(auto& tile:nearby){
      auto uv = engine.componentManager.getComponent<Component::UVRECT>(tile.id);
      if(TileMap::isDoor(uv.uvMin)){
        int x=-BLOCKSIZE/2;
        if(TileMap::isLeftDoor(uv.uvMin))x*=-1;
        auto doorTrans = engine.componentManager.getComponent<Component::TRANSFORM>(tile.id);
        popUps.try_emplace("door",engine,vec2{doorTrans.position.x+x,doorTrans.position.y-BLOCKSIZE},'E');
        needDoorPopUp=false;
        break;
      };
    }
}

void Player::makePopUps(){
  if(needDoorPopUp)AddDoorPopUpOnNearbyDoor();
}

void Player::updateTrails(){
  auto mult=dashTimer;
  if(dashTimer>0.1){
    mult = 0.2 - dashTimer;
    mult = std::max(mult, 0.0);
  }
  auto normalVelocity = glm::normalize(velocity);
  auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(id);
  trans.position.z-=1;

  trans.position.x-=normalVelocity.x*mult*100;
  trans.position.y-=normalVelocity.y*mult*100;
  engine.componentManager.setComponent(trail1, trans);

  trans.position.x-=normalVelocity.x*mult*150;
  trans.position.y-=normalVelocity.y*mult*150;
  engine.componentManager.setComponent(trail2, trans);

  trans.position.x-=normalVelocity.x*mult*200;
  trans.position.y-=normalVelocity.y*mult*200;
  engine.componentManager.setComponent(trail3, trans);
}


void Player::handleMove(double dt){
  if(!locked){
    updateFacingDirection();
    updatePosition(dt);
  }
}

void Player::updateDash(double dt){
    updateTrails();
    dashTimer+=dt;
}

void Player::update(double dt){
  makePopUps();
  handleInput();
  handleMove(dt);
  centerCameraOnPlayer();
  if(dashing)updateDash(dt);
}






