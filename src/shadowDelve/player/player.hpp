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
    MOVE,
    BASIC_ATTACK,
    HEAVY_ATTACK
  };

  static constexpr float uvSegmentsX=9.0f;
  static constexpr float uvSegmentsY=7.0f;
  static constexpr int maxSpeed=200;
  static constexpr AnimationData idleAnimationData{0,5,0.18f};
  static constexpr AnimationData moveAnimationData{1,7,0.1f};
  static constexpr AnimationData basicMelleAttackAnimationData{2,5,0.05f};
  static constexpr AnimationData heavyMelleAttackAnimationData{3,5,0.06f};


  EntityId id;
  TaskId animationJob=UINT32_MAX;
  int animationFrame=0;
  Engine& engine;
  TileMap& tileMap;
  MODE mode;
  vec2 velocity{0,0};
  bool dashing=false;
  bool canDash=true;
  int dashSpeedMult = 3; 
  float dashTime = 0.2f;
  float dashCooldown = 1.0f;
  bool locked=false;
  double dashTimer=0;
  EntityId trail1=UINT32_MAX,trail2=UINT32_MAX,trail3=UINT32_MAX;
  EntityId collider;

  bool needDoorPopUp=true;
  std::unordered_map<std::string,KeyPopUp> popUps;

  void applyVelocity(vec2 v);
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
  void basicAttack();
  void heavyAttack();
  void handleMove(double dt);
  void updateDash(double dt);

  public:
    Player(Engine& e,TileMap& t):engine(e),tileMap(t){};
    void init();
    void setMode(MODE mode);
    void update(double dt);
};
