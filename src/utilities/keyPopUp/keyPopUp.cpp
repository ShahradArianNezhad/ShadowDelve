#include "keyPopUp.hpp"
#include "engine/entityManager/component/components.hpp"
#include "engine/scheduleManager/scheduleManager.hpp"
#include "utilities/consts.hpp"
#include <format>



KeyPopUp::KeyPopUp(Engine& e,vec2 position,char key):engine(e){
  id = engine.makeSprite({position.x,position.y,POPUP_LAYER},std::format("./assets/keys/{}.png",key),{currFrame/3.0f,0},{currFrame+1/3.0f,1});
  auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(id);
  trans.scale.x/=2;
  trans.scale.y/=2;
  engine.componentManager.setComponent(id, trans);
  animationTask = ScheduleManager::do_every(0.5, [this](){
      engine.componentManager.setComponent(id, Component::UVRECT{{(++currFrame/3.0f),0},{(currFrame+1)/3.0f,1}});
      if(currFrame==2)currFrame=0;
  });
}

KeyPopUp::~KeyPopUp(){
  if(animationTask!=UINT32_MAX)ScheduleManager::cancel_task(animationTask);
  if(id!=UINT32_MAX)engine.entityManager.deleteEntity(id);
}

