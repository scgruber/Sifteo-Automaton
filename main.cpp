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
unsigned gInitialCells = 64;
SystemTime gLastTime;

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
    unsigned counter = 0;
    for (unsigned i=0; i<256; i++) {
      if (curMap[i]) counter++;
      oldMap[i] = curMap[i];
      curMap[i] = false;
    }
    LOG("Switched maps\n");
    LOG("  detected %d cells\n", counter);
  }

  void update() {
    for (unsigned y=0; y<16; y++) {
      for (unsigned x=0; x<16; x++) {
        if (!terMap[y*16+x]) {
          if (oldMap[y*16+x]) {
            if (countNeighbors(x,y) == 2 || countNeighbors(x,y) == 3)
              curMap[y*16+x] = true;
          } else {
            if (countNeighbors(x,y) == 3) curMap[y*16+x] = true;
          }
        }
      }
    }
    LOG("Updated map\n");
  }

  void draw() {
    for (unsigned i=0; i<256; i++) {
      drawCoords.y = i/16;
      drawCoords.x = i%16;
      if (curMap[i]) {
        vid.bg0.image(drawCoords, Cell);
      } else {
        vid.bg0.image(drawCoords, Ground);
      }
    }
    LOG("Drew map\n\n");
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
  gLastTime = SystemTime::now();
  
  static Automaton cubes[gCubeCt];
  for (unsigned i=0; i<gCubeCt; i++) {
    cubes[i].init(i,gInitialCells);
  }

  // Main loop
  while (1) {
    SystemTime rightNow = SystemTime::now();
    if ((rightNow - gLastTime) > 0.5) {
      gLastTime = SystemTime::now();

      for (unsigned i=0; i<gCubeCt; i++) {
        LOG("Eval for cube #%d:\n", i);
        cubes[i].switchMaps();
        cubes[i].update();
        cubes[i].draw();
      }
    }

    System::paint();
  }
}
