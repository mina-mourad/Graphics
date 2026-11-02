#pragma once

// The 16-second animated intro: the street wakes up, the slipper gang
// marches in and demands Am Sayed's koshk, El Bawab refuses, and the first
// slipper, pebble and tomato fly -- showing the duck / jump controls.
namespace Cutscene
{
    constexpr float DURATION = 16.0f;

    void Start();
    void Update(float deltaTime);
    void Render();
    bool IsFinished();
    float GetTime();
}
