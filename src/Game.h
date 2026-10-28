#pragma once

// Game state machine, gameplay, HUD and menus.
//
//   Main menu -> cutscene -> playing <-> paused
//                               |
//                               +--> win / lose menu
namespace Game
{
    enum class State { MainMenu, Cutscene, Playing, Paused, Won, Lost };

    // Call once after Renderer2D::Init and Input::Init.
    // seed = 0 uses a random seed; any other value makes spawns repeatable.
    void Init(unsigned seed = 0);

    void Update(float dt);   // once per frame, after Input::Update()
    void Render();           // once per frame, after Renderer2D::BeginFrame()

    bool WantsExit();        // true after the player chooses "Exit"

    // Read-only snapshot used by automated tests.
    struct Snapshot
    {
        State state;
        int   coins;
        int   wave;            // 1-based wave number being played
        int   players;
        int   enemies;
        int   selectedLanes;
        int   enemiesToSpawn;
    };
    Snapshot GetSnapshot();
#ifdef GAME_TEST_HOOKS
    void DebugAddCoins(int n);   // test harness only
#endif
}
