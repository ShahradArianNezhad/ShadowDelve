#pragma once
#include "shadowDelve/enemies/enemy.hpp"
#include "shadowDelve/enemies/skeleton/skeleton.hpp"




class DaggerSkeleton : public Skeleton{

  private:
    void setMode(MODE mode) override;
    void update(double dt) override;

  public:
    DaggerSkeleton(vec2 pos,Engine& e);

};
