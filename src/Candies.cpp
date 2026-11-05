// [LAB] Drawing: Rect / Tri / Circle from Shapes (Lab 2 + Lab 3).
//       Animation: positions and angles from sin() and timers (Lab 2 template).
// [NOT LAB] Game rules: plain C++ (vector, loops, if).
// Below every part that uses lab code there is a "LAB CODE" comment with the
// lab lines exactly as they are written in the lab files.
// Parts with no lab code say "no lab code".
// (All drawing uses Rect / Tri / Circle; their lab code is in Shapes.cpp.)
#include "Candies.h"
#include "Shapes.h"
#include "Sound.h"
#include "Hud.h"
#include <GLFW/glfw3.h>
#include <cmath>

const float ATTACK_TIME = 0.35f;
const float JUMP_TIME = 0.7f;
const float DUCK_TIME = 0.7f;
const glm::vec3 BLACK(0.1f), WHITE(1.0f);

void PlaceCandies(CandyType type)
{
    bool placed = false;
    for (int lane = 0; lane < LANES; ++lane)
    {
        if (!laneSelected[lane] || !CardUnlocked(type)) continue;
        candies.push_back({ type, lane, START_X, CANDY_HP[type], 0.3f, 0, 0, 0, 0 });
        sugar -= CANDY_COST[type];
        placed = true;
    }
    PlaySound(placed ? "place" : "locked");
}
// no lab code

// Keeps the chocolate bar on the shelf: not inside the jar, not inside the fridge.
static float ChocolateSpot(float mouseX)
{
    if (mouseX < START_X) return START_X;
    if (mouseX > LANE_RIGHT - 0.1f) return LANE_RIGHT - 0.1f;
    return mouseX;
}
// no lab code

void PlaceChocolate(int lane, float x)
{
    x = ChocolateSpot(x);
    for (Candy& c : candies)   // not on top of another chocolate bar
        if (c.type == CHOCOLATE && c.lane == lane && std::fabs(c.x - x) < 0.18f) { PlaySound("locked"); return; }
    if (!CardUnlocked(CHOCOLATE)) { PlaySound("locked"); placingChocolate = false; return; }
    candies.push_back({ CHOCOLATE, lane, x, CANDY_HP[CHOCOLATE], 0.0f, 0, 0, 0, 0 });
    sugar -= CANDY_COST[CHOCOLATE];
    placingChocolate = false;
    PlaySound("place");
}
// no lab code

// Is a soda standing right in front of this candy?
static Soda* SodaInFront(const Candy& c, float reach)
{
    Soda* best = nullptr;
    for (Soda& s : sodas)
        if (s.lane == c.lane && s.x - c.x >= -0.05f && s.x - c.x <= reach && (!best || s.x < best->x))
            best = &s;
    return best;
}
// no lab code

void UpdateCandies(float dt)
{
    // W / Up = jump, S / Down = duck, for the candies on the selected shelves
    bool up = keyPressed[GLFW_KEY_W] || keyPressed[GLFW_KEY_UP];
    bool down = keyPressed[GLFW_KEY_S] || keyPressed[GLFW_KEY_DOWN];
    for (Candy& c : candies)
    {
        if (c.type == CHOCOLATE || !laneSelected[c.lane]) continue;   // chocolate never jumps or ducks
        if (up && c.jump <= 0 && c.duck <= 0) c.jump = JUMP_TIME;
        if (down && c.jump <= 0 && c.duck <= 0) c.duck = DUCK_TIME;
    }
    if (up) PlaySound("jump");
    if (down) PlaySound("duck");
    // LAB CODE (Lab 3 Notes/main.cpp):
    //   if (key == GLFW_KEY_W && action == GLFW_REPEAT) posY += moveSpeed;
    //   if (key == GLFW_KEY_S && action == GLFW_REPEAT) posY -= moveSpeed;
    // (W = up, S = down like in the lab; the key is saved by keyCallback in main.cpp)

    for (Candy& c : candies)
    {
        c.jump -= dt; c.duck -= dt; c.hurt -= dt; c.attack -= dt;
        if (c.type != CHOCOLATE)   // the chocolate bar only stands and blocks
        {
            // walk right unless a soda blocks the way
            if (!SodaInFront(c, TOUCH))
                c.x += CANDY_SPEED[c.type] * dt;
            // LAB CODE (Lab 4 Code/Lab4Camera.cpp):
            //   float speed = MOVE_SPEED * deltaTime;
            // LAB CODE (Lab 3 Notes/main.cpp):
            //   if (key == GLFW_KEY_D && action == GLFW_REPEAT) posX += moveSpeed;
            // (moving = position + speed; times deltaTime so it is the same speed on every computer)

            // attack all the time, even with no soda around
            c.cooldown -= dt;
            if (c.cooldown <= 0)
            {
                c.attack = ATTACK_TIME;
                if (c.type == LOLLIPOP) {
                    c.cooldown = 0.8f;
                    Soda* s = SodaInFront(c, TOUCH + 0.06f);
                    if (s) { s->hp -= 1; s->hurt = 0.2f; PlaySound("whack"); }
                } else {
                    c.cooldown = 1.4f;
                    shots.push_back({ c.lane, c.x + 0.16f, FloorY(c.lane) + 0.15f, 0.8f, false, 1, 0.0f });
                    PlaySound("pellet");
                }
            }
        }

        // pick up sugar cubes it touches (the chocolate bar too)
        for (size_t i = 0; i < cubes.size(); ++i)
            if (cubes[i].lane == c.lane && std::fabs(cubes[i].x - c.x) < 0.08f) {
                sugar += cubes[i].value;
                sugarCollected += cubes[i].value;
                cubes.erase(cubes.begin() + i);
                PlaySound("sugar");
                break;
            }
    }

    // defeated candies are removed; candies that reach the fridge are done
    for (size_t i = 0; i < candies.size(); )
    {
        if (candies[i].hp <= 0 || candies[i].x > LANE_RIGHT) {
            if (candies[i].hp <= 0) PlaySound("defeated");
            candies.erase(candies.begin() + i);
        }
        else ++i;
    }
    // no lab code (attacks, sugar pick-up and removing candies are game rules)
}

static void Eyes(float x, float y, float s)
{
    Circle(x, y, 0.014f * s, 0.016f * s, BLACK);
    Circle(x + 0.035f * s, y, 0.014f * s, 0.016f * s, BLACK);
}
// no lab code

void DrawCandy(CandyType type, float x, float footY, float s, float attack, float squash)
{
    // P(dy) = height above the feet; ducking squashes everything down
    auto P = [&](float dy) { return footY + dy * s * squash; };
    float h = s * squash;

    if (type == LOLLIPOP)
    {
        glm::vec3 pink(0.95f, 0.35f, 0.6f);
        Rect(x, P(0.09f), 0.022f * s, 0.18f * h, WHITE);                       // stick
        Rect(x - 0.035f * s, P(0.11f), 0.05f * s, 0.012f * s, WHITE, 0.6f);    // arm
        Circle(x, P(0.24f), 0.085f * s, 0.085f * h, pink);                     // candy head
        Circle(x, P(0.24f), 0.06f * s, 0.06f * h, WHITE);                      // swirl
        Circle(x, P(0.24f), 0.04f * s, 0.04f * h, pink);
        Circle(x, P(0.24f), 0.018f * s, 0.018f * h, WHITE);
        Eyes(x + 0.02f * s, P(0.26f), s);
        // candy cane in the hand: it swings down when attacking (rotation, Lab 2)
        float angle = 1.3f - 1.6f * attack;
        float hx = x + 0.05f * s, hy = P(0.11f);
        float cx = hx + std::cos(angle) * 0.07f * s, cy = hy + std::sin(angle) * 0.07f * s;
        // LAB CODE (Lab 3 Notes/main.cpp):
        //   cosf(angle), sinf(angle), 0.0f,
        // (a point on a circle around the hand, the same math as the lab circle)
        Rect(cx, cy, 0.15f * s, 0.024f * s, glm::vec3(0.9f, 0.1f, 0.1f), angle);
        Rect(cx, cy, 0.03f * s, 0.026f * s, WHITE, angle);                      // stripe
        Circle(hx, hy, 0.016f * s, 0.016f * s, WHITE);                          // hand
    }
    else if (type == BONBON)
    {
        glm::vec3 orange(1.0f, 0.6f, 0.1f), yellow(1.0f, 0.9f, 0.3f);
        float recoil = 0.03f * attack;
        Rect(x - 0.04f * s, P(0.05f), 0.02f * s, 0.1f * h, orange);                     // two short legs
        Rect(x + 0.04f * s, P(0.05f), 0.02f * s, 0.1f * h, orange);
        Tri(x - 0.14f * s, P(0.2f), 0.11f * h, 0.09f * s, yellow, -1.5708f);             // wrapper ends
        Tri(x + 0.14f * s, P(0.2f), 0.11f * h, 0.09f * s, yellow, 1.5708f);
        Circle(x, P(0.2f), 0.11f * s, 0.105f * h, orange);                               // candy
        Rect(x - 0.045f * s, P(0.2f), 0.022f * s, 0.2f * h, yellow);                    // stripes
        Rect(x + 0.045f * s, P(0.2f), 0.022f * s, 0.2f * h, yellow);
        Eyes(x - 0.005f * s, P(0.24f), s);
        Rect(x + (0.1f - recoil) * s, P(0.15f), 0.11f * s, 0.035f * s, glm::vec3(0.45f));  // candy launcher held in front
        Circle(x + (0.05f - recoil) * s, P(0.15f), 0.02f * s, 0.02f * s, orange);            // hand
    }
    else // CHOCOLATE
    {
        glm::vec3 brown(0.4f, 0.22f, 0.1f), light(0.55f, 0.32f, 0.15f);
        Rect(x, P(0.15f), 0.16f * s, 0.3f * h, brown);                              // bar
        for (int row = 0; row < 3; ++row)                                          // squares
            for (int col = 0; col < 2; ++col)
                Rect(x + (col - 0.5f) * 0.07f * s, P(0.19f + row * 0.075f), 0.055f * s, 0.06f * h, light);
        Rect(x, P(0.05f), 0.17f * s, 0.1f * h, glm::vec3(0.75f, 0.75f, 0.8f));      // foil wrapper
        Tri(x, P(0.115f), 0.17f * s, 0.04f * h, glm::vec3(0.75f, 0.75f, 0.8f), 3.14159f);
        Eyes(x - 0.01f * s, P(0.26f), s);
        Rect(x + 0.01f * s, P(0.29f), 0.07f * s, 0.012f * s, BLACK, -0.15f);      // brave eyebrows
    }
    // no lab code (the shapes of the candies)
}

void DrawCandies()
{
    for (int lane = LANES - 1; lane >= 0; --lane)
        for (Candy& c : candies)
        {
            if (c.lane != lane) continue;
            float lift = c.jump > 0 ? 0.25f * std::sin(3.14159f * (1 - c.jump / JUMP_TIME)) : 0.0f;
            // LAB CODE (Lab 2 Template/main.cpp):
            //   float offsetX = sin(t) * 0.5f;
            // (the jump goes up and down with sin)
            float squash = c.duck > 0 ? 0.55f : 1.0f;
            float attack = c.attack > 0 ? 1.0f - c.attack / ATTACK_TIME : 0.0f;
            float health = (float)c.hp / CANDY_HP[c.type];   // 1 = full health ... 0 = no health left
            redness = 0.6f * (1.0f - health);                 // the more hurt, the more red
            if (c.hurt > 0) redness += 0.3f;                  // a short red flash just after a hit
            // LAB CODE (Lab 3 Notes/main.cpp):
            //   glUniform1f(locHighlight, triangleHighlight ? 1.0f : 0.0f);
            // (redness goes to u_Red in the shader, the same way the lab sends u_Highlight - see Shapes.cpp)
            DrawCandy(c.type, c.x, FloorY(lane) + lift, 1.0f, attack, squash);
            redness = 0;
            DrawHealth(c.x, FloorY(lane) - 0.05f, c.hp, CANDY_HP[c.type], true);
        }
    // no lab code (ducking, attack timing and the health bars)
}
