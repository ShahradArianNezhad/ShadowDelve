#pragma once
#include "engine/engine.hpp"



class HealthBar{
  private:
    static constexpr int maxSegmentsX=7;
    static constexpr int maxSegmentsY=13;
    EntityId frame,bar;
    Engine& engine;
    int currState=0;
    void putAtTopLeft();
    void update(int damage);

  public:
    HealthBar(Engine& e);
};
