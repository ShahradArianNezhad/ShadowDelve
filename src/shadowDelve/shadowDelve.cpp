#include "shadowDelve.hpp"

void ShadowDelve::init(){
  srand(time(0));
  engine.setAmbient(0.3);
  engine.setTargetFPS(60);
  auto cam = engine.componentManager.getComponent<Component::CAMERA2D>(engine.getActiveCamera());
  cam.zoom=1.5f;
  engine.componentManager.setComponent(engine.getActiveCamera(), cam);
  tileMap.init();
  player.init();
}


void ShadowDelve::update(double dt){
  player.update(dt);
  tileMap.update(dt);
}
