// The game states (menus, cutscene, playing...) and the order things are
// updated and drawn in. [NOT LAB] plain C++ (enum + switch).
#include "Game.h"
#include "Shapes.h"
#include "Shelves.h"
#include "Candies.h"
#include "Sodas.h"
#include "Hud.h"
#include "Menus.h"
#include "Cutscene.h"
#include "Sound.h"
#include <GLFW/glfw3.h>

// ---- the shared lists and numbers (see Game.h) ----
std::vector<Candy> candies;
std::vector<Soda>  sodas;
std::vector<Shot>  shots;
std::vector<Cube>  cubes;
int  sugar = START_SUGAR;
int  sugarCollected = 0;
bool laneSelected[LANES] = {};
int  hoverLane = -1;
bool placingChocolate = false;
int  wave = 1;
float gameTime = 0.0f;

float mouseX = 0.0f, mouseY = 0.0f;
bool  mouseClicked = false;
bool  keyPressed[512] = {};

enum State { MAIN_MENU, SOUND_MENU, CUTSCENE, PLAYING, PAUSED, WON, LOST };
static State state = MAIN_MENU;
static float cutsceneTime = 0.0f;
static bool exitGame = false;

void ResetGame()
{
    candies.clear(); sodas.clear(); shots.clear(); cubes.clear();
    sugar = START_SUGAR;
    sugarCollected = 0;
    for (int i = 0; i < LANES; ++i) laneSelected[i] = false;
    placingChocolate = false;
    ResetWaves();
}

bool GameWantsExit() { return exitGame; }

void GameUpdate(float dt)
{
    if (dt > 0.05f) dt = 0.05f;    // a slow frame must not make things jump
    if (state == PLAYING) gameTime += dt;   // after winning / losing nothing moves any more

    if (state == CUTSCENE)
    {
        cutsceneTime += dt;
        if (cutsceneTime > CUTSCENE_LENGTH || keyPressed[GLFW_KEY_SPACE] || keyPressed[GLFW_KEY_ENTER]) {
            ResetGame();
            state = PLAYING;
        }
    }
    else if (state == PLAYING)
    {
        if (keyPressed[GLFW_KEY_ESCAPE]) { placingChocolate = false; state = PAUSED; return; }
        UpdateHud();          // cards (clicks above the shelves)
        UpdateShelves();      // hover and select shelves
        UpdateCandies(dt);
        bool lost = UpdateSodas(dt);
        UpdateShots(dt);
        bool won = UpdateWaves(dt);
        if (lost) { state = LOST; PlaySound("lose"); }   // everything stops here
        else if (won) { state = WON; PlaySound("win"); }
    }
    else if (state == PAUSED && keyPressed[GLFW_KEY_ESCAPE])
        state = PLAYING;
}

static void DrawWorld()
{
    DrawStore(0.0f);
    DrawCubes();
    DrawSodas();
    DrawCandies();
    DrawShots();
    DrawHud();
}

// The menus are drawn and clicked in the same function, so the state
// changes for menu buttons happen here.
void GameDraw()
{
    int button;
    switch (state)
    {
    case MAIN_MENU:
        DrawStore(0.0f);
        button = MainMenu();
        if (button == 0) { state = CUTSCENE; cutsceneTime = 0.0f; }
        if (button == 1) state = SOUND_MENU;
        if (button == 2) exitGame = true;
        break;
    case SOUND_MENU:
        DrawStore(0.0f);
        if (SoundMenu() == 0) state = MAIN_MENU;
        break;
    case CUTSCENE:
        DrawCutscene(cutsceneTime);
        break;
    case PLAYING:
        DrawWorld();
        break;
    case PAUSED:
        DrawWorld();
        button = PauseMenu();
        if (button == 0) state = PLAYING;
        if (button == 1) { ResetGame(); state = PLAYING; }
        if (button == 2) { ResetGame(); state = MAIN_MENU; }
        break;
    case WON:
    case LOST:
        DrawWorld();                       // the scene stays frozen behind the menu
        button = EndMenu(state == WON);
        if (button == 0) { ResetGame(); state = PLAYING; }
        if (button == 1) { ResetGame(); state = MAIN_MENU; }
        break;
    }
}
