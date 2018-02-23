#define LIB_NAME "DAABBCC"
#define MODULE_NAME "daabbcc"

#include <dmsdk/sdk.h>
#include <iostream>
#include <algorithm>
#include <math.h>
#include <aabb/AABB.h>

#define TINYC2_IMPLEMENTATION
#include "aabb/tinyc2.h"

using namespace std;
using namespace aabb;

/*TODO: Camera update. */

Tree  * treeObjectPointer; // Tree Pointer
vector<Tree  *> treeArr; // Tree Array
vector<string> treeTxtArr; // Tree name array for name conversation

float _normalx;
float _normaly;

unsigned int particleCount = 0; // Particle IDs

//Debug vector values
void pprint (const vector<unsigned int>& v){
  cout << "HIT: \n";
  for (int i=0; i<v.size();i++){
    cout << v[i] << endl;
  }
}

void warning(string _name){
  cout << "WARNING!! : " << _name << " tree not found. Check your tree name. \n";
}

//Tree name conversation
pair<bool, int> checkTreeName(string _name)
{
  long pos = find(treeTxtArr.begin(), treeTxtArr.end(), _name) - treeTxtArr.begin();
  if( pos < treeTxtArr.size() ) return std::make_pair(true, pos);
  warning(_name);
  return std::make_pair(false, 0);
}

//Bounds
pair< vector<double> , vector<double> > calculateBounds(double x, double y, double w, double h)
{
  double xl = x - (w/2);
  double yb = y - (h/2);
  double xr = x + (w/2);
  double yt = y + (h/2);
  vector<double> lowerBound;
  lowerBound.push_back(xl);
  lowerBound.push_back(yb);
  vector<double> upperBound;
  upperBound.push_back(xr);
  upperBound.push_back(yt);
  return std::make_pair(lowerBound, upperBound);
}

//Constructor (non-periodic)
static int createTree(lua_State* L)
{
  int top = lua_gettop(L);
  string _name =  luaL_checkstring(L, 1);
  unsigned int _dimension =  luaL_checknumber(L, 2);
  double _skinThickness = luaL_checknumber(L, 3);
  unsigned int _nParticles = luaL_checknumber(L, 4);
  treeObjectPointer = new Tree(_dimension, _skinThickness, _nParticles);
  treeArr.push_back (treeObjectPointer);
  treeTxtArr.push_back(_name);
  return 0;
}

//Query the tree to find candidate interactions for a particle ID.
static int queryID(lua_State* L){
  int top = lua_gettop(L);
  string _name =  luaL_checkstring(L, 1);
  unsigned int _id =  luaL_checknumber(L, 2);
  vector<unsigned int> particles;
  pair<bool, int> _result = checkTreeName(_name);
  if( _result.first) {
    particles = treeArr[_result.second]->query(_id);
  }
  lua_createtable(L, particles.size(), 0);
  int newTable = lua_gettop(L);
  for(int i=0; i < particles.size(); i++) {
    lua_pushnumber(L, particles[i]);
    lua_rawseti(L, newTable, i + 1);
  }
  assert(top + 1 == lua_gettop(L));
  return 1;
}

//Query the tree to find candidate interactions for an AABB.
static int queryAABB(lua_State* L){
  int top = lua_gettop(L);
  string _name =  luaL_checkstring(L, 1);
  double x = luaL_checknumber(L, 2);
  double y = luaL_checknumber(L, 3);
  double w = luaL_checknumber(L, 4);
  double h = luaL_checknumber(L, 5);
  vector<unsigned int> particles;
  pair<bool, int> _result = checkTreeName(_name);
  if( _result.first) {
    pair< vector<double> , vector<double>  > _bounds = calculateBounds(x,y,w,h);
    AABB aabb(_bounds.first, _bounds.second);
    particles = treeArr[_result.second]->query(aabb);
  }
  lua_createtable(L, particles.size(), 0);
  int newTable = lua_gettop(L);
  for(int i=0; i < particles.size(); i++) {
    lua_pushnumber(L, particles[i]);
    lua_rawseti(L, newTable, i + 1);
  }
  assert(top + 1 == lua_gettop(L));
  return 1;
}

//Insert a particle into the tree (point particle)
static int insertCircle(lua_State* L){
  int top = lua_gettop(L);
  string _name = luaL_checkstring(L, 1);
  double _radius = luaL_checknumber(L, 2);
  double x = luaL_checknumber(L, 3);
  double y = luaL_checknumber(L, 4);
  unsigned int _index = particleCount;
  pair<bool, int> _result = checkTreeName(_name);
  if( _result.first) {
    vector<double> _position;
    _position.push_back(x);
    _position.push_back(y);
    treeArr[_result.second]->insertParticle(particleCount, _position, _radius);
    particleCount++;
  }
  lua_pushinteger(L, _index);
  assert(top + 1 == lua_gettop(L));
  return 1;
}

//Insert a particle into the tree (arbitrary shape with bounding box)
static int insertRect(lua_State* L){
  int top = lua_gettop(L);
  string _name = luaL_checkstring(L, 1);
  double x = luaL_checknumber(L, 2);
  double y = luaL_checknumber(L, 3);
  double w = luaL_checknumber(L, 4);
  double h = luaL_checknumber(L, 5);
  unsigned int _index = particleCount;
  pair<bool, int> _result = checkTreeName(_name);
  if( _result.first ) {
    pair< vector<double> , vector<double>  > _bounds = calculateBounds(x,y,w,h);
    treeArr[_result.second]->insertParticle(particleCount, _bounds.first, _bounds.second);
    particleCount++;
  }
  lua_pushinteger(L, _index);
  assert(top + 1 == lua_gettop(L));
  return 1;
}

//Remove a particle from the tree.
static int removeAABB(lua_State* L){
  int top = lua_gettop(L);
  string _name = luaL_checkstring(L, 1);
  unsigned int _id = luaL_checknumber(L, 2);
  pair<bool, int> _result = checkTreeName(_name);
  if( _result.first ) {
    treeArr[_result.second]->removeParticle(_id);
  }
  return 0;
}

//Update the tree if a particle moves outside its fattened AABB.
static int updateCircle(lua_State* L){
  int top = lua_gettop(L);
  string _name = luaL_checkstring(L, 1);
  unsigned int _id = luaL_checknumber(L, 2);
  double _radius = luaL_checknumber(L, 3);
  double x = luaL_checknumber(L, 4);
  double y = luaL_checknumber(L, 5);
  pair<bool, int> _result = checkTreeName(_name);
  if( _result.first ) {
    vector<double> _position;
    _position.push_back(x);
    _position.push_back(y);
    treeArr[_result.second]->updateParticle(_id, _position, _radius);
  }
  return 0;
}

//Update the tree if a particle moves outside its fattened AABB.
static int updateRect(lua_State* L){
  int top = lua_gettop(L);
  string _name = luaL_checkstring(L, 1);
  unsigned int _id = luaL_checknumber(L, 2);
  double x = luaL_checknumber(L, 3);
  double y = luaL_checknumber(L, 4);
  double w = luaL_checknumber(L, 5);
  double h = luaL_checknumber(L, 6);
  pair<bool, int> _result = checkTreeName(_name);
  if( _result.first ) {
    pair< vector<double> , vector<double>  > _bounds = calculateBounds(x,y,w,h);
    treeArr[_result.second]->updateParticle(_id, _bounds.first, _bounds.second);
  }
  return 0;
}

/*---------------
**** Helpers ****
-----------------*/

//Validate the tree.
static int validateTree(lua_State* L){
  int top = lua_gettop(L);
  string _name = luaL_checkstring(L, 1);
  pair<bool, int> _result = checkTreeName(_name);
  if( _result.first ) {
    treeArr[_result.second]->validate();
  }
  return 0;
}

//Rebuild an optimal tree.
static int rebuildTree(lua_State* L){
  int top = lua_gettop(L);
  string _name = luaL_checkstring(L, 1);
  pair<bool, int> _result = checkTreeName(_name);
  if( _result.first ) {
    treeArr[_result.second]->rebuild();
  }
  return 0;
}

//Get the height of the binary tree..
static int getHeight(lua_State* L){
  int top = lua_gettop(L);
  string _name = luaL_checkstring(L, 1);
  unsigned int _height=0;
  pair<bool, int> _result = checkTreeName(_name);
  if( _result.first ) {
    _height = treeArr[_result.second]->getHeight();
  }
  lua_pushinteger(L, _height);
  assert(top + 1 == lua_gettop(L));
  return 1;
}

//Get the number of nodes in the tree.
static int getNodeCount(lua_State* L){
  int top = lua_gettop(L);
  string _name = luaL_checkstring(L, 1);
  unsigned int _nodeCount=0;
  pair<bool, int> _result = checkTreeName(_name);
  if( _result.first ) {
    _nodeCount = treeArr[_result.second]->getNodeCount();
  }
  lua_pushinteger(L, _nodeCount);
  assert(top + 1 == lua_gettop(L));
  return 1;
}

//Get the number of nodes in the tree.
static int getAABB(lua_State* L){
  int top = lua_gettop(L);
  string _name = luaL_checkstring(L, 1);
  unsigned int _id = luaL_checknumber(L, 2);
  pair<bool, int> _result = checkTreeName(_name);
  if( _result.first ) {
    AABB aabb = treeArr[_result.second]->getAABB(_id);

    double sizew =(aabb.upperBound[0] - aabb.lowerBound[0]);
    double sizeh =(aabb.upperBound[1] - aabb.lowerBound[1]);
    double posx = aabb.upperBound[0] - (sizew/2);
    double posy = aabb.lowerBound[1] + (sizeh/2);

    lua_pushnumber(L, posx);
    lua_pushnumber(L, posy);
    lua_pushnumber(L, sizew);
    lua_pushnumber(L, sizeh);
    assert(top + 4== lua_gettop(L));
    return 4;
  }

  return 0;
}

//Internal
AABB _getAABB(string _name, int _id){
  AABB aabb;
  pair<bool, int> _result = checkTreeName(_name);
  if( _result.first ) {
   aabb = treeArr[_result.second]->getAABB(_id);
 }
 return aabb;
}



float SweptAABB(AABB box1, AABB box2, vector<float> vel1, float& normalx, float& normaly)
{

  float xDistanceEntry, yDistanceEntry;
  float xDistanceExit, yDistanceExit;

  if (vel1[0] > 0.0f) {
    xDistanceEntry = (box2).lowerBound[0] - (box1).upperBound[0];
    xDistanceExit = (box2).upperBound[0] - (box1).lowerBound[0];
  } else {
    xDistanceEntry = (box2).upperBound[0] - (box1).lowerBound[0];
    xDistanceExit = (box2).lowerBound[0] - (box1).upperBound[0];
  }

  if (vel1[1] > 0.0f) {
    yDistanceEntry = (box2).lowerBound[1] - (box1).upperBound[1];
    yDistanceExit = (box2).upperBound[1] - (box1).lowerBound[1];
  } else {
    yDistanceEntry = (box2).upperBound[1] - (box1).lowerBound[1];
    yDistanceExit = (box2).lowerBound[1] - (box1).upperBound[1];
  }

  
  float xEntryTime, yEntryTime;
  float xExitTime, yExitTime;
  
  if (vel1[0] == 0.0f)  {
   if (max(fabsf(xDistanceEntry), fabsf(xDistanceExit)) > (((box1).upperBound[0] - (box1).lowerBound[0]) + ((box2).upperBound[0] - (box2).lowerBound[0]))){
    xEntryTime = 2.0f;
  } else {  
    xEntryTime = -numeric_limits<float>::infinity();
  }

  xExitTime = numeric_limits<float>::infinity();

} else {
 xEntryTime = xDistanceEntry / vel1[0];
 xExitTime = xDistanceExit / vel1[0];
}

if (vel1[1] == 0.0f){
  if (max(fabsf(yDistanceEntry), fabsf(yDistanceExit)) > (((box1).upperBound[1] - (box1).lowerBound[1]) + ((box2).upperBound[1] - (box2).lowerBound[1]))){
    yEntryTime = 2.0f;
  } else {
    yEntryTime = -std::numeric_limits<float>::infinity();
  }

  yExitTime = std::numeric_limits<float>::infinity();
}
else {
  yEntryTime = yDistanceEntry / vel1[1];
  yExitTime = yDistanceExit / vel1[1];
}

float entryTime = max(xEntryTime, yEntryTime);
float exitTime = min(xExitTime, yExitTime);

if (entryTime > exitTime || (xEntryTime < 0.0f && yEntryTime < 0.0f)  || xEntryTime > 1.0f || yEntryTime > 1.0f){
  normalx = 0.0f;
  normaly = 0.0f;
  return 2.0f;
} else {
  if (xEntryTime > yEntryTime) {
    if (xDistanceEntry < 0.0f) {
      normalx = 1.0f;
      normaly = 0.0f;
    } else {
      normalx = -1.0f;
      normaly = 0.0f;
    }
  } else if (yEntryTime > xEntryTime) {
    if (yDistanceEntry < 0.0f) {
      normalx = 0.0f;
      normaly = 1.0f;
    } else {
      normalx = 0.0f;
      normaly = -1.0f;
    }
  }

  _normalx = normalx;
  _normaly = normaly;
  return entryTime;
}
}

//Manifold
static int checkManifold(lua_State* L){
  int top = lua_gettop(L);

  string _name = luaL_checkstring(L, 1);
  int _moving_id = luaL_checkint(L, 2);
  int _other_id = luaL_checkint(L, 3);
  

  unsigned int _nodeCount=0;
  pair<bool, int> _result = checkTreeName(_name);

  if( _result.first ) {

   AABB box1 = _getAABB(_name, _moving_id);
   AABB box2  = _getAABB(_name, _other_id);

   c2AABB aabb;
   /* aabb.min.x =  box1.lowerBound[0];
     aabb.min.y = box1.lowerBound[1];
    aabb.max.x = box1.upperBound[0];
    aabb.max.y =  box1.upperBound[1];
    */
   aabb.min = c2V(box1.lowerBound[0], box1.lowerBound[1]);
   aabb.max = c2V(box1.upperBound[0], box1.upperBound[1]);


   c2AABB aabb2;
   /* aabb2.min.x =  box2.lowerBound[0];
    aabb2.min.y = box2.lowerBound[1];
    aabb2.max.x = box2.upperBound[0];
    aabb2.max.y =  box2.upperBound[1];
   */
   aabb2.min = c2V(box2.lowerBound[0], box2.lowerBound[1]);
   aabb2.max = c2V(box2.upperBound[0], box2.upperBound[1]);
   
   c2Manifold manifold;
   c2AABBtoAABBManifold(aabb, aabb2, &manifold);

   if (manifold.count > 0) {
    lua_pushinteger(L, manifold.count); // manifold.count; 
    lua_pushnumber(L, manifold.depths[0]);

    lua_createtable(L, 2, 0);

    lua_pushstring(L, "x");
    lua_pushnumber(L, manifold.normal.x);
    lua_settable(L, -3);

    lua_pushstring(L, "y");
    lua_pushnumber(L, manifold.normal.y);
    lua_settable(L, -3);

    lua_createtable(L, 2, 0);
   
    lua_pushstring(L, "x");
    lua_pushnumber(L, manifold.contact_points[0].x);
    lua_settable(L, -3);

    lua_pushstring(L, "y");
    lua_pushnumber(L, manifold.contact_points[0].y);
    lua_settable(L, -3);

    assert(top + 4 == lua_gettop(L));
    return 4;

  }
  
}
return 0;
}

// Check simple hit.
static int checkHit(lua_State* L){
  int top = lua_gettop(L);

  string _name = luaL_checkstring(L, 1);
  int _moving_id = luaL_checkint(L, 2);
  int _other_id = luaL_checkint(L, 3);

  pair<bool, int> _result = checkTreeName(_name);
  if( _result.first ) {

   AABB box1 = _getAABB(_name, _moving_id);
   AABB box2  = _getAABB(_name, _other_id);

   c2AABB aabb;
   aabb.min.x =  box1.lowerBound[0];
   aabb.min.y = box1.lowerBound[1];
   aabb.max.x = box1.upperBound[0];
   aabb.max.y =  box1.upperBound[1];

   c2AABB aabb2;
   aabb2.min.x =  box2.lowerBound[0];
   aabb2.min.y = box2.lowerBound[1];
   aabb2.max.x = box2.upperBound[0];
   aabb2.max.y =  box2.upperBound[1];
   
   int hit =  c2AABBtoAABB(aabb, aabb2);
   
   lua_pushinteger(L, hit);
   assert(top + 1 == lua_gettop(L));
   return 1;
 }
 return 0;
}

static int checkSweptCollision(lua_State* L){
  int top = lua_gettop(L);

  string _name = luaL_checkstring(L, 1);
  int _moving_id = luaL_checkint(L, 2);
  int _static_id = luaL_checkint(L, 3);
  float _moving_velocity_x = luaL_checknumber(L, 4);
  float _moving_velocity_y = luaL_checknumber(L, 5);
  float _moving_velocity_normal_x = luaL_checknumber(L, 6);
  float _moving_velocity_normal_y = luaL_checknumber(L, 7);

  unsigned int _nodeCount=0;
  pair<bool, int> _result = checkTreeName(_name);

  if( _result.first ) {

   AABB _moving_aabb = _getAABB(_name, _moving_id);
   AABB _static_aabb = _getAABB(_name, _static_id);
/*
   vector<double> _position;
    _position.push_back(x);
    _position.push_back(y);
*/
   vector<float> _moving_velocity ;
   _moving_velocity.push_back(_moving_velocity_x);
   _moving_velocity.push_back(_moving_velocity_y);
   float _result = SweptAABB(_moving_aabb,_static_aabb, _moving_velocity, _moving_velocity_normal_x, _moving_velocity_normal_y );

   lua_pushnumber(L, _result);
   lua_pushnumber(L, _normalx);
   lua_pushnumber(L, _normaly);
   assert(top + 3 == lua_gettop(L));
   return 3;
 }

  //lua_pushinteger(L, _nodeCount);
  //assert(top + 1 == lua_gettop(L));
 return 0;
}
// Functions exposed to Lua
static const luaL_reg Module_methods[] =
{

  {"checkManifold", checkManifold},
  {"checkHit", checkHit},
  {"checkSweptCollision", checkSweptCollision},
  {"getAABB", getAABB},
  {"rebuildTree", rebuildTree},
  {"validateTree", validateTree},
  {"queryAABB", queryAABB},
  {"queryID", queryID},
  {"updateRect", updateRect},
  {"updateCircle", updateCircle},
  {"getNodeCount", getNodeCount},
  {"getHeight", getHeight},
  {"removeAABB", removeAABB},
  {"insertCircle", insertCircle},
  {"insertRect", insertRect},
  {"createTree", createTree},
  {0, 0}
};

static void LuaInit(lua_State* L)
{
  int top = lua_gettop(L);
  luaL_register(L, MODULE_NAME, Module_methods);
  lua_pop(L, 1);
  assert(top == lua_gettop(L));
}

dmExtension::Result AppInitializeDAABBCC(dmExtension::AppParams* params)
{
  return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeDAABBCC(dmExtension::Params* params)
{
  LuaInit(params->m_L);
  printf("Registered %s Extension\n", MODULE_NAME);
  return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeDAABBCC(dmExtension::AppParams* params)
{
  return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeDAABBCC(dmExtension::Params* params)
{
  return dmExtension::RESULT_OK;
}


// Defold SDK uses a macro for setting up extension entry points:
//
// DM_DECLARE_EXTENSION(symbol, name, app_init, app_final, init, update, on_event, final)

DM_DECLARE_EXTENSION(DAABBCC, LIB_NAME, AppInitializeDAABBCC, AppFinalizeDAABBCC, InitializeDAABBCC, 0, 0, FinalizeDAABBCC)
