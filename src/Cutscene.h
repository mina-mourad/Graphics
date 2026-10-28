#pragma once

// Animated introductory cutscene (about 16 seconds): the Medjay guard, an
// archer ally and the cursed army of Set meet in front of the Sun Temple, and
// a short dialogue sets up what the player is defending and why.
namespace Cutscene
{
    constexpr float DURATION = 16.0f;

    void  Start();
    void  Update(float dt);
    void  Render();
    bool  IsFinished();
    float Time();
}
