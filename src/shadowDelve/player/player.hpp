#pragma once
#include "engine/engine.hpp"
#include "engine/entityManager/component/components.hpp"
#include "engine/scheduleManager/task/task.hpp"
#include "shadowDelve/tileMap/tileMap.hpp"
#include "utilities/keyPopUp/keyPopUp.hpp"
#include "utilities/types.hpp"
#include <cstdint>



class Player {

  enum class MODE{
    IDLE,
    MOVE
  };

  static constexpr float uvSegmentsX=9.0f;
  static constexpr float uvSegmentsY=7.0f;
  static constexpr int maxSpeed=200;
  static constexpr AnimationData idleAnimationData{0,5,0.18f};
  static constexpr AnimationData moveAnimationData{1,7,0.1f};


  EntityId id;
  TaskId animationJob=UINT32_MAX;
  int animationFrame=0;
  Engine& engine;
  TileMap& tileMap;
  MODE mode;
  vec2 velocity{0,0};
  bool dashing=false;
  bool canDash=true;
  double dashTimer=0;
  EntityId trail1=UINT32_MAX,trail2=UINT32_MAX,trail3=UINT32_MAX;

  bool needDoorPopUp=true;
  size_t doorPopUpIndex=SIZE_MAX;
  std::vector<KeyPopUp> popUps;

  void applyVelocity(double dt);
  void updateFacingDirection();
  void animationFunction(const AnimationData& data);
  void updateVelocity();
  void updatePosition(double dt);
  void handleInput();
  void handleInteract();
  void centerCameraOnPlayer();
  void dash();
  void makePopUps();
  void AddDoorPopUpOnNearbyDoor();
  void updateTrails();

  public:
    Player(Engine& e,TileMap& t):engine(e),tileMap(t){};
    void init();
    void setMode(MODE mode);
    void update(double dt);
};
