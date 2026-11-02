#pragma once

#include <glm/glm.hpp>

// The setting: a narrow street (hara) in an old Cairo neighbourhood.
// Brick and concrete houses with balconies, laundry and satellite dishes,
// power poles with tangled cables, parked tuk-tuks, rubble, tires and
// crates -- and the koshk (kiosk) the players defend at the left end.
namespace Environment
{
    // Sky, houses, poles, cables and the props at the back of the street.
    void DrawBackground(float time);

    // The dirt road of one lane. `highlight` mixes it toward yellow
    // (hover / selected lanes).
    void DrawLane(int lane, float highlight);

    // The home base: Am Sayed's koshk.
    void DrawHomeBase(float time);

    // The near sidewalk strip at the bottom of the screen.
    void DrawForeground(float time);

    // A tuk-tuk (also used in the cutscene). `feet` = middle of the wheels on
    // the ground, facing = +1 right / -1 left.
    void DrawTukTuk(glm::vec2 feet, float scale, float facing, float time);
}
