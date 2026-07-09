#include "enemy.hpp"
#include "engine/scheduleManager/scheduleManager.hpp"
#include "shadowDelve/tileMap/tileMap.hpp"
#include "utilities/consts.hpp"
#include <functional>
#include <queue>
#include <unordered_set>

EnemyEntity::EnemyEntity(Engine& e):engine(e){}


void EnemyEntity::applyVelocity(vec2 v){
    auto comp = engine.componentManager.getComponent<Component::TRANSFORM>(id);
    auto nearbyTiles = TileMap::getNearbyTiles(comp.position);

    comp.position.x+=v.x;
    engine.componentManager.setComponent(id, comp);
    for(auto& tile:nearbyTiles){
      if(tile.type==TileType::Wall && engine.rectIsColliding(tile.id, id)){
        comp.position.x-=v.x;
        engine.componentManager.setComponent(id, comp);
      }
    }

    comp.position.y+=v.y;
    engine.componentManager.setComponent(id, comp);
    for(auto& tile:nearbyTiles){
      if(tile.type==TileType::Wall && engine.rectIsColliding(tile.id, id)){
        comp.position.y-=v.y;
        engine.componentManager.setComponent(id, comp);
      }
    }


    if(showCollider){
      auto colTrans = engine.componentManager.getComponent<Component::TRANSFORM>(collider);
      auto colRect = engine.componentManager.getComponent<Component::RECTCOLLIDER>(id);
      colTrans.position = {comp.position.x+colRect.offset.x,comp.position.y+colRect.offset.y,comp.position.z-1};
      colTrans.scale = vec3{colRect.scale,0};
      engine.componentManager.setComponent(collider, colTrans);
    }

}


void EnemyEntity::updateFacingDirection(){
  auto comp = engine.componentManager.getComponent<Component::TRANSFORM>(id);
  if(velocity.x < 0) comp.scale.x = -std::abs(comp.scale.x);
  else if(velocity.x > 0) comp.scale.x = std::abs(comp.scale.x);
  engine.componentManager.setComponent(id, comp);
}


std::vector<vec2> EnemyEntity::findPath(vec2 position){
  struct vec2Hash{
    std::size_t operator()(const vec2& v) const {
      std::size_t h1 = std::hash<int>{}(v.x);
      std::size_t h2 = std::hash<int>{}(v.y);
      return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1>>2));
    }
  };
  auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(id);
  auto [gridX,gridY] = TileMap::positionToGridCords(trans.position);
  auto [gridXGoal,gridYGoal] = TileMap::positionToGridCords(position);
  vec2 goal{gridXGoal,gridYGoal};
  vec2 start{gridX,gridY};
  std::unordered_map<vec2,vec2,vec2Hash> parentMap;
  std::unordered_set<vec2,vec2Hash> visited;
  std::queue<vec2> queue;
  queue.push(vec2{gridX,gridY});
  visited.insert(vec2{gridX,gridY});
  while(!queue.empty()){
    vec2 curr = queue.front();
    queue.pop();
    if(curr==goal)break;
    for(int i=-1;i<=1;i++){
      for(int j=-1;j<=1;j++){
        if(!visited.count(vec2{curr.x+i,curr.y+j})){
          if(i==0&&j==0)continue;
          if(!isTileWalkable(vec2{curr.x+i,curr.y+j}) && vec2{curr.x+i,curr.y+j}!=goal)continue;
          if(i!=0&&j!=0&&(!isTileWalkable(vec2{curr.x+i,curr.y})||!isTileWalkable(vec2{curr.x,curr.y+j})))continue;
          visited.insert(vec2{curr.x+i,curr.y+j});
          queue.push(vec2{curr.x+i,curr.y+j});
          parentMap[vec2{curr.x+i,curr.y+j}]=curr;
        }
      }
    }
  }
  if(!visited.count(goal))return {};
  std::vector<vec2> path;
  for(vec2 v=goal;v!=start;v=parentMap[v])path.push_back(v);
  reverse(path.begin(),path.end());
  return path;
}


bool EnemyEntity::isTileWalkable(vec2 tileCords){
  if(TileMap::isGridEmpty(tileCords.x, tileCords.y))return false;
  for(auto& tile: TileMap::tileMap[tileCords.x][tileCords.y]){
    auto uv = engine.componentManager.getComponent<Component::UVRECT>(tile.id);
    if(TileMap::isWall(uv.uvMin))return false;
  }
  return true;
}

vec2 EnemyEntity::getRandomMove(){
  while(true){
    int deltaX = (random()%3)-1;
    int deltaY = (random()%3)-1;
    if(deltaX==0 && deltaY==0)continue;
    auto trans = engine.componentManager.getComponent<Component::TRANSFORM>(id);
    auto [gridX,gridY] = TileMap::positionToGridCords(trans.position);
    if(!isTileWalkable(vec2{gridX+deltaX,gridY+deltaY}))continue;
    return vec2{(gridX+deltaX)*BLOCKSIZE,(gridY+deltaY)*BLOCKSIZE};
  }
}


