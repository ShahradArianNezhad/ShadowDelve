#pragma once
#include "engine/entityManager/component/components.hpp"
#include "game/game.hpp"
#include "json.hpp"

struct EditorTile{
  EntityId id=UINT32_MAX;
  vec2 uvMin={0,0};
  bool h,v;
};


struct EditorEntity{
  EntityId id=UINT32_MAX;
  std::string name;
};

using json = nlohmann::json;



class MapEditor : public Game{
public:
  static constexpr int blocksize=32;
  float uvSizeX=10.0;
  float uvSizeY=10.0;
  bool h=false,v=false;
  std::string mode = "tile";
  std::string spriteFile = "./assets/Dungeon_Tileset.png";
  std::unordered_map<long,std::unordered_map<long,std::vector<EditorTile>>> tileMap;
  std::unordered_map<long,std::unordered_map<long,EditorEntity>> entityMap;
  std::unordered_map<long,std::unordered_map<long,EntityId>> lightMap;
  vec2 cantDeleteGrid;

  vec2 uvMin={0,0};
  vec2 uvMax={1.0/10,1.0/10};
  EntityId selection = engine.makeSprite({0,0,10},spriteFile,uvMin,uvMax);
  EntityId selectionBox = engine.makeRect({0,0,20}, {200,200},Layer::UI);
  EntityId middle = engine.makeRect({0,0,0}, {blocksize/2,blocksize/2});
  std::vector<EntityId> selectionItems;


  void init() override;
  void update(double) override; 
  void setModeEditor();
  void selectTile();
  void openSelectMenu();
  void closeSelectMenu();
  void nextSelection();
  void clearScreen();
  void loadTileMap();
  void writeTileMap();
  void nextTile(EntityId entity);
  void prevTile(EntityId entity);
  void placeAtMousePos();
  void deleteAtMousePos();
  void showPreviewAtMousePos(vec2 pos);
};

