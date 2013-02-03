/*
 * Sifteo SDK Example.
 */

#include "assets.gen.h"
#include <sifteo.h>
using namespace Sifteo;

static const unsigned gNumCubes = 3;
Random gRandom;

static AssetSlot MainSlot = AssetSlot::allocate()
    .bootstrap(GameAssets);

static Metadata M = Metadata()
    .title("Stars SDK Example")
    .package("com.sifteo.sdk.stars", "1.0")
    .icon(Icon)
    .cubeRange(gNumCubes);

static unsigned const enemies [] = {500, 1000, 1500, 2000};
static unsigned enemyLoc = enemies[0];
static unsigned mode = 0;
static unsigned endPoint = 2000;
static unsigned currentEnemy = 0;
static TiltShakeRecognizer tsr;
static const float millisPerFrame = 50.0f;

/* mode 0 - walk
 * mode 1 - fight
 * mode 2 - win
*/


class StarDemo {
public:

    static const unsigned numStars = 8;
    
    static const float textSpeed = 0.2f;
    static const float bgScrollSpeed = 10.0f;
    static const float bgTiltSpeed = 1.0f;
    static const float starEmitSpeed = 60.0f;
    static const float starTiltSpeed = 3.0f;

    VideoBuffer vid;

    void init(CubeID cube)
    {
        frame = 0;
        bg.x = 0;
        bg.y = 0;

        vid.initMode(BG0_SPR_BG1);
        vid.attach(cube);

        // Our background is 18x18 to match BG0, and it seamlessly tiles
        //vid.bg0.image(vec(0,0), Background);
        for (int x = -1; x < 17; x++) {
            drawColumn(x);
        }

        if((int)cube == 0)
        {
            vid.sprites[2].setImage(CharacterIdle);
            vid.sprites[2].move(vec(-20, 82-64));
        }

        // Allocate 16x2 tiles on BG1 for text at the bottom of the screen
        //vid.bg1.setMask(BG1Mask::filled(vec(0,14), vec(16,2)));
    }

    void drawColumn(int x)
    {
        // Draw a vertical column of tiles

        int bg0Width = vid.bg0.tileWidth();
        int dstx = umod(x, bg0Width);
        int srcx = umod(x, Background.tileWidth());

        vid.bg0.image(vec(dstx, 0), vec(1, bg0Width), Background, vec(srcx, 0));
    }

    void  updateEnemy()
    {

    }

    
    void updateBG(float x)
    {
        int id = vid.cube();
        //LOG("cubeID: %i\n", id+1);
        x = x + (200 * (int)(vid.cube()+1));

        // Integer pixels
        int xi = x + 0.5f;

        // Integer tiles
        int xt = x / 8;

        while (prev_xt < xt) {
            // Fill in new tiles, just past the right edge of the screen
            drawColumn(prev_xt + 17);
            prev_xt++;
        }

        while (prev_xt > xt) {
            // Fill in new tiles, just past the left edge
            drawColumn(prev_xt - 2);
            prev_xt--;
        }

        // pixel-level scrolling within the current column
        vid.bg0.setPanning(vec(xi, 0));
        
        //HANDLE THE ENEMY MOVEMENT ALONG THE BASE
        if((x-200)<= enemyLoc && enemyLoc < (x-200+128))
        {
            float normLoc = (enemyLoc - (x-200))/128;

            //LOG("%i | normLoc: %f\n", (int)vid.cube(), normLoc);
            //LOG("***POS: %i", (int)round(normLoc*128));
            vid.sprites[1].setImage(EnemyIdle);
            vid.sprites[1].move(vec((normLoc*128.0f) - 64, 82.0f - 64.0f));

            int nextFrame = SystemTime::now().cycleFrame(millisPerFrame, 100);
            vid.sprites[1].setImage(EnemyIdle, nextFrame % 2);
        }
        else 
        {
            vid.sprites[1].hide();
        }
    }

private:   
    struct {
        Float2 pos, velocity;
    } stars[numStars];

    unsigned frame;
    Float2 bg, text, textTarget;
    float fpsTimespan;
    
    int prev_xt = 0;
};

float x = 0;

static StarDemo instances[gNumCubes];
float newX = -200;

void wait(TimeDelta t)
{
    auto deadline = SystemTime::now() + t;
    while(deadline.inFuture()) {
        System::paint();
    }
}

void doRestart()
{
    newX = -200;
    enemyLoc = enemies[0];
    currentEnemy = 0;

    instances[0].vid.sprites[2].setImage(CharacterIdle);
    instances[0].vid.sprites[2].move(vec(-20, 82-64));
}

void doRestartScreen()
{

    instances[0].vid.bg0.image(vec(0,0), Win);
    instances[1].vid.bg0.image(vec(0,0), Win);
    instances[2].vid.bg0.image(vec(0,0), Win);
    instances[0].vid.bg0.setPanning(vec(0,0));
    instances[1].vid.bg0.setPanning(vec(0,0));
    instances[2].vid.bg0.setPanning(vec(0,0));

    while(!CubeID(0).isTouching()&&!CubeID(1).isTouching()&&!CubeID(2).isTouching())
    {
        System::paint();
    }

    doRestart();
}

void doEnd()
{
    TimeStep ts;
    float sX = -30;
    int lastID = 0;
    float charFrame = 0;

    while(sX < (192*3)+30)
    {
        LOG_FLOAT(ts.delta().seconds());
        sX = sX + 2;
        int cubeID = (int)((sX)/192);   
        LOG_FLOAT(sX);
        int sXint = (int)sX;

        if(cubeID != lastID) 
        {
            instances[lastID].vid.sprites[2].hide();
            lastID = cubeID;
        }

        charFrame += 0.2f;
        instances[cubeID].vid.sprites[2].setImage(CharacterWalk, ((int)charFrame)%2);
        instances[cubeID].vid.sprites[2].move(vec((int)(sXint%192)-64, 82-64));

        System::paint();
        wait(0.01f);
    }

    doRestartScreen();
}
    

void startFight()
{
    instances[0].vid.sprites[2].hide();
    instances[0].vid.bg0.image(vec(0,0), Fight);
    instances[0].vid.bg0.setPanning(vec(0,0));
    instances[0].vid.sprites[1].hide();
    instances[2].vid.bg0.image(vec(0,0), Fight);
    instances[2].vid.bg0.setPanning(vec(0,0));

    instances[1].vid.sprites[2].setImage(CharacterIdle);
    instances[1].vid.sprites[1].setImage(EnemyIdle);
    instances[1].vid.sprites[2].move(vec(-30, 82-64));
    instances[1].vid.sprites[1].move(vec(30, 82-64));

    tsr.attach(1);
}

void endFight()
{

    CubeID(1).detachMotionBuffer();

    for (int _x = x-18; _x < x; _x++) {
        instances[0].drawColumn(_x);
        instances[2].drawColumn(_x);
    }

    instances[0].vid.sprites[2].setImage(CharacterIdle);
    instances[0].vid.sprites[2].move(vec(-20, 82-64));
    instances[1].vid.sprites[2].hide();
    instances[1].vid.sprites[1].hide();

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

void flashHit(TimeDelta t)
{
    auto deadline = SystemTime::now() + t;
    while(deadline.inFuture()) {
        int nextFrame = SystemTime::now().cycleFrame(millisPerFrame, 100);
        instances[1].vid.bg0.image(vec(0,0),Hit, nextFrame % 2);
        System::paint();
    }
}

bool haveShake() {
    tsr.update();
    return tsr.shake;
}

void doFight()
{
    startFight();
    //Fight Idle
    while(!haveShake())
    {
        int nextFrame = SystemTime::now().cycleFrame(10.0f, 100);
        instances[1].vid.sprites[1].setImage(EnemyIdle, nextFrame % 2);
        System::paint();
    }
    instances[1].vid.sprites[1].setImage(EnemyHit);
    instances[1].vid.sprites[2].setImage(CharacterPunch);

    instances[0].vid.bg0.image(vec(0,0), Hit);
    instances[2].vid.bg0.image(vec(0,0), Hit);

    wait(2.0f);
    //Fight Hit
    
    endFight();
    //if win doWalk()
}


void doWalk()
{
    Byte2 accel;

    while(newX < endPoint)
    {
        if(newX >= enemyLoc - 100) doFight();

        
        // Scroll based on accelerometer tilt
        accel = instances[0].vid.physicalAccel().xy();

        if (accel.x > 10 || accel.x < -10)
        {
            // Floating point pixels
            newX += accel.x * (10.0f / 128.0f);
            int nextFrame = SystemTime::now().cycleFrame(millisPerFrame, 100);
            
            instances[0].vid.sprites[2].setImage(CharacterWalk, nextFrame % 2);
        }

        for (unsigned i = 0; i < arraysize(instances); i++)
            instances[i].updateBG(newX);
            //instances[i].update(ts.delta());

        System::paint();
    }
}

void main()
{
    AudioTracker::play(Music);

    for (unsigned i = 0; i < arraysize(instances); i++)
        instances[i].init(i);
    
    TimeStep ts;
    unsigned last = mode;

    doWalk();

}
