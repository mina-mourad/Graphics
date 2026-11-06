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

enum State { MainMenuState, SoundMenuState, Cutscene, Playing, Paused, Won, Lost };
static State state = MainMenuState;
static float cutsceneTime = 0.0f;
static bool exitGame = false;

void ResetGame()
{
    candies.clear(); sodas.clear(); shots.clear(); cubes.clear();
    sugar = StartSugar;
    sugarCollected = 0;
    for (int i = 0; i < Lanes; ++i) 
    {
        laneSelected[i] = false;
    }
    placingChocolate = false;
    ResetWaves();
}

bool GameWantsExit() { return exitGame; }

void GameUpdate(float dt)
{
    if (dt > 0.05f) 
    {
        dt = 0.05f;
    }
    if (state == Playing) 
    {
        gameTime += dt;
    }

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
        if (keyPressed[GLFW_KEY_ESCAPE]) { 
            placingChocolate = false; 
            state = Paused; return; 
        }
        UpdateHud();
        UpdateShelves();
        UpdateCandies(dt);
        bool lost = UpdateSodas(dt);
        UpdateShots(dt);
        bool won = UpdateWaves(dt);
        if (lost) { 
            state = Lost; 
            PlaySound("lose"); 
        }
        else if (won) { 
            state = Won; 
            PlaySound("win"); 
        }
    }
    else if (state == Paused && keyPressed[GLFW_KEY_ESCAPE])
        state = Playing;
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

void GameDraw()
{
    int button;
    switch (state)
    {
    case MainMenuState:
        DrawStore(0.0f);
        button = MainMenu();
        if (button == 0) { 
            state = Cutscene; 
            cutsceneTime = 0.0f; 
        }
        if (button == 1) 
        {
            state = SoundMenuState;
        }
        if (button == 2) 
        {
            exitGame = true;
        }
        break;
    case SoundMenuState:
        DrawStore(0.0f);
        if (SoundMenu() == 0) 
        {
            state = MainMenuState;
        }
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
        if (button == 0) 
        {
            state = Playing;
        }
        if (button == 1) 
        {
            ResetGame();
            state = Playing;
        }
        if (button == 2) 
        {
            ResetGame();
            state = MainMenuState;
        }
        break;
    case Won:
    case Lost:
        DrawWorld();
        button = EndMenu(state == Won);
        if (button == 0) { 
            ResetGame(); 
            state = Playing; 
        }
        if (button == 1) { 
            ResetGame(); 
            state = MainMenuState; 
        }
        break;
    }
}
