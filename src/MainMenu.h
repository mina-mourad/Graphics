#pragma once

// The game's menus. Every button uses the Lab 3 point-in-triangle hover
// test and the u_Highlight hover color.
namespace MainMenu
{
    enum class Action { None, Start, Sound, Exit, Back, Continue, Restart, ToMainMenu };

    void Init();

    // Main menu: START GAME / SOUND / EXIT.
    Action Render(float time);

    // Sound settings (bonus): master / music / effects volume and mute.
    Action RenderSound(float time);

    // Pause menu (Esc): CONTINUE / RESTART / MAIN MENU.
    Action RenderPause();

    // Win / lose menu: RESTART / MAIN MENU.
    Action RenderEnd(bool won, float time);

    void Shutdown();
}
