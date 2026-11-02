// [LAB] Buttons: pointInTriangle hover test + u_Highlight (Lab 3).
#include "Menus.h"
#include "Game.h"
#include "Shapes.h"
#include "Text.h"
#include "Hud.h"
#include "Sound.h"
#include "Candies.h"
#include "Sodas.h"
#include <string>

// A button: highlighted when the mouse is over it, true when clicked.
static bool Button(const std::string& label, float x, float y, float w = 0.7f)
{
    bool hover = InBox(mouseX, mouseY, x, y, w, 0.14f);
    Rect(x, y, w, 0.14f, glm::vec3(0.55f, 0.3f, 0.45f), 0.0f, hover ? 0.4f : 0.0f);
    Text(label, x, y, 0.012f, glm::vec3(1.0f));
    if (hover && mouseClicked) { PlaySound("click"); return true; }
    return false;
}

static void Panel(float h)
{
    Rect(0.0f, 0.0f, 1.3f, h, glm::vec3(1.0f, 0.93f, 0.8f));
    Rect(0.0f, 0.0f, 1.22f, h - 0.08f, glm::vec3(0.98f, 0.85f, 0.7f));
}

int MainMenu()
{
    Text("SODA VS CANDY", 0.0f, 0.55f, 0.022f, glm::vec3(0.6f, 0.15f, 0.35f));
    Text("DEFEND THE CANDY JAR!", 0.0f, 0.38f, 0.012f, glm::vec3(0.3f, 0.15f, 0.1f));
    DrawCandy(LOLLIPOP, -0.75f, -0.35f, 1.6f, 0.0f, 1.0f);
    DrawSoda(BOTTLE, 0.75f, -0.35f, 1.6f, 0.0f);
    if (Button("START", 0.0f, 0.1f)) return 0;
    if (Button("SOUND", 0.0f, -0.1f)) return 1;
    if (Button("EXIT", 0.0f, -0.3f)) return 2;
    return -1;
}

int SoundMenu()
{
    Panel(1.1f);
    Text("SOUND SETTINGS", 0.0f, 0.4f, 0.016f, glm::vec3(0.6f, 0.15f, 0.35f));
    Text("VOLUME", -0.35f, 0.18f, 0.012f, glm::vec3(0.3f, 0.15f, 0.1f));
    if (Button("-", -0.05f, 0.18f, 0.14f)) ChangeVolume(-0.1f);
    if (Button("+", 0.45f, 0.18f, 0.14f)) ChangeVolume(+0.1f);
    for (int i = 0; i < 10; ++i)   // volume bar
        Rect(0.09f + i * 0.025f, 0.18f, 0.018f, 0.08f, i < (int)(GetVolume() * 10 + 0.5f) ? glm::vec3(0.9f, 0.4f, 0.6f) : glm::vec3(0.7f));
    Text("MUTE", -0.35f, -0.02f, 0.012f, glm::vec3(0.3f, 0.15f, 0.1f));
    if (Button(IsMuted() ? "ON" : "OFF", 0.2f, -0.02f, 0.3f)) ToggleMute();
    if (Button("BACK", 0.0f, -0.3f)) return 0;
    return -1;
}

int PauseMenu()
{
    Panel(0.9f);
    Text("PAUSED", 0.0f, 0.3f, 0.02f, glm::vec3(0.6f, 0.15f, 0.35f));
    if (Button("CONTINUE", 0.0f, 0.1f)) return 0;
    if (Button("RESTART", 0.0f, -0.07f)) return 1;
    if (Button("MAIN MENU", 0.0f, -0.24f)) return 2;
    return -1;
}

int EndMenu(bool won)
{
    Panel(0.8f);
    Text(won ? "YOU WIN!" : "YOU LOSE!", 0.0f, 0.22f, 0.022f, won ? glm::vec3(0.2f, 0.6f, 0.2f) : glm::vec3(0.8f, 0.1f, 0.1f));
    Text(won ? "THE CANDY JAR IS SAFE" : "THE SODAS GOT THE SUGAR", 0.0f, 0.08f, 0.009f, glm::vec3(0.3f, 0.15f, 0.1f));
    if (Button("RESTART", 0.0f, -0.08f)) return 0;
    if (Button("MAIN MENU", 0.0f, -0.25f)) return 1;
    return -1;
}
