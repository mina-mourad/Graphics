#include "Game.h"
#include "GameConfig.h"
#include "Renderer2D.h"
#include "Input.h"
#include "EnemySystem.h"
#include "WaveManager.h"
#include "PlayerSystem.h"
#include "Economy.h"
#include "Hud.h"
#include "Environment.h"
#include "Cutscene.h"
#include "MainMenu.h"
#include "Audio.h"

#include <algorithm>

namespace
{
    Game::State state = Game::State::MainMenu;
    bool wantsExit = false;

    float time = 0.0f;       // always running (menus, background animation)
    float endTime = 0.0f;    // seconds since the game was won / lost

    // Clicks are read in Update and acted on in Render (the menus are
    // immediate-mode buttons), so the menu result is stored here.
    MainMenu::Action pendingAction = MainMenu::Action::None;

    void ResetWorld()
    {
        EnemySystem::Shutdown();
        WaveManager::Shutdown();
        EnemySystem::Init();
        WaveManager::Init();
        PlayerSystem::Init();
        Economy::Init();
        Hud::Init();
        endTime = 0.0f;
    }

    void StartPlaying()
    {
        ResetWorld();
        state = Game::State::Playing;
    }

    void SetState(Game::State s)
    {
        state = s;
        if (s == Game::State::Won || s == Game::State::Lost)
            endTime = 0.0f;
    }

    // The game world, back to front.
    void DrawWorld()
    {
        Environment::DrawBackground(time);
        for (int lane = NUM_LANES - 1; lane >= 0; --lane)
            Environment::DrawLane(lane, PlayerSystem::GetLaneHighlight(lane));
        Environment::DrawHomeBase(time);

        // characters lane by lane: the top lane is furthest away
        for (int lane = NUM_LANES - 1; lane >= 0; --lane)
        {
            Economy::RenderLane(lane);
            EnemySystem::RenderLane(lane);
            PlayerSystem::RenderLane(lane);
        }
        EnemySystem::RenderProjectiles();
        PlayerSystem::RenderProjectiles();
        Hud::RenderEffects();

        Environment::DrawForeground(time);
        Hud::Render();
    }

    void UpdatePlaying(float dt)
    {
        if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
        {
            SetState(Game::State::Paused);
            return;
        }

        // input: HUD first (cards, pause button), then the lanes
        bool used = Hud::HandleInput();
        if (Hud::PauseClicked())
        {
            SetState(Game::State::Paused);
            return;
        }
        if (!used)
            PlayerSystem::UpdateLanes();
        PlayerSystem::HandleKeys();

        // simulation
        WaveManager::Update(dt);
        PlayerSystem::ShareWithEnemies();
        EnemySystem::Update(dt);
        for (const EnemySystem::Defeated& d : EnemySystem::GetDefeated())
            Economy::DropCoins(d.lane, d.x, ENEMY_COIN[static_cast<int>(d.type)]);
        PlayerSystem::Update(dt);
        Economy::Update(dt);
        Hud::Update(dt);

        // losing: an enemy reached the koshk -> everything stops right here
        if (EnemySystem::ReachedBase())
        {
            SetState(Game::State::Lost);
            Audio::Play(Audio::Sfx::Lose);
        }
        else if (WaveManager::AllWavesCleared())
        {
            SetState(Game::State::Won);
            Audio::Play(Audio::Sfx::Win);
        }
    }
}

namespace Game
{
    State GetState() { return state; }
    bool WantsExit() { return wantsExit; }

    void Init()
    {
        Audio::Init();
        MainMenu::Init();
        ResetWorld();
        state = State::MainMenu;
        wantsExit = false;
    }

    void Update(float dt)
    {
        dt = std::min(dt, 0.05f);   // a long frame must not let things jump through each other
        time += dt;

        // music for each state
        switch (state)
        {
        case State::MainMenu: case State::SoundSettings: case State::Cutscene:
            Audio::SetMusic(Audio::Music::Menu); break;
        case State::Playing: case State::Paused:
            Audio::SetMusic(Audio::Music::Battle); break;
        default:
            Audio::SetMusic(Audio::Music::None); break;
        }
        Audio::SetMusicDucked(state == State::Paused);

        // act on the button clicked during the last Render
        MainMenu::Action a = pendingAction;
        pendingAction = MainMenu::Action::None;

        switch (state)
        {
        case State::MainMenu:
            if (a == MainMenu::Action::Start) { ResetWorld(); state = State::Cutscene; Cutscene::Start(); }
            if (a == MainMenu::Action::Sound) state = State::SoundSettings;
            if (a == MainMenu::Action::Exit) wantsExit = true;
            break;

        case State::SoundSettings:
            if (a == MainMenu::Action::Back) state = State::MainMenu;
            break;

        case State::Cutscene:
            Cutscene::Update(dt);
            if (Cutscene::IsFinished() || Input::IsKeyPressed(GLFW_KEY_SPACE) || Input::IsKeyPressed(GLFW_KEY_ENTER))
                StartPlaying();
            break;

        case State::Playing:
            UpdatePlaying(dt);
            break;

        case State::Paused:
            if (Input::IsKeyPressed(GLFW_KEY_ESCAPE) || a == MainMenu::Action::Continue) state = State::Playing;
            if (a == MainMenu::Action::Restart) StartPlaying();
            if (a == MainMenu::Action::ToMainMenu) { ResetWorld(); state = State::MainMenu; }
            break;

        case State::Won:
        case State::Lost:
            endTime += dt;
            if (a == MainMenu::Action::Restart) StartPlaying();
            if (a == MainMenu::Action::ToMainMenu) { ResetWorld(); state = State::MainMenu; }
            break;
        }
    }

    void Render()
    {
        switch (state)
        {
        case State::MainMenu:
            pendingAction = MainMenu::Render(time);
            break;

        case State::SoundSettings:
            pendingAction = MainMenu::RenderSound(time);
            break;

        case State::Cutscene:
            Cutscene::Render();
            break;

        case State::Playing:
            DrawWorld();
            break;

        case State::Paused:
            Renderer2D::SetFade(0.45f);
            DrawWorld();
            Renderer2D::SetFade(0.0f);
            pendingAction = MainMenu::RenderPause();
            break;

        case State::Won:
        case State::Lost:
            // the scene stays frozen behind the menu
            Renderer2D::SetFade(std::min(0.45f, endTime));
            DrawWorld();
            Renderer2D::SetFade(0.0f);
            if (endTime > 0.8f)
                pendingAction = MainMenu::RenderEnd(state == State::Won, time);
            break;
        }
    }

    void Shutdown()
    {
        EnemySystem::Shutdown();
        WaveManager::Shutdown();
        PlayerSystem::Shutdown();
        MainMenu::Shutdown();
        Audio::Shutdown();
    }
}
