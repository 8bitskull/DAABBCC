# DAABBCC
AABBs tree native extension for Defold Engine

## About
DAABBCC is a C++ wrapper of [AABB.cc](https://github.com/lohedges/aabbcc) lib for Defold Engine. [AABB.cc](https://github.com/lohedges/aabbcc) is developed by [Lester Hedges](http://lesterhedges.net) and released under the [Zlib](http://zlib.net/zlib_license.html) license. The code in [AABB.cc](https://github.com/lohedges/aabbcc) library was adapted from parts of the [Box2D](http://www.box2d.org) physics engine. DAABBCC is a native wrapper developed by [Selim Anaç](https://twitter.com/selimanac)

DAABBCC is a C++ implementation of a dynamic bounding volume hierarchy ([BVH](https://en.wikipedia.org/wiki/Bounding_volume_hierarchy)) using axis-aligned bounding boxes ([AABBs](https://en.wikipedia.org/wiki/Minimum_bounding_box)).
The data structure provides an efficient way of detecting potential overlap between objects of arbitrary shape and size and is commonly used in computer games engines for collision detection and ray tracing.

## Installation
You can use DAABBCC in your own project by adding this project as a [Defold library dependency](http://www.defold.com/manuals/libraries/). Open your game.project file and in the dependencies field under project add:

	https://github.com/selimanac/DAABBCC/archive/master.zip
  
## Usage in Defold Engine

You can use this native extension directly.

### Creating a new AABB Tree

You should create a new tree(s)

#### - createTree
```lua
local tree_name = "particles" -- Name of your tree
local dimension = 2 -- 2D. Original library works with 2D and 3D. Only 2D is implemented   
local skin_thickness = 0.1 -- Thickness of bounding boxes.
local n_particles = 4 -- Number of bounding boxes

daabbcc.createTree(tree_name, dimension, skin_thickness, n_particles)
```

You can create as much as trees according to your needs.
Pseudo example for a platformer:

```lua
daabbcc.createTree("walls", 2, 0.1, 4)
daabbcc.createTree("enemies", 2, 0.1, 50)
daabbcc.createTree("collectables", 2, 0.1, 150)
```

### Inserting AABBs

Two ways of inserting AABBs into the tree. 

#### - insertCircle

Insert a AABB into the name given tree. Works with radius. This is **not** going to create a circular shape. It creates a square from given radius.

insertCircle, returns the ID of the added AABB. You should keep track of those IDs in a table.

**Caution**: IDs starts with '0'

```lua
local tree_name = "particles" -- Name of your tree
local radius = 10 -- radius of the circle. It is basically:  width/2 
local position = vmath.vector3(x,y,z) -- Position of your game object / go.get_position()

local _id = daabbcc.insertCircle(tree_name, radius, position.x , position.y)
```

#### - insertRect

Insert a AABB into the name given tree.

insertRect, returns the ID of the added AABB. You should keep track of those IDs in a table.

**Caution**: IDs starts with '0'

```lua
local tree_name = "particles" -- Name of your tree
local position = vmath.vector3(x,y,z) -- Position of your game object / go.get_position()
local size = vmath.vector3(x,y,z) -- Size of your game object or sprite / go.get("#sprite", "size")

local _id = daabbcc.insertRect(tree_name, position.x , position.y, size.x , size.y)
```
### Updating AABBs

If your game objects are not static, you should update their position and size when they move or resize.

#### - updateCircle

```lua
local tree_name = "particles" -- Name of your tree
local id = 0 -- ID of your object
local radius = 10 -- radius of the circle. It is basically:  width/2
local position = vmath.vector3(x,y,z) -- Position of your game object / go.get_position()

daabbcc.updateCircle(tree_name,id, radius, position.x , position.y)
```

#### - updateRect
```lua
local tree_name = "particles" -- Name of your tree
local id = 0 -- ID of your object
local position = vmath.vector3(x,y,z) -- Position of your game object / go.get_position()
local size = vmath.vector3(x,y,z) -- Size of your game object or sprite / go.get("#sprite", "size")

daabbcc.updateRect(tree_name,id, position.x , position.y, size.x, size.y)
```
### Removing AABBs from Tree

#### - removeAABB
```lua
local tree_name = "particles" -- Name of your tree
local id = 0 -- ID of your object

daabbcc.removeAABB(tree_name,id)
```
### Queries
You can query the tree(s) by id or AABB. Queries returns a table of object IDs

### Query with ID

#### - queryID
```lua
local tree_name = "particles" -- Name of your tree
local id = 0 -- ID of your object
local _result = daabbcc.queryID(tree_name, id)
```
### Query with AABB

#### - queryAABB
```lua
local tree_name = "particles" -- Name of your tree
local position = go.get_position() -- Position of your object
local size = go.get("#sprite", "size")
local _result = daabbcc.queryAABB(tree_name,position.x,position.y,size.x,size.y)
```

## Performance and Notes

## Examples

## Building [AABB.cc](https://github.com/lohedges/aabbcc) lib

https://github.com/selimanac/aabbcc/tree/defoldWrapper

```bash
make release 
make build
make PREFIX=MY_INSTALL_DIR install
```

## [AABB.cc](https://github.com/lohedges/aabbcc) Docs

Doxygen Docs: https://codedocs.xyz/selimanac/aabbcc/
