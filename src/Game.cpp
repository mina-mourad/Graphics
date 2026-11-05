// Person 4 (store, game flow and intro).
// The game states (menus, cutscene, playing...) and the order things are
// updated and drawn in. [NOT LAB] plain C++ (enum + switch).
// [LAB] the mouse / key variables and Esc (Lab 3).
// Below every part that uses lab code there is a "LAB CODE" comment with the
// lab lines exactly as they are written in the lab files.
// Parts with no lab code say "no lab code".
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
int  sugar = StartSugar;
int  sugarCollected = 0;
bool laneSelected[Lanes] = {};
int  hoverLane = -1;
bool placingChocolate = false;
int  wave = 1;
float gameTime = 0.0f;

float mouseX = 0.0f, mouseY = 0.0f;
bool  mouseClicked = false;
bool  keyPressed[512] = {};
// LAB CODE (Lab 3 Notes/Lab3ExtraNotes.cpp):
//   float mouseX = 0;
//   float mouseY = 0;
//   bool clicked = false;

enum State { MainMenuState, SoundMenuState, Cutscene, Playing, Paused, Won, Lost };
static State state = MainMenuState;
static float cutsceneTime = 0.0f;
static bool exitGame = false;

void ResetGame()
{
    candies.clear(); sodas.clear(); shots.clear(); cubes.clear();
    sugar = StartSugar;
    sugarCollected = 0;
    for (int i = 0; i < Lanes; ++i) laneSelected[i] = false;
    placingChocolate = false;
    ResetWaves();
}
// no lab code

bool GameWantsExit() { return exitGame; }
// no lab code (main.cpp stops its loop when this is true)

void GameUpdate(float dt)
{
    if (dt > 0.05f) dt = 0.05f;    // a slow frame must not make things jump
    if (state == Playing) gameTime += dt;   // after winning / losing nothing moves any more

    if (state == Cutscene)
    {
        cutsceneTime += dt;
        if (cutsceneTime > CutsceneLength || keyPressed[GLFW_KEY_SPACE] || keyPressed[GLFW_KEY_ENTER]) {
            ResetGame();
            state = Playing;
        }
    }
    else if (state == Playing)
    {
        if (keyPressed[GLFW_KEY_ESCAPE]) { placingChocolate = false; state = Paused; return; }
        // LAB CODE (Lab 3 Notes/main.cpp):
        //   if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        // (Esc pauses the game instead of closing the window)
        UpdateHud();          // cards (clicks above the shelves)
        UpdateShelves();      // hover and select shelves
        UpdateCandies(dt);
        bool lost = UpdateSodas(dt);
        UpdateShots(dt);
        bool won = UpdateWaves(dt);
        if (lost) { state = Lost; PlaySound("lose"); }   // everything stops here
        else if (won) { state = Won; PlaySound("win"); }
    }
    else if (state == Paused && keyPressed[GLFW_KEY_ESCAPE])
        state = Playing;
    // no lab code (the game states)
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
// no lab code

// The menus are drawn and clicked in the same function, so the state
// changes for menu buttons happen here.
void GameDraw()
{
    int button;
    switch (state)
    {
    case MainMenuState:
        DrawStore(0.0f);
        button = MainMenu();
        if (button == 0) { state = Cutscene; cutsceneTime = 0.0f; }
        if (button == 1) state = SoundMenuState;
        if (button == 2) exitGame = true;
        break;
    case SoundMenuState:
        DrawStore(0.0f);
        if (SoundMenu() == 0) state = MainMenuState;
        break;
    case Cutscene:
        DrawCutscene(cutsceneTime);
        break;
    case Playing:
        DrawWorld();
        break;
    case Paused:
        DrawWorld();
        button = PauseMenu();
        if (button == 0) state = Playing;
        if (button == 1) { ResetGame(); state = Playing; }
        if (button == 2) { ResetGame(); state = MainMenuState; }
        break;
    case Won:
    case Lost:
        DrawWorld();                       // the scene stays frozen behind the menu
        button = EndMenu(state == Won);
        if (button == 0) { ResetGame(); state = Playing; }
        if (button == 1) { ResetGame(); state = MainMenuState; }
        break;
    }
}
// no lab code
