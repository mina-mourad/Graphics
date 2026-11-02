#pragma once

// Game states and the main loop's update / render, so main.cpp only has to
// create the window and call these every frame.
namespace Game
{
    enum class State { MainMenu, SoundSettings, Cutscene, Playing, Paused, Won, Lost };

    void Init();
    void Update(float deltaTime);
    void Render();
    bool WantsExit();
    void Shutdown();

    State GetState();
}
