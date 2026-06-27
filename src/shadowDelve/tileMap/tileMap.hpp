#pragma once
#include "engine/engine.hpp"
#include "json.hpp"
#include <string>
#include <string_view>
#include <unordered_map>

enum class TileType{
  Wall,
  Floor
};

struct DoorPair{
  EntityId first=UINT32_MAX;
  EntityId second=UINT32_MAX;
};


struct Tile{
  EntityId id;
  TileType type;
};
struct Enemy{
  EntityId id;
};


class TileMap{
  private:
    Engine& engine;
    std::vector<DoorPair> unusedDoors;
    static constexpr std::string_view roomsPath = "./assets/map/rooms";


    nlohmann::json parseJson(const std::string& path);
    void makeRoom(const std::string& path,int dx=0,int dy=0,bool hidden=false);
    bool connectToUnusedDoor(DoorPair& pair,size_t attempts=0);
    bool roomCanBePlaced(nlohmann::json& data,int dx,int dy);
    bool roomCollidesWithMap(nlohmann::json& data,int dx,int dy);
    vec2 parseUvFromJson(std::string& uvText);
    bool isSideTorch(vec2 uv);
    bool parseFlipHFromJson(std::string& uvText);
    bool parseFlipVFromJson(std::string& uvText);
    void addColliderForTile(vec2 uv,TileType& type,EntityId id);
    int getRandomRoomCount();
    std::filesystem::path selectRandomRoom();

    void drawTilesFromJson(nlohmann::json& data,int dx=0,int dy=0,bool hidden=false);
    void drawLightsFromJson(nlohmann::json& data,int dx=0,int dy=0,bool hidden=false);
    void spawnEntitiesFromJson(nlohmann::json& data,int dx=0,int dy=0,bool hidden=false);
    void revealTiles(vec2 pos,std::vector<vec2>& visited);
    void revealFromDoor(DoorPair& pair,vec2 from);
    void deleteEntity(EntityId id);
    void sealUnusedDoors();
    std::pair<int,int> findMatchingDoorDiff(nlohmann::json& data,DoorPair& pair);


  public:
    static inline std::unordered_map<int, std::unordered_map<int,std::vector<Tile>>> tileMap;
    static inline std::unordered_map<int, std::unordered_map<int,std::vector<Enemy>>> enemyMap;
    [[nodiscard]] static std::vector<Tile> getNearbyTiles(vec2 position,int radius=1);
    static bool isGridEmpty(int gridX,int gridY);
    static bool hasEnemy(int gridX,int gridY);
    static bool isBackWall(vec2 uv);
    static bool isTopWall(vec2 uv);
    static bool isLeftSideWall(vec2 uv);
    static bool isRightSideWall(vec2 uv);
    static bool isDoor(vec2 uv);
    static bool isRoomDoor(vec2 uv);
    static bool isTopLeftCorner(vec2 uv);
    static bool isTopRightCorner(vec2 uv);
    static bool isHorizontalDoor(vec2 uv);
    static bool isVerticalDoor(vec2 uv);
    static bool isLeftDoor(vec2 uv);
    static bool isRightDoor(vec2 uv);
    static bool isTopDoor(vec2 uv);
    static bool isBottomDoor(vec2 uv);
    static bool isCorner(vec2 uv);
    static bool isWall(vec2 uv);
    DoorPair getDoorPair(EntityId door);
    std::pair<int,int> positionToGridCords(vec2 pos);
    void toggleDoor(DoorPair& pair,vec2 from={0,0});
    void toggleDoor(EntityId door,vec2 from={0,0});


    TileMap(Engine& e):engine(e){};
    void init();
    void update(double);
    void generateMap();
};
