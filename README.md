# PurrfectEngine

PurrfectEngine is a game engine written in C++.

## Building

### Assimp

PurrfectEngine uses assimp for loading models. So to build PurrfectEngine you have to build assimp first.
Here's how to do this:
```shell
cd deps/assimp
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF
```