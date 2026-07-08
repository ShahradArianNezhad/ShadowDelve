#pragma once
#include "shadowDelve/enemies/enemy.hpp"
#include "utilities/types.hpp"
#include "shadowDelve/enemies/skeleton/skeleton.hpp"
#include <cstdint>



class ScytheSkeleton : public Skeleton{
  private:
    void setMode(MODE mode) override;
    void update(double dt) override;
  public:
    ScytheSkeleton(vec2 pos,Engine& e);

};
