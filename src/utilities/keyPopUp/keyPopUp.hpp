#pragma once
#include "engine/engine.hpp"
#include "engine/scheduleManager/task/task.hpp"



class KeyPopUp{
  Engine& engine;
  EntityId id=UINT32_MAX;
  TaskId animationTask=UINT32_MAX;
  int currFrame=0;
  
  public:
    KeyPopUp(Engine& e,vec2 position,char key);
    ~KeyPopUp();
};
