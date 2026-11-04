#pragma once
// Person 2 (candies, menus and sound).
// Main menu, sound settings, pause menu, win / lose menu.
// Each returns which button was clicked (0, 1, 2 ...) or -1 for none.
// no lab code in this file (only declarations; the code is in the .cpp file)
int MainMenu();      // 0 start, 1 sound, 2 exit
int SoundMenu();     // 0 back (the - / + / mute buttons work by themselves)
int PauseMenu();     // 0 continue, 1 restart, 2 main menu
int EndMenu(bool won);   // 0 restart, 1 main menu
