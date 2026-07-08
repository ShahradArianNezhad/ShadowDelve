#include "engine/entityManager/component/components.hpp"
#include "engine/meshManager/meshManager.hpp"
#include "engine/scheduleManager/scheduleManager.hpp"
#include "shadowDelve/enemies/skeleton/daggerSkeleton.hpp"
#include "shadowDelve/enemies/skeleton/scytheSkeleton.hpp"
#include "utilities/consts.hpp"
#include "vireon.hpp"
#include "tileMap.hpp"
#include "json.hpp"
#include "utilities/consts.hpp"
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>


void TileMap::init(){
  generateMap();
}

void TileMap::update(double dt){
  for(auto& e:spawnedEnemies)e->update(dt);

  updateBackWallZ();
}

void TileMap::generateMap(){
  makeRoom("./assets/map/spawn.json");
  for(int i=0;i<10;i++){
    auto index =random()%unusedDoors.size();
    if(connectToUnusedDoor(unusedDoors[index]))unusedDoors.erase(unusedDoors.begin()+index);
  }
  sealUnusedDoors();

}


void TileMap::sealUnusedDoors(){
  for(auto& doorPair:unusedDoors){
    if(doorPair.first==UINT32_MAX || doorPair.second==UINT32_MAX)continue;
    auto uv = engine.componentManager.getComponent<Component::UVRECT>(doorPair.first);
    Component::UVRECT sampleUv;
    if(isHorizontalDoor(uv.uvMin)){
      auto left=doorPair.first;
      if(!isLeftDoor(uv.uvMin))left=doorPair.second;
      auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(left);
      auto [gridX,gridY] = positionToGridCords(trans.position);
      if(isGridEmpty(gridX, gridY-1)){
        sampleUv = Component::UVRECT{{1.0f/10,4.0f/10},{2.0f/10,5.0f/10}};
        if(isGridEmpty(gridX-2, gridY) && isGridEmpty(gridX+3, gridY)){
          auto leftCorner = Component::UVRECT{{0,5.0f/10},{1.0f/10,6.0f/10}};
          auto rightCorner = Component::UVRECT{{3.0f/10,5.0f/10},{4.0f/10,6.0f/10}};
          engine.componentManager.setComponent(tileMap[gridX-1][gridY].back().id, leftCorner);
          engine.componentManager.setComponent(tileMap[gridX+2][gridY].back().id, rightCorner);
        }
        
      }else sampleUv = Component::UVRECT{{1.0f/10,0.0f/10},{2.0f/10,1.0f/10}};
    }else{
      auto top=doorPair.first;
      if(!isTopDoor(uv.uvMin))top=doorPair.second;
      auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(top);
      auto [gridX,gridY] = positionToGridCords(trans.position);
      if(isGridEmpty(gridX-1, gridY)){
        sampleUv = Component::UVRECT{{5.0f/10,1.0f/10},{6.0f/10,2.0f/10}};
        if(isGridEmpty(gridX, gridY-2)){
          auto leftCorner = Component::UVRECT{{0,5.0f/10},{1.0f/10,6.0f/10}};
          engine.componentManager.setComponent(tileMap[gridX][gridY-1].back().id, leftCorner);
        }
      }else{
        sampleUv = Component::UVRECT{{0.0f/10,1.0f/10},{1.0f/10,2.0f/10}};
        if(isGridEmpty(gridX, gridY-2)){
          auto rightCorner = Component::UVRECT{{3.0f/10,5.0f/10},{4.0f/10,6.0f/10}};
          engine.componentManager.setComponent(tileMap[gridX][gridY-1].back().id, rightCorner);
        }
      }
    }
    engine.componentManager.setComponent(doorPair.first, sampleUv);
    engine.componentManager.setComponent(doorPair.second, sampleUv);
  }
  unusedDoors.clear();
}


std::pair<int,int> TileMap::findMatchingDoorDiff(nlohmann::json& data,DoorPair& pair){
  auto selectedDoor = pair.first;
  auto uvDoor = engine.componentManager.getComponent<Component::UVRECT>(selectedDoor);
  if(isHorizontalDoor(uvDoor.uvMin) && !isLeftDoor(uvDoor.uvMin))selectedDoor=pair.second;
  if(isVerticalDoor(uvDoor.uvMin) && !isTopDoor(uvDoor.uvMin))selectedDoor=pair.second;
  auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(selectedDoor);
  int gridX = std::ceil((trans.position.x)/BLOCKSIZE);
  int gridY = std::ceil((trans.position.y)/BLOCKSIZE);
  int dx=0,dy=0;
  if(isGridEmpty(gridX,gridY-1)){
    for(auto& [tileX,tileYtoUv]:data["tile"].items()){
      for(auto& [tileY,uvArray]:tileYtoUv.items()){
        for(std::string uvText:uvArray){
          vec2 uv = parseUvFromJson(uvText);
          auto x = std::stoi(tileX);
          auto y = std::stoi(tileY);
          if(isLeftDoor(uv)){
            if(!data["tile"][tileX].contains(std::to_string(std::stoi(tileY)+1))){
              dx = gridX-x;
              dy = gridY-y;
            }
          }
        } 
      }
    }
  }else if(isGridEmpty(gridX, gridY+1)){
    for(auto& [tileX,tileYtoUv]:data["tile"].items()){
      for(auto& [tileY,uvArray]:tileYtoUv.items()){
        for(std::string uvText:uvArray){
          vec2 uv = parseUvFromJson(uvText);
          auto x = std::stoi(tileX);
          auto y = std::stoi(tileY);
          if(isLeftDoor(uv)){
            if(!data["tile"][tileX].contains(std::to_string(std::stoi(tileY)-1))){
              dx = gridX-x;
              dy = gridY-y;
            }
          }
        } 
      }
    }
  }else if(isGridEmpty(gridX+1, gridY)){
    for(auto& [tileX,tileYtoUv]:data["tile"].items()){
      for(auto& [tileY,uvArray]:tileYtoUv.items()){
        for(std::string uvText:uvArray){
          vec2 uv = parseUvFromJson(uvText);
          auto x = std::stoi(tileX);
          auto y = std::stoi(tileY);
          if(isTopDoor(uv)){
            if(!data["tile"].contains(std::to_string(std::stoi(tileX)-1)) || !data["tile"][std::to_string(std::stoi(tileX)-1)].contains(tileY) ){
              dx = gridX-x;
              dy = gridY-y;
            }
          }
        } 
      }
    }
  }else if(isGridEmpty(gridX-1, gridY)){
    for(auto& [tileX,tileYtoUv]:data["tile"].items()){
      for(auto& [tileY,uvArray]:tileYtoUv.items()){
        for(std::string uvText:uvArray){
          vec2 uv = parseUvFromJson(uvText);
          auto x = std::stoi(tileX);
          auto y = std::stoi(tileY);
          if(isTopDoor(uv)){
            if(!data["tile"].contains(std::to_string(std::stoi(tileX)+1)) || !data["tile"][std::to_string(std::stoi(tileX)+1)].contains(tileY)){
              dx = gridX-x;
              dy = gridY-y;
            }
          }
        } 
      }
    }
  }else{
    return {INT32_MAX,INT32_MAX};
  }
  return {dx,dy};

}


bool TileMap::roomCollidesWithMap(nlohmann::json& data,int dx,int dy){
  std::vector<int> DoorRow,DoorCol;

  for(auto& [tileX,tileYtoUv]:data["tile"].items()){
    for(auto& [tileY,uvArray]:tileYtoUv.items()){
      for(std::string uvText:uvArray){
        vec2 uv = parseUvFromJson(uvText);
        auto x = std::stoi(tileX);
        auto y = std::stoi(tileY);
        if(isHorizontalDoor(uv))DoorRow.push_back(y+dy);
        else if(isVerticalDoor(uv))DoorCol.push_back(x+dx);
      } 
    }
  }

  for(auto& [tileX,tileYtoUv]:data["tile"].items()){
    for(auto& [tileY,uvArray]:tileYtoUv.items()){
      for(std::string uvText:uvArray){
        vec2 uv = parseUvFromJson(uvText);
        auto x = std::stoi(tileX);
        auto y = std::stoi(tileY);
        if(!isGridEmpty(dx+x,dy+y)){
          bool col=false,row=false;
          for(auto doorcol:DoorCol){if(dx+x == doorcol)col=true;}
          for(auto doorrow:DoorRow){if(dy+y == doorrow)row=true;}
          if(!(col||row))return true;//return connectToUnusedDoor(pair);
        }
      } 
    }
  }
  return false;
}


bool TileMap::connectToUnusedDoor(DoorPair& pair,size_t attempts){
  if(attempts>20)return false;
  auto room = selectRandomRoom();
  auto data = parseJson(room.string());
  auto [dx,dy] = findMatchingDoorDiff(data, pair);
  if(dx==INT32_MAX || dy==INT32_MAX)return false;
  if(dx==0 && dy==0)return connectToUnusedDoor(pair,attempts+1);
  if(roomCollidesWithMap(data, dx, dy))return connectToUnusedDoor(pair,attempts+1);
  makeRoom(room.string(),dx,dy,true);
  return true;
}





int TileMap::getRandomRoomCount(){
  int count=0;
  for(auto& f:std::filesystem::directory_iterator(roomsPath)){
    count++;
  }
  return count;
}

std::filesystem::path TileMap::selectRandomRoom(){
  auto availableRoomsCount = getRandomRoomCount();
  auto selectedRoom = random()%availableRoomsCount;
  auto counter=0;
  for(auto& f:std::filesystem::directory_iterator(roomsPath)){
    if(counter==selectedRoom)return f.path();
    counter++;
  }
  return "";
}

void TileMap::makeRoom(const std::string& path,int dx,int dy,bool hidden){
    auto data = parseJson(path);
    drawTilesFromJson(data,dx,dy,hidden);
    spawnEntitiesFromJson(data,dx,dy,hidden);
    drawLightsFromJson(data,dx,dy,hidden);
}

nlohmann::json TileMap::parseJson(const std::string& path){
  using json = nlohmann::json;
  std::ifstream file;
  file.open(path);
  return json::parse(file);
}


void TileMap::spawnEntitiesFromJson(nlohmann::json& data,int dx,int dy,bool hidden){
    for(auto& [tileX,tileYtoName]:data["entity"].items()){
      for(auto& [tileY,name]:tileYtoName.items()){
        auto gridX = std::stoi(tileX)+dx;
        auto gridY = std::stoi(tileY)+dy;
        enemyMap[gridX][gridY].push_back(Enemy{name});
      }
    }
}


void TileMap::drawLightsFromJson(nlohmann::json& data,int dx,int dy,bool hidden){
    for(auto& [tileX,tileYtoLight]:data["light"].items()){
      for(auto& [tileY,light]:tileYtoLight.items()){
        auto gridX = std::stoi(tileX)+dx;
        auto gridY = std::stoi(tileY)+dy;
        vec3 position = {gridX*BLOCKSIZE - BLOCKSIZE/2,gridY*BLOCKSIZE - BLOCKSIZE/2,10};
        auto id = engine.makeLight(position,{255,200,140},200,0.002);
        //lightMap[gridX][gridY]=id;
      }
    }
}

std::vector<Tile> TileMap::getNearbyTiles(vec2 position,int radius){
  std::vector<Tile> result;
  int gridX = std::ceil((position.x)/BLOCKSIZE);
  int gridY = std::ceil((position.y)/BLOCKSIZE);

  for(int i=gridX-radius;i<=gridX+radius;i++){
    for(int j=gridY-radius;j<=gridY+radius;j++){
      for(auto& tile:tileMap[i][j])result.push_back(tile);
    }
  }

  return result;
}


bool TileMap::isFloor(vec2 uv){
  return (uv.x>=6.0/10 && uv.y<=2.0f/10) || (uv.x<=3.0f/10 && uv.y>=6.0f/10 && uv.y<=7.0f/10) || (uv.x>=1.0f/10 && uv.x<=4.0f/10 && uv.y>=1.0f/10 && uv.y<=3.0f/10)||(uv.x==9.0f/10 && uv.y==7.0f/10);
}

bool TileMap::isCorner(vec2 uv){
  return (uv.x==0 && uv.y>=4.0f/10 && uv.y<=5.0f/10) || (uv.x>=3.0f/10 && uv.x<=5.0f/10 && uv.y==5.0f/10) || (uv.x==5.0f/10 && uv.y==4.0f/10);
}

bool TileMap::isWall(vec2 uv){
  return isBackWall(uv) || isTopWall(uv) || isRightSideWall(uv) || isLeftSideWall(uv) || isCorner(uv);
}

bool TileMap::isBackWall(vec2 uv){
  return uv.x>=1.0f/10 && uv.x<=4.0f/10 && uv.y==0;
}

bool TileMap::isLeftSideWall(vec2 uv){
  return uv.x==5.0f/10 && (uv.y>=0 && uv.y<=3.0f/10);
}

bool TileMap::isRightSideWall(vec2 uv){
  return uv.x==0 && (uv.y>=0 && uv.y<=3.0f/10);
}

bool TileMap::isTopWall(vec2 uv){
  return (uv.x>=1.0f/10 && uv.x<=4.0f/10 && uv.y==4.0f/10) || (uv.y==5.0f/10 && uv.x>=1.0f/10 && uv.x<=2.0f/10);
}

bool TileMap::isLeftDoor(vec2 uv){
  return (uv.x==6.0f/10 && (uv.y==6.0f/10 || uv.y==3.0f/10));
}
bool TileMap::isRightDoor(vec2 uv){
  return (uv.x==7.0f/10 && (uv.y==6.0f/10 || uv.y==3.0f/10));
}

bool TileMap::isHorizontalDoor(vec2 uv){
  return isRightDoor(uv) || isLeftDoor(uv);
}

bool TileMap::isTopDoor(vec2 uv){
  return (uv.x==6.0f/10) && (uv.y==5.0f/10);
}
bool TileMap::isBottomDoor(vec2 uv){
  return (uv.x==6.0f/10) && (uv.y==4.0f/10);
}

bool TileMap::isVerticalDoor(vec2 uv){
  return (uv.x==6.0f/10 && uv.y>=4.0f/10 && uv.y<=5.0f/10);
}

bool TileMap::isRoomDoor(vec2 uv){
  return (uv.x>=6.0f/10 && uv.x<=7.0f/10 && uv.y==3.0f/10) || (uv.x>=6.0f/10 && uv.x<=8.0f/10 && uv.y>=4.0f/10 && uv.y<=5.0f/10);
}

bool TileMap::isDoor(vec2 uv){
  return  isHorizontalDoor(uv)||isVerticalDoor(uv);
}

bool TileMap::isTopLeftCorner(vec2 uv){
  return (uv.x==0 && uv.y==5.0f/10) || (uv.x==4.0f/10 && uv.y==5.0f/10);
}

bool TileMap::isTopRightCorner(vec2 uv){
  return (uv.x==3.0f/10 && uv.y==5.0f/10) || (uv.x==5.0f/10 && uv.y==5.0f/10);
}


vec2 TileMap::parseUvFromJson(std::string& uvText){
  vec2 uv;
  std::string x = uvText.substr(1,uvText.find(",")-1);
  uvText.erase(0,uvText.find(",")+1);
  std::string y = uvText.substr(0,uvText.find(","));
  uvText.erase(0,uvText.find(",")+1);
  uv.x = std::stof(x);
  uv.y = std::stof(y);
  return uv;
}

bool TileMap::isSideTorch(vec2 uv){
  return uv.x==1.0f/10 && uv.y==9.0f/10;
}

bool TileMap::isHorizontalTorch(vec2 uv){
  return uv.x==0.0f/10 && uv.y==9.0f/10;
}


bool TileMap::parseFlipHFromJson(std::string& uvText){
          std::string h = uvText.substr(0,uvText.find(","));
          uvText.erase(0,uvText.find(",")+1);
          if(h=="true")return true;
          return false;
}


bool TileMap::parseFlipVFromJson(std::string& uvText){
          std::string v = uvText.substr(0,uvText.length()-1);
          uvText.erase(0,uvText.length()-1);
          if(v=="true")return true;
          return false;
}

bool TileMap::isGridEmpty(int gridX,int gridY){
  return !tileMap.contains(gridX) || !tileMap[gridX].contains(gridY) || tileMap[gridX][gridY].size()==0;
}

bool TileMap::hasEnemy(int gridX,int gridY){
  return enemyMap.contains(gridX) && enemyMap[gridX].contains(gridY) && enemyMap[gridX][gridY].size()!=0;
}



void TileMap::drawTilesFromJson(nlohmann::json& data,int dx,int dy,bool hidden){
  std::unordered_map<int,std::unordered_map<int,bool>> changed;
  for(auto& [tileX,tileYtoUv]:data["tile"].items()){
    for(auto& [tileY,uvArray]:tileYtoUv.items()){
      bool wallPlaced = false;
      size_t z=4;
      for(std::string uvText:uvArray){
        vec2 uv = parseUvFromJson(uvText);
        bool flip_h = parseFlipHFromJson(uvText);
        bool flip_v = parseFlipVFromJson(uvText);
        auto gridX = std::stoi(tileX) + dx;
        auto gridY = std::stoi(tileY) + dy;

        if(!changed[gridX].contains(gridY) && !isGridEmpty(gridX,gridY))break;
        vec3 position = {gridX*BLOCKSIZE - BLOCKSIZE/2,gridY*BLOCKSIZE - BLOCKSIZE/2,0};

        if(isSideTorch(uv)){
          if(flip_v)position.x-=(BLOCKSIZE/2.0)-3;
          else position.x+=(BLOCKSIZE/2.0)-3;
        }
        if(isFloor(uv))position.z=FLOOR_LAYER;
        else if(isWall(uv)){
          wallPlaced=true;
          position.z=WALL_LAYER;
        }
        else if(isDoor(uv))position.z=DOOR_LAYER;
        else if(isHorizontalTorch(uv)&&!wallPlaced)position.z=0;
        else position.z=z;

        auto id =engine.makeSprite(position,"./assets/Dungeon_Tileset.png",uv,{uv.x+1/10.0,uv.y+1/10.0});
        if(hidden){
          auto render = engine.componentManager.getComponent<Component::RENDER>(id);
          render.visible=false;
          engine.componentManager.setComponent(id, render);
        }
        changed[gridX][gridY]=true;

        if(isRoomDoor(uv)){
          if(unusedDoors.size()==0)unusedDoors.emplace_back(id,UINT32_MAX);
          else{
            bool foundPair=false;
            for(auto& doorPair:unusedDoors){
              if(doorPair.second==UINT32_MAX){
                auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(doorPair.first);
                auto firstX = std::ceil(trans.position.x/BLOCKSIZE);
                auto firstY = std::ceil(trans.position.y/BLOCKSIZE);
                auto delta = std::abs(firstX-gridX) + std::abs(firstY-gridY);
                if(delta!=1)continue;
                doorPair.second=id;
                foundPair=true;
                break;
              }
            }
            if(!foundPair)unusedDoors.emplace_back(id,UINT32_MAX);
          }
        }

        auto type = TileType::Floor;
        auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(id);

        if(flip_h)trans.scale.y*=-1;
        if(flip_v)trans.scale.x*=-1;


        addColliderForTile(uv,type,id);
        engine.componentManager.setComponent(id, trans);
        tileMap[gridX][gridY].emplace_back(id,type);
      }
    }
  }

}




void TileMap::addColliderForTile(vec2 uv,TileType& type,EntityId id){
  if(isBackWall(uv)){
    engine.componentManager.setComponent(id, Component::RECTCOLLIDER{{0,0},{(BLOCKSIZE/2)-2,BLOCKSIZE/64},0});
    type=TileType::Wall;
  }else if(isLeftSideWall(uv)){
    engine.componentManager.setComponent(id, Component::RECTCOLLIDER{{-(BLOCKSIZE/4),0},{(BLOCKSIZE/4)-2,BLOCKSIZE/2},0});
    type=TileType::Wall;
  }else if(isRightSideWall(uv)){
    engine.componentManager.setComponent(id, Component::RECTCOLLIDER{{(BLOCKSIZE/4),0},{(BLOCKSIZE/4)-2,BLOCKSIZE/2},0});
    type=TileType::Wall;
  }else if(isTopWall(uv)){
    engine.componentManager.setComponent(id, Component::RECTCOLLIDER{{0,-(BLOCKSIZE/4)-1},{BLOCKSIZE/2,(BLOCKSIZE/4)-3},0});
    type=TileType::Wall;
  }else if(isHorizontalDoor(uv)){
    engine.componentManager.setComponent(id, Component::RECTCOLLIDER{{0,-(BLOCKSIZE/4)-1},{BLOCKSIZE/2,(BLOCKSIZE/4)-3},0});
    type=TileType::Wall;
  }else if(isVerticalDoor(uv)){
    engine.componentManager.setComponent(id, Component::RECTCOLLIDER{{0,0},{(BLOCKSIZE/4)-5,BLOCKSIZE/2},0});
    type=TileType::Wall;
  }else if(isTopLeftCorner(uv)){
    engine.componentManager.setComponent(id, Component::RECTCOLLIDER{{0,-(BLOCKSIZE/4)-1},{(BLOCKSIZE/2)-2,(BLOCKSIZE/4)-3},0});
    type=TileType::Wall;
  }else if(isTopRightCorner(uv)){
    engine.componentManager.setComponent(id, Component::RECTCOLLIDER{{0,-(BLOCKSIZE/4)-1},{(BLOCKSIZE/2)-2,(BLOCKSIZE/4)-3},0});
    type=TileType::Wall;
  }else return;
  if(showCollider){
    auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(id);
    auto collider = engine.componentManager.getComponent<Component::RECTCOLLIDER>(id);
    engine.makeRect({trans.position.x+collider.offset.x,trans.position.y+collider.offset.y,ENTITY_LAYER+2},collider.scale);
  }
}

std::pair<int,int> TileMap::positionToGridCords(vec2 position){
  int gridX = std::ceil((position.x)/BLOCKSIZE);
  int gridY = std::ceil((position.y)/BLOCKSIZE);
  return {gridX,gridY};
}



DoorPair TileMap::getDoorPair(EntityId door){
  auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(door);
  auto uv1 = engine.componentManager.getComponent<Component::UVRECT>(door);
  if(!isDoor(uv1.uvMin))LOG_WARN("get door pair called when argument is not a door");
  auto [gridX,gridY] = positionToGridCords(trans.position);

  if(isHorizontalDoor(uv1.uvMin)){
    for(auto tile:tileMap[gridX+1][gridY]){
      auto uv2 = engine.componentManager.getComponent<Component::UVRECT>(tile.id);
      if(isDoor(uv2.uvMin))return DoorPair{door,tile.id};
    };
    for(auto tile:tileMap[gridX-1][gridY]){
      auto uv2 = engine.componentManager.getComponent<Component::UVRECT>(tile.id);
      if(isDoor(uv2.uvMin))return DoorPair{door,tile.id};
    };
  }else if(isVerticalDoor(uv1.uvMin)){
    for(auto tile:tileMap[gridX][gridY+1]){
      auto uv2 = engine.componentManager.getComponent<Component::UVRECT>(tile.id);
      if(isDoor(uv2.uvMin))return DoorPair{door,tile.id};
    };
    for(auto tile:tileMap[gridX][gridY-1]){
      auto uv2 = engine.componentManager.getComponent<Component::UVRECT>(tile.id);
      if(isDoor(uv2.uvMin))return DoorPair{door,tile.id};
    };
  }
  return DoorPair{door,UINT32_MAX};
}

void TileMap::revealTiles(vec2 gridCoords,std::vector<vec2>& visited){
  bool shouldEnd=false;
  for(auto visit:visited){
    if(visit==gridCoords)return;
  }
  visited.push_back(gridCoords);
  if(!isGridEmpty(gridCoords.x, gridCoords.y)){
    auto render = engine.componentManager.getComponent<Component::RENDER>(tileMap[gridCoords.x][gridCoords.y][0].id);
    if(render.visible==true)return;
    for(auto tile:tileMap[gridCoords.x][gridCoords.y]){
      auto render = engine.componentManager.getComponent<Component::RENDER>(tile.id);
      render.visible=true;
      engine.componentManager.setComponent(tile.id, render);
      auto uv = engine.componentManager.getComponent<Component::UVRECT>(tile.id);
      if(isWall(uv.uvMin) || isDoor(uv.uvMin))shouldEnd=true;
    }
    if(hasEnemy(gridCoords.x, gridCoords.y)){
      for(auto enemy:enemyMap[gridCoords.x][gridCoords.y]){
        if(enemy.type=="skeleton"){
          int num = random()%3;
          if(num>=1)spawnedEnemies.emplace_back(std::make_unique<ScytheSkeleton>(vec2{gridCoords.x*BLOCKSIZE,gridCoords.y*BLOCKSIZE},engine));
          else spawnedEnemies.emplace_back(std::make_unique<DaggerSkeleton>(vec2{gridCoords.x*BLOCKSIZE,gridCoords.y*BLOCKSIZE},engine));
        }
      }
    }
    if(shouldEnd)return;
  }
  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      if (dx == 0 && dy == 0) continue;
      revealTiles({gridCoords.x + dx, gridCoords.y + dy},visited);
    }
  }
}

void TileMap::revealFromDoor(DoorPair& pair,vec2 from){
  auto [fromX,fromY] = positionToGridCords(from);
  auto uv1 = engine.componentManager.getComponent<Component::UVRECT>(pair.first);
  auto uv2 = engine.componentManager.getComponent<Component::UVRECT>(pair.second);
  if(isHorizontalDoor(uv1.uvMin)){
    EntityId left = pair.first;
    int y=1;
    if(isRightDoor(uv1.uvMin))left=pair.second;
    auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(left);
    auto [doorX,doorY] = positionToGridCords(trans.position);
    if(trans.position.y<from.y)y=-1;
    std::vector<vec2> visited;
    revealTiles({doorX,doorY+y},visited);
    revealTiles({doorX+1,doorY+y},visited);
  }else{
    EntityId top = pair.first;
    int x=1;
    if(isBottomDoor(uv1.uvMin))top=pair.second;
    auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(top);
    auto [doorX,doorY] = positionToGridCords(trans.position);
    if(trans.position.x<from.x)x=-1;
    std::vector<vec2> visited;
    revealTiles({doorX+x,doorY},visited);
    revealTiles({doorX+x,doorY+1},visited);
  }

}


void TileMap::toggleDoor(EntityId door,vec2 from){
  auto uv = engine.componentManager.getComponent<Component::UVRECT>(door);
  auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(door);
  auto collider = engine.componentManager.getComponent<Component::RECTCOLLIDER>(door);
  auto [gridX,gridY] = positionToGridCords(trans.position);
  for(auto& tile:tileMap[gridX][gridY]){
    if(tile.id==door){
      tile.type=TileType::Floor;
      break;
    }
  }

  if(isLeftDoor(uv.uvMin)){
    if(trans.position.y<from.y){
      uv.uvMin = {7.0f/10,5.0f/10};
      uv.uvMax = {8.0f/10,6.0f/10};
      trans.position.y-=BLOCKSIZE/3.0;
    }else{
      uv.uvMin = {7.0f/10,4.0f/10};
      uv.uvMax = {8.0f/10,5.0f/10};
      trans.position.y+=BLOCKSIZE/3.0;
    }
  }else if(isRightDoor(uv.uvMin)){
    if(trans.position.y<from.y){
      uv.uvMin = {8.0f/10,5.0f/10};
      uv.uvMax = {9.0f/10,6.0f/10};
      trans.position.y-=BLOCKSIZE/3.0;
    }else{
      uv.uvMin = {8.0f/10,4.0f/10};
      uv.uvMax = {9.0f/10,5.0f/10};
      trans.position.y+=BLOCKSIZE/3.0;
    }
  }else if(isTopDoor(uv.uvMin)){
    uv.uvMin = {6.0f/10,3.0f/10};
    uv.uvMax = {7.0f/10,4.0f/10};
    trans.position.y-=BLOCKSIZE/3.0;
    trans.position.z=WALL_LAYER+1;
    if(trans.position.x>from.x){
      trans.position.x+=BLOCKSIZE/2.0;
    }else{
      trans.scale.x*=-1;
      trans.position.x-=BLOCKSIZE/2.0;
    }
  }else if(isBottomDoor(uv.uvMin)){
    uv.uvMin = {7.0f/10,3.0f/10};
    uv.uvMax = {8.0f/10,4.0f/10};
    trans.position.y+=BLOCKSIZE/3.0;
    if(trans.position.x>from.x){
      trans.scale.x*=-1;
      trans.position.x+=BLOCKSIZE/2.0;
    }else{
      trans.position.x-=BLOCKSIZE/2.0;
    }
  }else{
    return;
  }
  engine.componentManager.setComponent(door, uv);
  engine.componentManager.setComponent(door, trans);
  engine.componentManager.setComponent(door, collider);

}

void TileMap::deleteEntity(EntityId id){
  engine.entityManager.deleteEntity(id);
  deleteFromTilemap(id);
}

void TileMap::deleteFromTilemap(EntityId id){
  auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(id);
  auto [gridX,gridY] = positionToGridCords(trans.position);
  for(int i=0;i<tileMap[gridX][gridY].size();i++){
    if(tileMap[gridX][gridY][i].id==id){
      tileMap[gridX][gridY].erase(tileMap[gridX][gridY].begin()+i);
      return;
    }
  }
}


void TileMap::toggleDoor(DoorPair& pair,vec2 from){
  if(pair.first==UINT32_MAX || pair.second==UINT32_MAX)return;
  else if(pair.first==pair.second)return;
  auto firstTrans = engine.componentManager.getComponent<Component::TRANSFORM>(pair.first).position;
  auto secondTrans = engine.componentManager.getComponent<Component::TRANSFORM>(pair.second).position;
  auto firstUv = engine.componentManager.getComponent<Component::UVRECT>(pair.first);
  auto secondUv = engine.componentManager.getComponent<Component::UVRECT>(pair.second);
  if((isHorizontalDoor(firstUv.uvMin) && isHorizontalDoor(secondUv.uvMin) && firstTrans.y==secondTrans.y)
      || (isVerticalDoor(firstUv.uvMin)&&isVerticalDoor(secondUv.uvMin)&&firstTrans.x==secondTrans.x)){
    revealFromDoor(pair, from);
    toggleDoor(pair.first,from);
    toggleDoor(pair.second,from);
  }
}


void TileMap::updateBackWallZ(){
  auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(player);
  auto [gridX,gridY] = positionToGridCords(trans.position);

  for(int i=-1;i<=1;i++){
    for(int j=-1;j<=1;j++){
      for(auto tile:tileMap[gridX+i][gridY+j]){
        if(tile.type==TileType::Wall){
          auto uv = engine.componentManager.getComponent<Component::UVRECT>(tile.id);
          if(isBackWall(uv.uvMin)){
            auto wallTrans = engine.componentManager.getComponent<Component::TRANSFORM>(tile.id);
            auto lastTile = tileMap[gridX+i][gridY+j].back().id;
            auto lastTileUv = engine.componentManager.getComponent<Component::UVRECT>(lastTile);
            auto lastTileTrans = engine.componentManager.getComponent<Component::TRANSFORM>(lastTile);
            if(trans.position.y < wallTrans.position.y){
              wallTrans.position.z=ENTITY_LAYER+1;
              if(isHorizontalTorch(lastTileUv.uvMin))lastTileTrans.position.z=ENTITY_LAYER+2;
            }else{
              wallTrans.position.z=WALL_LAYER;
              if(isHorizontalTorch(lastTileUv.uvMin))lastTileTrans.position.z=WALL_LAYER+1;
            }
            engine.componentManager.setComponent(tile.id, wallTrans);
            if(isHorizontalTorch(lastTileUv.uvMin))engine.componentManager.setComponent(lastTile, lastTileTrans);
          }
        }
      }
    }
  }
}


bool TileMap::hasWall(vec2 gridCords){
  for(auto& tile : tileMap[gridCords.x][gridCords.y]){
    auto uv = engine.componentManager.getComponent<Component::UVRECT>(tile.id);
    if(isWall(uv.uvMin))return true;
  }
  return false;
}

bool TileMap::isWalkable(vec2 gridCords){
  return !isGridEmpty(gridCords.x, gridCords.y) && !hasWall(vec2{gridCords.x,gridCords.y});
}
