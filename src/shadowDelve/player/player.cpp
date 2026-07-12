#include "player.hpp"
#include "engine/entityManager/component/components.hpp"
#include "engine/eventManager/eventManager.hpp"
#include "engine/scheduleManager/scheduleManager.hpp"
#include "shadowDelve/tileMap/tileMap.hpp"
#include "utilities/consts.hpp"
#include "utilities/keyPopUp/keyPopUp.hpp"
#include <cstdlib>
#include <glm/geometric.hpp>


void Player::init(){
  id = engine.makeSprite({0,0,PLAYER_LAYER}, "./assets/Soldier/Soldier2.png",{0,0},{1.0/uvSegmentsX,1.0/uvSegmentsY});
  engine.componentManager.setComponent(id, Component::RECTCOLLIDER{{0,-3},colliderScale,0});
  if(showCollider){
    auto comp = engine.componentManager.getComponent<Component::TRANSFORM>(id);
    collider = engine.makeRect(comp.position, colliderScale);
  }
  setMode(MODE::IDLE);
  tileMap.setPlayer(id);
  EventManager::subscribe<PlayerDamagedEvent>([this](const PlayerDamagedEvent& e){
      auto playerPos = engine.componentManager.getComponent<Component::TRANSFORM>(id).position;
      auto enemyPos = engine.componentManager.getComponent<Component::TRANSFORM>(e.from).position;
      vec2 dir = glm::normalize(vec2{enemyPos.x-playerPos.x,enemyPos.y-playerPos.y});
      dir.x*=-7;
      dir.y*=-7;
      auto knockbackTask = ScheduleManager::do_every(0.01,[this, dir](){
          applyVelocity(dir);
      });
      ScheduleManager::do_after(0.1, [this, knockbackTask](){ScheduleManager::cancel_task(knockbackTask);});
      health-=e.damage;
      if(health<=0)setMode(MODE::DEATH);
      else setMode(MODE::DAMAGED);
  });
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
    case MODE::BASIC_ATTACK:
      data=basicMelleAttackAnimationData;
      break;
    case MODE::HEAVY_ATTACK:
      data=heavyMelleAttackAnimationData;
      break;
    case MODE::DAMAGED:
      data=DamagedAnimationData;
      break;
    case MODE::DEATH:
      data=DeathAnimationData;
      break;
    case MODE::FALL:
      ScheduleManager::cancel_task(animationJob);
      animationJob = ScheduleManager::do_every(0.01, [this](){fallAnimationFunc();});
      return;
  }

  animationFrame=0;
  if(animationJob!=UINT32_MAX)ScheduleManager::cancel_task(animationJob);
  animationJob = ScheduleManager::do_every(data.secsPerFrame,[this, data](){animationFunction(data);});
}

void Player::fallAnimationFunc(){
  auto render = engine.componentManager.getComponent<Component::RENDER>(id);
  if((render.color&0x000000FF) > 10)render.color-=10;
  else {
    render.color&=0xFFFFFF00;
    engine.componentManager.setComponent(id, render);
    ScheduleManager::cancel_task(animationJob);
    animationJob=UINT32_MAX;
    ScheduleManager::do_after(1, [this, render](){
        auto render = engine.componentManager.getComponent<Component::RENDER>(id);
        render.color|=0x000000FF;
        respawn();
        engine.componentManager.setComponent(id, render);
        });
  }
  engine.componentManager.setComponent(id, render);
}


void Player::applyVelocity(vec2 v){
    auto comp = engine.componentManager.getComponent<Component::TRANSFORM>(id);
    auto nearbyTiles = TileMap::getNearbyTiles(comp.position);

    comp.position.x+=v.x;
    engine.componentManager.setComponent(id, comp);
    for(auto& tile:nearbyTiles){
      if(tile.type==TileType::Wall && engine.rectIsColliding(tile.id, id)){
        comp.position.x-=v.x;
        engine.componentManager.setComponent(id, comp);
      }
    }

    comp.position.y+=v.y;
    engine.componentManager.setComponent(id, comp);
    for(auto& tile:nearbyTiles){
      if(tile.type==TileType::Wall && engine.rectIsColliding(tile.id, id)){
        comp.position.y-=v.y;
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
    vec2 v = glm::normalize(velocity);
    v.x *= maxSpeed*dt;
    v.y *= maxSpeed*dt;
    applyVelocity(v);
    if(mode!=MODE::DAMAGED)setMode(MODE::MOVE);
  }else if(mode==MODE::MOVE)setMode(MODE::IDLE);   
}

void Player::updateVelocity(){
  vec2 v{0,0};
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
      if(pair.locked)return;
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
    auto uv = engine.componentManager.getComponent<Component::UVRECT>(id);
    for(int i=0;i<trailCount;i++){
      auto& trail = trails[i];
      trail = engine.makeSprite({pos.x,pos.y,PLAYER_LAYER-1}, "./assets/Soldier/Soldier.png",uv.uvMin,uv.uvMax);
      auto render = engine.componentManager.getComponent<Component::RENDER>(trail); 
      uint32_t color = 34 + i*70;
      color |= 0xFFFFFF00;
      render.color &= color;
      engine.componentManager.setComponent(trail,render);
    }
    ScheduleManager::do_after(dashTime, [this](){
        dashTimer=0;
        dashing=false;
        for(auto& trail:trails){
          engine.entityManager.deleteEntity(trail);
          trail=UINT32_MAX;
        }
    });
    ScheduleManager::do_after(dashCooldown, [this](){canDash=true;});
  }
}


void Player::handleInput(){
  if(!dashing)updateVelocity();
  if(engine.inputHandler.checkKeyPress(Key::LeftShift)&&!locked)dash();
  if(engine.inputHandler.checkKeyPress(Key::E))handleInteract();
  if(engine.inputHandler.checkMousePress(Mouse::LEFT))basicAttack();
  else if(engine.inputHandler.checkMousePress(Mouse::RIGHT) && glm::length(velocity)>0 && canDash)heavyAttack();
}

void Player::basicAttack(){
  if(!dashing && !locked ){
  setMode(MODE::BASIC_ATTACK);
  locked=true;
  ScheduleManager::do_after(basicMelleAttackAnimationData.secsPerFrame*(basicMelleAttackAnimationData.maxFrames+1),[this](){
      locked=false;
      setMode(MODE::IDLE);
      });
  }
  engine.componentManager.setComponent(id, Component::RECTCOLLIDER{{4,-3},{20,18}});
  EventManager::emit(PlayerAttackedEvent{10});
  engine.componentManager.setComponent(id, Component::RECTCOLLIDER{{0,-3},colliderScale});
}

void Player::heavyAttack(){
  if(!dashing && !locked ){
    dash();
    setMode(MODE::HEAVY_ATTACK);
    locked=true;
    ScheduleManager::do_after(heavyMelleAttackAnimationData.secsPerFrame*(heavyMelleAttackAnimationData.maxFrames+1),[this](){
        locked=false;
        engine.componentManager.setComponent(id, Component::RECTCOLLIDER{{4,-3},{20,20}});
        EventManager::emit(PlayerAttackedEvent{10});
        engine.componentManager.setComponent(id, Component::RECTCOLLIDER{{0,-3},colliderScale});
        setMode(MODE::IDLE);
        });
  }
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
  if(dashTimer>dashTime/2){
    mult = dashTime - dashTimer;
    mult = std::max(mult, 0.0);
  }
  auto normalVelocity = glm::normalize(velocity);
  auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(id);
  auto uv = engine.componentManager.getComponent<Component::UVRECT>(id);
  trans.position.z-=1;

  for(int i=0;i<trailCount;i++){
    auto& trail = trails[i];
    trans.position.x-=normalVelocity.x*mult*(i+1)*70;
    trans.position.y-=normalVelocity.y*mult*(i+1)*70;
    engine.componentManager.setComponent(trail, trans);
    engine.componentManager.setComponent(trail, uv);
  }
}


void Player::handleMove(double dt){
  if(!locked){
    updateFacingDirection();
    updatePosition(dt);
  }
}

void Player::updateDash(double dt){
  if(glm::length(velocity)>0.0f){
    vec2 v = glm::normalize(velocity);
    v.x*=maxSpeed*dt*dashSpeedMult;
    v.y*=maxSpeed*dt*dashSpeedMult;
    applyVelocity(v);
    updateTrails();
    dashTimer+=dt;
  }
}

void Player::update(double dt){
  if(mode==MODE::DEATH && animationFrame==3)deathHandler();
  if(mode==MODE::DEATH || mode==MODE::FALL)return;
  if(mode==MODE::DAMAGED && animationFrame==3)setMode(MODE::MOVE);
  if(mode==MODE::HEAVY_ATTACK)EventManager::emit(PlayerAttackedEvent{10});
  if(!dashing && shouldFall()){
      setMode(MODE::FALL);
      return;
  }
  makePopUps();
  handleInput();
  handleMove(dt);
  if(dashing)updateDash(dt);
  centerCameraOnPlayer();
}


bool Player::shouldFall(){
  vec2 pos = engine.componentManager.getComponent<Component::TRANSFORM>(id).position;
  auto [gridX,gridY] = tileMap.positionToGridCords(pos);
  if(tileMap.isGridEmpty(gridX, gridY))return true;
  return false;
}

void Player::deathHandler(){
    ScheduleManager::cancel_task(animationJob);
    animationJob=UINT32_MAX;
    animationFrame=0;
    ScheduleManager::do_after(1, [this](){
        respawn();
    });
}


void Player::respawn(){
  health=maxHealth;
  setMode(MODE::IDLE);
  auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(id);
  trans.position.x=0;
  trans.position.y=0;
  engine.componentManager.setComponent(id,trans);
}
