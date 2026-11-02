#pragma once
// Person 1: the store (background), the shelves (lanes), the candy jar
// (home base) and choosing shelves with the mouse.
void UpdateShelves();            // hover + click to select / unselect shelves
void DrawStore(float doorOpen);  // wall, floor, fridge (doorOpen 0..1), shelves, candy jar
