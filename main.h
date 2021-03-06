#include <sifteo.h>
#include "assets.gen.h"

using namespace Sifteo;

#define NUM_CUBES	3

static AssetSlot MainSlot = AssetSlot::allocate()
        .bootstrap(GameAssets);

static Metadata M = Metadata()
    .title("Stars SDK Example")
    .package("com.sifteo.sdk.stars", "1.0")
    .icon(Icon)
    .cubeRange(NUM_CUBES);

static const unsigned enemies [] = {500, 1000};
static const float millisPerFrame = 50.0f;

static unsigned enemyLoc = enemies[0];
static unsigned endPoint = 2000;
static unsigned currentEnemy = 0;
static TiltShakeRecognizer tsr;
static VideoBuffer buffers[NUM_CUBES];

int prev_xt = 0;
//float x = 0;
float newX = 120;
float newY = 150;
int lastEnemyCube = 2; //enemies always come in from the right

//game flow
void main();
void initCube(int _buffer);
void doPan();

//helpers
void wait(TimeDelta t);
bool haveShake();

//drawing & the like
void updateBG(int id, float x, float y);
void drawColumn(int _buffer, int x, int y);


