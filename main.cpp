#include "assets.gen.h"
#include "main.h"
#include <sifteo.h>

using namespace Sifteo;

void main()
{
    //AudioTracker::play(Music);

    for (unsigned i = 0; i < NUM_CUBES; i++) 
        initCube(i);
    
    initCharacter(0);
    doWalk();
    return;
}

void initCube(int id)
{
    buffers[id].initMode(BG0_SPR_BG1);
    buffers[id].attach(id);

    for (int x = 0; x < 17; x++) {
        drawColumn(id, x);
    }
}

void initCharacter(int id)
{
    buffers[id].sprites[2].setImage(CharacterIdle);
    buffers[id].sprites[2].move(vec(-20, 82-64));
}

void doWalk()
{
    Byte2 accel;

    for (unsigned i = 0; i < NUM_CUBES; i++)
        updateBG(i, newX);

    while(newX < enemies[arraysize(enemies)-1])
    {
        if(newX >= enemyLoc - 100) doFight();
        
        // Scroll based on accelerometer tilt
        accel = buffers[0].physicalAccel().xy();

        if (accel.x > 10 || accel.x < -10) 
        {
            // Floating point pixels
            newX += accel.x * (10.0f / 128.0f);
            int nextFrame = SystemTime::now().cycleFrame(millisPerFrame, 100);
            
            buffers[0].sprites[2].setImage(CharacterWalk, nextFrame % 2);

            for (unsigned i = 0; i < NUM_CUBES; i++)
                updateBG(i, newX);
        }

        updateEnemy();

        System::paint();
    }
}

void startFight()
{
    buffers[0].sprites[2].hide();
    buffers[0].bg0.image(vec(0,0), Fight);
    buffers[0].bg0.setPanning(vec(0,0));
    buffers[0].sprites[1].hide();
    buffers[2].bg0.image(vec(0,0), Fight);
    buffers[2].bg0.setPanning(vec(0,0));

    buffers[1].sprites[2].setImage(CharacterIdle);
    buffers[1].sprites[1].setImage(EnemyIdle);
    buffers[1].sprites[2].move(vec(-30, 82-64));
    buffers[1].sprites[1].move(vec(30, 82-64));

    tsr.attach(1);
}

void doFight()
{
    startFight();
    //Fight Idle
    while(!haveShake())
    {
        int nextFrame = SystemTime::now().cycleFrame(10.0f, 100);
        buffers[1].sprites[1].setImage(EnemyIdle, nextFrame % 2);
        System::paint();
    }
    buffers[1].sprites[1].setImage(EnemyHit);
    buffers[1].sprites[2].setImage(CharacterPunch);

    buffers[0].bg0.image(vec(0,0), Hit);
    buffers[2].bg0.image(vec(0,0), Hit);

    wait(2.0f);
    //Fight Hit
    
    endFight();
    //if win doWalk()
}

void endFight()
{
    CubeID(1).detachMotionBuffer();

    for (int _x = x-18; _x < x; _x++) {
        drawColumn(0, _x);
        drawColumn(2, _x);
    }

    buffers[0].sprites[2].setImage(CharacterIdle);
    buffers[0].sprites[2].move(vec(-20, 82-64));
    buffers[1].sprites[2].hide();
    buffers[1].sprites[1].hide();

    currentEnemy++;
    if(currentEnemy < 3)
    {
        enemyLoc = enemies[currentEnemy];
    } 
    else
    {
        //end the game instead of this hack
        //enemyLoc = 10000;
        doEnd();
    }
}

void doEnd()
{
    TimeStep ts;
    float sX = -30;
    int lastID = 0;
    float charFrame = 0;

    while(sX < (192*3)+30)
    {
        sX = sX + 2;
        int cubeID = (int)((sX)/192);   
        int sXint = (int)sX;

        if(cubeID != lastID) 
        {
            buffers[lastID].sprites[2].hide();
            lastID = cubeID;
        }

        charFrame += 0.2f;
        buffers[cubeID].sprites[2].setImage(CharacterWalk, ((int)charFrame)%2);
        buffers[cubeID].sprites[2].move(vec((int)(sXint%192)-64, 82-64));

        System::paint();
        wait(0.01f);
    }

    doRestartScreen();
}

void doRestartScreen()
{

    buffers[0].bg0.image(vec(0,0), Win);
    buffers[1].bg0.image(vec(0,0), Win);
    buffers[2].bg0.image(vec(0,0), Win);
    buffers[0].bg0.setPanning(vec(0,0));
    buffers[1].bg0.setPanning(vec(0,0));
    buffers[2].bg0.setPanning(vec(0,0));

    while(!CubeID(0).isTouching()&&!CubeID(1).isTouching()&&!CubeID(2).isTouching())
    {
        System::paint();
    }

    doRestart();
}

void doRestart()
{
    newX = -200;
    enemyLoc = enemies[0];
    currentEnemy = 0;

    buffers[0].sprites[2].setImage(CharacterIdle);
    buffers[0].sprites[2].move(vec(-20, 82-64));
}

//helpers

void wait(TimeDelta t)
{
    auto deadline = SystemTime::now() + t;
    while(deadline.inFuture()) {
        System::paint();
    }
}

void flashHit(TimeDelta t)
{
    auto deadline = SystemTime::now() + t;
    while(deadline.inFuture()) {
        int nextFrame = SystemTime::now().cycleFrame(millisPerFrame, 100);
        buffers[1].bg0.image(vec(0,0),Hit, nextFrame % 2);
        System::paint();
    }
}

bool haveShake() {
    tsr.update();
    return tsr.shake;
}

//drawing

void updateBG(int id, float x)
{
    x = x + (200 * (id+1));

    // Integer pixels
    int xi = x + 0.5f;

    // Integer tiles
    int xt = x / 8;

    while (prev_xt < xt) {
        // Fill in new tiles, just past the right edge of the screen
        drawColumn(id, prev_xt + 17);
        prev_xt++;
    }

    while (prev_xt > xt) {
        // Fill in new tiles, just past the left edge
        drawColumn(id, prev_xt - 2);
        prev_xt--;
    }

    // pixel-level scrolling within the current column
    buffers[id].bg0.setPanning(vec(xi, 0));
}

void drawColumn(int _buffer, int x)
{
    // Draw a vertical column of tiles

    int bg0Width = buffers[_buffer].bg0.tileWidth();
    int dstx = umod(x, bg0Width);
    int srcx = umod(x, Background.tileWidth());

    buffers[_buffer].bg0.image(vec(dstx, 0), vec(1, bg0Width), Background, vec(srcx, 0));
}

void updateEnemy()
{
    int sXint = (int)enemyLoc - newX;
    int cubeID = sXint/192;   

    //Don't try and draw to a cube that doesn't exist
    if(cubeID > NUM_CUBES-1)
    {
        buffers[lastEnemyCube].sprites[2].hide();
    } 
    else 
    {
        if(cubeID != lastEnemyCube) 
        {

            buffers[lastEnemyCube].sprites[1].hide();
            lastEnemyCube = cubeID;
        }

        int nextFrame = SystemTime::now().cycleFrame(millisPerFrame, 100);
        buffers[cubeID].sprites[1].setImage(EnemyIdle, nextFrame % 2);
        buffers[cubeID].sprites[1].move(vec((sXint%192)-64, 82-64));
    }
}