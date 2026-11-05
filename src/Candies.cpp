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
#include <glm/gtc/matrix_transform.hpp>
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

static void Eyes(float x, float y)
{
    Circle(x, y, 0.014f, 0.016f, BLACK);
    Circle(x + 0.035f, y, 0.014f, 0.016f, BLACK);
}
// no lab code

void DrawCandy(CandyType type, float x, float footY, float s, float attack, float squash)
{
    // Like Lab 2: one transform moves the whole candy to its place and scales it.
    // s = size (1 normal, smaller on the cards), squash < 1 = flattened while ducking.
    // The parts below are drawn around (0, 0) = the middle of the candy's feet,
    // so the squash pushes it down onto the shelf.
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, footY, 0.0f));
    model = glm::scale(model, glm::vec3(s, s * squash, 1.0f));
    // LAB CODE (Lab 2 Template/main.cpp):
    //   glm::mat4 transform = glm::mat4(1.0f);
    //   //transform = glm::translate(transform, glm::vec3(offsetX, 0.0f, 0.0f));
    //   //transform = glm::scale(transform, glm::vec3(pulse, pulse, 1.0f));
    // (model is used by every shape below - see Prepare() in Shapes.cpp)

    if (type == LOLLIPOP)      // melee: a pink lollipop with a candy cane
    {
        glm::vec3 pink(0.95f, 0.35f, 0.6f);
        Rect(0.0f, 0.09f, 0.022f, 0.18f, pink);                   // stick
        Circle(0.0f, 0.24f, 0.085f, 0.085f, pink);                // candy head
        Eyes(-0.005f, 0.25f);
        // the candy cane swings down when attacking
        float angle = 0.9f - 1.2f * attack;
        float hx = 0.015f, hy = 0.11f;                            // where the cane is held
        float cx = hx + std::cos(angle) * 0.07f, cy = hy + std::sin(angle) * 0.07f;
        // LAB CODE (Lab 3 Notes/main.cpp):
        //   cosf(angle), sinf(angle), 0.0f,
        // (a point on a circle around the hand, the same math as the lab circle)
        Rect(cx, cy, 0.15f, 0.024f, pink, angle);                 // candy cane
    }
    else if (type == BONBON)   // long range: an orange bonbon in a yellow wrapper, with a launcher
    {
        glm::vec3 orange(1.0f, 0.6f, 0.1f), yellow(1.0f, 0.9f, 0.3f);
        float recoil = 0.03f * attack;                            // the launcher kicks back when it shoots
        Rect(-0.04f, 0.05f, 0.02f, 0.1f, orange);                 // two short legs
        Rect(0.04f, 0.05f, 0.02f, 0.1f, orange);
        Tri(-0.14f, 0.2f, 0.11f, 0.09f, yellow, -1.5708f);        // wrapper ends
        Tri(0.14f, 0.2f, 0.11f, 0.09f, yellow, 1.5708f);
        Circle(0.0f, 0.2f, 0.11f, 0.105f, orange);                // candy
        Eyes(-0.005f, 0.24f);
        Rect(0.1f - recoil, 0.15f, 0.11f, 0.035f, yellow);        // candy launcher
    }
    else                       // defense: a brown chocolate bar of 4 pieces
    {
        glm::vec3 brown(0.4f, 0.22f, 0.1f);
        for (int row = 0; row < 2; ++row)                         // 2 x 2 pieces with small gaps
            for (int col = 0; col < 2; ++col)
                Rect((col - 0.5f) * 0.09f, 0.075f + row * 0.16f, 0.07f, 0.14f, brown);
        Eyes(-0.0175f, 0.25f);
    }
    model = glm::mat4(1.0f);   // back to "no change" for whatever is drawn next
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
