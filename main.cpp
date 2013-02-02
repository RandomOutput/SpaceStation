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

static unsigned enemyLoc = 200;
static unsigned mode = 0;
static unsigned endPoint = 2000;
static TiltShakeRecognizer tsr;

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
            vid.sprites[0].setImage(Character);
            vid.sprites[0].move(vec(-20, 82-64));
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
            vid.sprites[1].setImage(Enemy);
            vid.sprites[1].move(vec((normLoc*128.0f) - 64, 82.0f - 64.0f));

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
float newX;

void startFight()
{
    instances[0].vid.sprites[0].hide();
    instances[0].vid.bg0.image(vec(0,0), Fight);
    instances[0].vid.bg0.setPanning(vec(0,0));
    instances[0].vid.sprites[1].hide();
    instances[2].vid.bg0.image(vec(0,0), Fight);
    instances[2].vid.bg0.setPanning(vec(0,0));

    instances[1].vid.sprites[0].setImage(Character);
    instances[1].vid.sprites[1].setImage(Enemy);
    instances[1].vid.sprites[0].move(vec(-20, 82-64));
    instances[1].vid.sprites[1].move(vec(20, 82-64));

    tsr.attach(1);
}

void endFight()
{
    enemyLoc = endPoint;
    CubeID(1).detachMotionBuffer();

    for (int _x = x-18; _x < x; _x++) {
        instances[0].drawColumn(_x);
        instances[2].drawColumn(_x);
    }

    instances[0].vid.sprites[0].setImage(Character);
    instances[0].vid.sprites[0].move(vec(-20, 82-64));
    instances[1].vid.sprites[0].hide();
    instances[1].vid.sprites[1].hide();
}

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

void doFight()
{
    startFight();
    //Fight Idle
    while(!haveShake())
    {
        System::paint();
    }
    //Fight Hit
    
    endFight();
    //if win doWalk()
}


void doWalk()
{
    Byte2 accel;
    while(newX < endPoint)
    {
        if(newX >= enemyLoc - 20) doFight();

        
        // Scroll based on accelerometer tilt
        accel = instances[0].vid.physicalAccel().xy();

        if (accel.x > 10 || accel.x < -10)
        {
            // Floating point pixels
            newX += accel.x * (10.0f / 128.0f);
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
