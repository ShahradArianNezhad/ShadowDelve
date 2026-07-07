#include "shadowDelve/shadowDelve.hpp"
#include "mapEditor/mapEditor.hpp"

constexpr bool editorMode = true;

int main(){
  if(editorMode){
    MapEditor game;
    game.run();
    return 0;
  }else{
    ShadowDelve game;
    game.run();
    return 0;
  }
}
