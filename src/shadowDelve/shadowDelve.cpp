#include "shadowDelve.hpp"
#include "utilities/consts.hpp"

void ShadowDelve::init(){
  Logger::setLogLevel(LogLevel::Info);
  srand(time(0));
  engine.setAmbient(0.3);
  engine.setTargetFPS(60);
  auto cam = engine.componentManager.getComponent<Component::CAMERA2D>(engine.getActiveCamera());
  cam.zoom=CAMERA_ZOOM;
  engine.componentManager.setComponent(engine.getActiveCamera(), cam);
  tileMap.init();
  player.init();
}


void ShadowDelve::update(double dt){
  player.update(dt);
  tileMap.update(dt);
}
