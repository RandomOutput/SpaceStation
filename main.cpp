#include "assets.gen.h"
#include "main.h"
#include <sifteo.h>

using namespace Sifteo;

void main()
{
    //AudioTracker::play(Music);

    for (unsigned i = 0; i < NUM_CUBES; i++) 
        initCube(i);

    doPan();
    
    return;
}

void initCube(int id)
{
    buffers[id].initMode(BG0_SPR_BG1);
    buffers[id].attach(id);
    for (int x = -1; x < 17; x++) {
        drawColumn(id, x, 16);
    }
}

void doPan()
{
    while(true)
    {
        // Scroll based on accelerometer tilt
        Byte2 accel = buffers[0].physicalAccel().xy();

        if (accel.x > 10 || accel.x < -10) 
        {
            // Floating point pixels
            newX += accel.x * (10.0f / 128.0f); 
        }

        if (accel.y > 10 || accel.y < -10) 
        {
            // Floating point pixels
            newY += accel.y * (1.0f / 128.0f); 
        }
        
        for (unsigned i = 0; i < NUM_CUBES; i++)
            updateBG(i, newX, newY);
        
        System::paint();
    }
}

//helpers

void wait(TimeDelta t)
{
    auto deadline = SystemTime::now() + t;
    while(deadline.inFuture()) {
        System::paint();
    }
}

bool haveShake() {
    tsr.update();
    return tsr.shake;
}

//drawing

void updateBG(int id, float x, float y)
{
    // Integer pixels
    int xi = x + 0.5f;
    int yi = y + 0.5f;

    // Integer tiles
    int xt = x / 8;
    int yt = y / 8;

    for(int scan=xt-1;scan<xt+17;scan++)
    {
        drawColumn(id, scan, y);
    }
    /*
    while (prev_xt < xt) {
        // Fill in new tiles, just past the right edge of the screen
        drawColumn(id, prev_xt + 17, y);
        prev_xt++;
    }

    while (prev_xt > xt) {
        // Fill in new tiles, just past the left edge
        drawColumn(id, prev_xt - 2, y);
        prev_xt--;
    }
    */
    // pixel-level scrolling within the current column
    buffers[id].bg0.setPanning(vec(xi, 0));
}

void drawColumn(int _buffer, int x, int y)
{
    // Draw a vertical column of tiles

    int bg0Width = buffers[_buffer].bg0.tileWidth();
    int bg0Height = buffers[_buffer].bg0.tileHeight();
    int dstx = umod(x, bg0Width);
    int srcx = umod(x, StationWalls.tileWidth());
    //int dsty = umod(y, bg0Height);
    int srcy = umod(y, StationWalls.tileHeight());
    LOG_INT(bg0Width);

    //buffers[_buffer].bg0.image(vec(dstx, 0), vec(1, bg0Width), StationWalls, vec(srcx, srcy));

    for(int scan=0;scan<bg0Height;scan++)
    {
        int srcy = umod(y+scan, StationWalls.tileHeight());
        buffers[_buffer].bg0.image(vec(dstx, 0+scan), vec(1, 1), StationWalls, vec(srcx, srcy));
    }

    LOG_INT(dstx);
    //LOG_INT(dsty);
    LOG_INT(srcx);
    LOG_INT(srcy);

}
