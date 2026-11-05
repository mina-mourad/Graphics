// [LAB] Only moving / rotating / scaling shapes over time (Lab 2: sin and deltaTime).
// [NOT LAB] Text() for the dialogue and subtitles.
// Below every part that uses lab code there is a "LAB CODE" comment with the
// lab lines exactly as they are written in the lab files.
// Parts with no lab code say "no lab code".
// (All drawing uses Rect / Tri / Circle; their lab code is in Shapes.cpp.)
#include "Cutscene.h"
#include "Game.h"
#include "Shapes.h"
#include "Text.h"
#include "Shelves.h"
#include "Candies.h"
#include "Sodas.h"
#include <string>
#include <cmath>

// 0 before `start`, 1 after `end`, smooth in between
static float Progress(float t, float start, float end)
{
    if (t <= start) return 0.0f;
    if (t >= end) return 1.0f;
    return (t - start) / (end - start);
}
// no lab code

static void Bubble(const std::string& text, float x, float y)
{
    float w = text.size() * 0.066f + 0.08f;
    Rect(x, y, w, 0.12f, glm::vec3(1.0f));
    Tri(x, y - 0.08f, 0.06f, 0.05f, glm::vec3(1.0f), 3.14159f);
    Text(text, x, y, 0.011f, glm::vec3(0.1f));
}
// no lab code

static void Subtitle(const std::string& text)
{
    Rect(0.0f, -0.93f, 2 * ASPECT, 0.12f, glm::vec3(0.1f));
    Text(text, 0.0f, -0.93f, 0.01f, glm::vec3(1.0f));
}
// no lab code

void DrawCutscene(float t)
{
    DrawStore(Progress(t, 2.5f, 3.5f));   // the fridge door opens at 2.5 s

    // the candies wait on their shelves next to the jar
    float duck = (t > 10.0f && t < 11.0f) ? 0.55f : 1.0f;                  // the lollipop ducks under the fizz
    float swing = (t > 8.0f && t < 10.0f) ? 0.5f + 0.5f * std::sin(t * 8.0f) : 0.0f;
    // LAB CODE (Lab 2 Template/main.cpp):
    //   float t = currentFrame;
    //   float pulse = 0.75f + 0.25f * sin(t * 2.0f);
    // (the lollipop swings its cane back and forth with sin of the time)
    DrawCandy(CHOCOLATE, -0.9f, FloorY(2), 1.0f, 0.0f, 1.0f);
    DrawCandy(LOLLIPOP, -0.6f, FloorY(1), 1.0f, swing, duck);
    float shoot = (t > 11.0f && t < 11.4f) ? Progress(t, 11.0f, 11.4f) : 0.0f;
    DrawCandy(BONBON, -0.75f, FloorY(0), 1.0f, shoot, 1.0f);

    // the sodas come out of the fridge
    float bottleX = 1.22f - 0.8f * Progress(t, 3.0f, 5.5f);
    float canX = 1.22f - 0.6f * Progress(t, 3.6f, 6.0f);
    float bottleAttack = (t > 9.6f && t < 10.0f) ? Progress(t, 9.6f, 10.0f) : 0.0f;
    if (t > 2.8f) DrawSoda(BOTTLE, bottleX, FloorY(1), 1.0f, bottleAttack);
    if (t > 3.4f) {
        redness = (t > 11.6f && t < 12.0f) ? 0.7f : 0.0f;   // the can gets hit
        // LAB CODE (Lab 3 Notes/main.cpp):
        //   glUniform1f(locHighlight, triangleHighlight ? 1.0f : 0.0f);
        // (redness goes to u_Red in the shader, the same way the lab sends u_Highlight - see Shapes.cpp)
        DrawSoda(CAN, canX, FloorY(0), 1.0f, 0.0f);
        redness = 0.0f;
    }

    // fizz flies over the ducking lollipop, a candy pellet hits the can
    if (t > 10.0f && t < 11.2f) Circle(bottleX - 0.1f - 0.9f * Progress(t, 10.0f, 11.2f), FloorY(1) + 0.27f, 0.025f, 0.025f, glm::vec3(0.7f, 0.9f, 1.0f));
    if (t > 11.0f && t < 11.6f) Circle(-0.65f + (canX - 0.7f) * Progress(t, 11.0f, 11.6f), FloorY(0) + 0.08f, 0.022f, 0.022f, glm::vec3(1.0f, 0.4f, 0.7f));

    // dialogue
    if (t > 6.0f && t < 8.5f) Bubble("GIVE US YOUR SUGAR!", bottleX, FloorY(1) + 0.45f);
    if (t > 8.5f && t < 10.5f) Bubble("NEVER! THE JAR IS OURS!", -0.4f, FloorY(1) + 0.45f);

    // story text
    if (t < 3.0f) Subtitle("THE CANDY JAR HOLDS THE SUGAR THAT KEEPS EVERY CANDY SWEET...");
    else if (t < 6.0f) Subtitle("BUT THE SODAS IN THE FRIDGE WANT IT TO STAY SWEET!");
    else if (t < 12.0f) Subtitle("DUCK UNDER FIZZ - JUMP OVER SYRUP");
    else {
        Rect(0.0f, 0.86f, 2 * ASPECT, 0.25f, glm::vec3(0.45f, 0.25f, 0.35f));
        Text("DEFEND THE CANDY JAR!", 0.0f, 0.86f, 0.024f, glm::vec3(1.0f, 0.9f, 0.3f));
    }
    Text("SPACE: SKIP", 1.1f, 0.95f, 0.006f, glm::vec3(0.3f));
    // no lab code (moving the characters with Progress, dialogue and subtitles)
}
