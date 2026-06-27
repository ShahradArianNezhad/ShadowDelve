#pragma once
#include "engine/engine.hpp"
#include "engine/scheduleManager/task/task.hpp"



class KeyPopUp{
  Engine& engine;
  EntityId id;
  TaskId animationTask;
  int currFrame=0;
  
  public:
    KeyPopUp(Engine& e,vec2 position,char key);
    ~KeyPopUp();
};
