#include "healthBar.hpp"
#include "engine/entityManager/component/components.hpp"
#include "engine/eventManager/eventManager.hpp"
#include "platform/window/GLFWwindow.hpp"
#include "shadowDelve/player/player.hpp"
#include "utilities/consts.hpp"

HealthBar::HealthBar(Engine& e):engine(e){
  frame = engine.makeSprite({0,0,0}, "./assets/ui.png",{0.0f/maxSegmentsX,0.0f/maxSegmentsY},{1.0f/maxSegmentsX,1.0f/maxSegmentsY},Layer::UI);
  bar = engine.makeSprite({0,0,0}, "./assets/ui.png",{1.0f/maxSegmentsX,0.0f/maxSegmentsY},{2.0f/maxSegmentsX,1.0f/maxSegmentsY},Layer::UI);
  putAtTopLeft();

  EventManager::subscribe<WindowSizeChangeEvent>([this](const WindowSizeChangeEvent&){putAtTopLeft();});
  EventManager::subscribe<PlayerHealthChangedEvent>([this](const PlayerHealthChangedEvent& e){update(e.newHealth);});
}


void HealthBar::update(int health){
  int segments = Player::maxHealth / (maxSegmentsX-1);
  int minUv = (maxSegmentsX-1) - (health/segments) + 1;
  engine.changeSprite(bar, "./assets/ui.png",{(float)minUv/maxSegmentsX,0.0f/maxSegmentsY},{((float)minUv+1)/maxSegmentsX,1.0f/maxSegmentsY});
}


void HealthBar::putAtTopLeft(){
  auto frameTrans = engine.componentManager.getComponent<Component::TRANSFORM>(frame);
  auto barTrans = engine.componentManager.getComponent<Component::TRANSFORM>(bar);
  frameTrans.position = {(-(float)Screen::width/2.0f)+frameTrans.scale.x*1.5 + 10,(-(float)Screen::height/2.0f)+frameTrans.scale.y*1.5,0};
  frameTrans.position/= CAMERA_ZOOM;
  barTrans.position = {(-(float)Screen::width/2.0f)+barTrans.scale.x*1.5 + 10,(-(float)Screen::height/2.0f)+barTrans.scale.y*1.5,0};
  barTrans.position/= CAMERA_ZOOM;
  engine.componentManager.setComponent(frame, frameTrans);
  engine.componentManager.setComponent(bar,barTrans);
}
