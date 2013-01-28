/*
 * Cellular automaton for Sifteo
 * Sam Gruber <grubermensch@gmail.com>
 * IACD S2013
 */

#include <sifteo.h>
#include "assets.gen.h"
using namespace Sifteo;

#define _DEBUG

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
  enum Dir { None=0, Up=1, Down=2, Left=3, Right=4 };
  
  void init(CubeID cube, unsigned startct) {
    vid.initMode(BG0_BG1);
    vid.attach(cube);

    accelDir = None;
    
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

  void add(unsigned ct) {
    while (ct != 0) {
      unsigned rX = gRandom.randint(0,15);
      unsigned rY = gRandom.randint(0,15);

      if (!curMap[rY*16+rX]) {
        curMap[rY*16+rX] = true;
        ct--;
      }
    }
  }

  void setAccel(Dir d) {
    accelDir = d;
  }

  void tilt() {
    switch (accelDir) {
      case Up:
        for (int y=1; y<16; y++) {
          for (int x=0; x<16; x++) {
            if (curMap[y*16+x]) {
              if (!curMap[(y-1)*16+x]) {
                curMap[(y-1)*16+x] = true;
                curMap[y*16+x] = false;
              }
            }
          }
        }
        break;
      case Down:
        for (int y=0; y<15; y++) {
          for (int x=0; x<16; x++) {
            if (curMap[y*16+x]) {
              if (!curMap[(y+1)*16+x]) {
                curMap[(y+1)*16+x] = true;
                curMap[y*16+x] = false;
              }
            }
          }
        }
        break;
      case Left:
        for (int y=0; y<16; y++) {
          for (int x=1; x<16; x++) {
            if (curMap[y*16+x]) {
              if (!curMap[y*16+(x-1)]) {
                curMap[y*16+(x-1)] = true;
                curMap[y*16+x] = false;
              }
            }
          }
        }
        break;
      case Right:
        for (int y=0; y<16; y++) {
          for (int x=0; x<15; x++) {
            if (curMap[y*16+x]) {
              if (!curMap[y*16+(x+1)]) {
                curMap[y*16+(x+1)] = true;
                curMap[y*16+x] = false;
              }
            }
          }
        }
        break;
      default: break;
    }
    accelDir = None;
  }

  void switchMaps() {
    unsigned counter = 0;
    for (unsigned i=0; i<256; i++) {
      if (curMap[i]) counter++;
      oldMap[i] = curMap[i];
      curMap[i] = false;
    }
#ifdef _DEBUG
    LOG("Switched maps\n");
    LOG("  detected %d cells\n", counter);
#endif
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
#ifdef _DEBUG
    LOG("Updated map\n");
#endif
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
#ifdef _DEBUG
    LOG("Drew map\n\n");
#endif
  }


private:
  VideoBuffer vid;
  UInt2 drawCoords;

  bool terMap[256];
  bool curMap[256];
  bool oldMap[256];

  Dir accelDir;

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

class Listener {
public:
  void init(Automaton* cubes) {
    cubeArray = cubes;
    Events::cubeTouch.set(&Listener::onTouch, this);
    Events::cubeAccelChange.set(&Listener::onAccel, this);
  }

private:
  Automaton* cubeArray;

  void onTouch(unsigned id) {
    cubeArray[id].add(gInitialCells);
#ifdef _DEBUG
    LOG("Recieved touch event\n");
#endif
  }

  void onAccel(unsigned id) {
    CubeID cube(id);
    if (cube.accel().x < -16) {
      cubeArray[id].setAccel(Automaton::Left);
    } else if (cube.accel().x > 16) {
      cubeArray[id].setAccel(Automaton::Right);
    }
    if (cube.accel().y < -16) {
      cubeArray[id].setAccel(Automaton::Up);
    } else if (cube.accel().y > 16) {
      cubeArray[id].setAccel(Automaton::Down);
    }
#ifdef _DEBUG
    LOG("Received accel event\n");
#endif
  }

};

void main()
{
  gLastTime = SystemTime::now();
  
  static Automaton cubes[gCubeCt];
  static Listener listen;
  listen.init(cubes);
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
        cubes[i].tilt();
        cubes[i].draw();
      }
    }

    System::paint();
  }
}
