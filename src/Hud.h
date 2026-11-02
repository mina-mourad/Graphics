#pragma once

#include <glm/glm.hpp>
#include <string>

// In-game interface (Person 4): player cards, coin counter, wave counter,
// pause button, messages, and small effects (sparks, "+10" texts).
namespace Hud
{
    void Init();

    // Card clicks / 1-2-3 keys place players on the selected lanes. Returns
    // true if the mouse click was used by the HUD.
    bool HandleInput();

    // True for one frame when the pause button was clicked.
    bool PauseClicked();

    void Update(float deltaTime);

    void Render();          // cards, coins, wave, messages (on top of the scene)
    void RenderEffects();   // sparks and floating texts (in the scene)

    void ShowToast(const std::string& text);
    void AddSpark(glm::vec2 pos, glm::vec3 color);
    void AddFloatText(glm::vec2 pos, const std::string& text, glm::vec3 color);

    // Shared UI pieces (menus use them too).
    void DrawPanel(glm::vec2 center, glm::vec2 size, float highlight = 0.0f);
    bool InBox(glm::vec2 p, glm::vec2 center, glm::vec2 size);   // Lab 3 point-in-triangle on the two halves
}
