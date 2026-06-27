#pragma once
#include "player/player.hpp"
#include "shadowDelve/tileMap/tileMap.hpp"
#include "vireon.hpp"

class ShadowDelve : public Game{
  private:
    TileMap tileMap{engine};
    Player player{engine,tileMap};

    void init();
    void update(double);
};
