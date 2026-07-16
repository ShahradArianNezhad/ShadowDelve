#pragma once
#include "player/player.hpp"
#include "shadowDelve/tileMap/tileMap.hpp"
#include "shadowDelve/ui/healthBar.hpp"
#include "vireon.hpp"

class ShadowDelve : public Game{
  private:
    TileMap tileMap{engine};
    Player player{engine,tileMap};
    HealthBar bar{engine};

    void init();
    void update(double);
};
