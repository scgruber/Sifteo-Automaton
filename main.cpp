/*
 * Cellular automaton for Sifteo
 * Sam Gruber <grubermensch@gmail.com>
 * IACD S2013
 */

#include <sifteo.h>
#include "assets.gen.h"
using namespace Sifteo;

static const unsigned gCubeCt = 3;
Random gRandom;

static AssetSlot MainSlot = AssetSlot::allocate()
  .bootstrap(GameAssets);

static Metadata M = Metadata()
  .title("Cellular Automaton")
  .package("com.scgruber.sifteo.automaton", "0.1")
  .icon(Icon)
  .cubeRange(gCubeCt);

class Automaton {
public:
  void init(CubeID cube, unsigned startct) {
    vid.initMode(BG0_BG1);
    vid.attach(cube);
    
    for (unsigned i=0; i<256; i++) {
      terMap[i] = false;
      curMap[i] = false;
      oldMap[i] = false;
    }

    while (startct != 0) {
      unsigned rX = gRandom.randint(0, 15);
      unsigned rY = gRandom.randint(0, 15);

      if (!curMap[rY*16+rX]) {
        curMap[rY*16+rX] = true;
        startct--;
      }
    }
  }

  void switchMaps() {
    for (unsigned i=0; i<256; i++) {
      oldMap[i] = curMap[i];
      curMap[i] = false;
    }
  }

  void update() {
    for (unsigned y=0; y<16; y++) {
      for (unsigned x=0; x<16; x++) {
        if (!terMap[y*16+x]) {
          if (countNeighbors(x,y) >= 3) curMap[y*16+x] = true;
        }
      }
    }
  }

  void draw() {
    vid.bg0.erase(Ground);
    for (unsigned i=0; i<256; i++) {
      if (curMap[i]) {
        drawCoords.y = i/16;
        drawCoords.x = i%16;
        vid.bg0.image(drawCoords, Cell);
        LOG("Drew cell at (%d,%d)",drawCoords.x,drawCoords.y);
      }
    }
  }
private:
  VideoBuffer vid;
  UInt2 drawCoords;

  bool terMap[256];
  bool curMap[256];
  bool oldMap[256];

  unsigned countNeighbors(unsigned x, unsigned y) {
    unsigned ct = 0;

    // Up
    if ((y != 0) && oldMap[(y-1)*16+x]) ct++;

    // Down
    if ((y != 15) && oldMap[(y+1)*16+x]) ct++;

    // Left
    if ((x != 0) && oldMap[y*16+(x-1)]) ct++;

    // Right
    if ((x != 15) && oldMap[y*16+(x+1)]) ct++;

    return ct;
  }
};

void main()
{
  static Automaton cubes[gCubeCt];
  for (unsigned i=0; i<gCubeCt; i++) {
    cubes[i].init(i,25);
  }

  // Main loop
  while (1) {
    for (unsigned i=0; i<gCubeCt; i++) {
//      cubes[i].switchMaps();
//      cubes[i].update();
      cubes[i].draw();
      LOG("Drew Cube #%d\n\n",i);
    }

    System::paint();
  }
}
