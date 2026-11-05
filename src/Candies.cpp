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

// The numbers used below, with a name (all times in seconds).
const float FIRST_ATTACK = 0.3f;        // a new candy attacks 0.3 s after it is placed
const float ATTACK_TIME = 0.35f;        // one attack animation lasts 0.35 s
const float LOLLIPOP_COOLDOWN = 0.8f;   // the lollipop hits every 0.8 s
const float BONBON_COOLDOWN = 1.4f;     // the bonbon shoots every 1.4 s
const float CANE_REACH = 0.06f;         // the cane reaches this much further than touching
const float PELLET_SPEED = 0.8f;        // how fast a candy pellet flies
const float SODA_FLASH = 0.2f;          // a hit soda flashes red for 0.2 s
const float PICKUP_DISTANCE = 0.08f;    // how close a sugar cube must be to pick it up
const float JUMP_TIME = 0.7f;           // a jump lasts 0.7 s
const float JUMP_HEIGHT = 0.25f;        // how high a candy jumps
const float DUCK_TIME = 0.7f;           // a duck lasts 0.7 s
const float DUCK_SQUASH = 0.55f;        // a ducking candy is 55% of its height
const float LAUNCHER_X = 0.16f;         // a pellet leaves the launcher this far in front of the bonbon
const float LAUNCHER_Y = 0.15f;         // ... and this high above its feet
const float FRIDGE_GAP = 0.1f;          // the chocolate bar stays this far from the fridge
const float BEHIND = 0.05f;             // a soda this little behind a candy still counts as touching it
const glm::vec3 BLACK(0.1f);

// Adds a new candy on a shelf and pays for it.
static void Buy(CandyType type, int lane, float x)
{
    candies.push_back({ type, lane, x, CANDY_HP[type], FIRST_ATTACK, 0, 0, 0, 0 });
    sugar -= CANDY_COST[type];
}
// no lab code

void PlaceCandies(CandyType type)
{
    bool placed = false;
    for (int lane = 0; lane < LANES; ++lane)
    {
        if (!laneSelected[lane] || !CardUnlocked(type)) continue;
        Buy(type, lane, START_X);
        placed = true;
    }
    PlaySound(placed ? "place" : "locked");
}
// no lab code

void PlaceChocolate(int lane, float x)
{
    placingChocolate = false;                                     // this click is used up either way
    if (!CardUnlocked(CHOCOLATE)) { PlaySound("locked"); return; }
    if (x < START_X) x = START_X;                                 // not inside the jar
    if (x > LANE_RIGHT - FRIDGE_GAP) x = LANE_RIGHT - FRIDGE_GAP; // not inside the fridge
    Buy(CHOCOLATE, lane, x);
    PlaySound("place");
}
// no lab code

// Is a soda standing right in front of this candy?
static Soda* SodaInFront(const Candy& c, float reach)
{
    for (Soda& s : sodas)
        if (s.lane == c.lane && s.x - c.x >= -BEHIND && s.x - c.x <= reach)
            return &s;      // the first soda in reach
    return nullptr;         // no soda in front
}
// no lab code

// W / Up = jump, S / Down = duck, for the candies on the selected shelves.
static void JumpAndDuck()
{
    bool up = keyPressed[GLFW_KEY_W] || keyPressed[GLFW_KEY_UP];
    bool down = keyPressed[GLFW_KEY_S] || keyPressed[GLFW_KEY_DOWN];
    for (Candy& c : candies)
    {
        if (c.type == CHOCOLATE || !laneSelected[c.lane]) continue;   // chocolate never jumps or ducks
        if (c.jump > 0 || c.duck > 0) continue;                       // already jumping or ducking
        if (up) c.jump = JUMP_TIME;
        else if (down) c.duck = DUCK_TIME;
    }
    if (up) PlaySound("jump");
    if (down) PlaySound("duck");
    // LAB CODE (Lab 3 Notes/main.cpp):
    //   if (key == GLFW_KEY_W && action == GLFW_REPEAT) posY += moveSpeed;
    //   if (key == GLFW_KEY_S && action == GLFW_REPEAT) posY -= moveSpeed;
    // (W = up, S = down like in the lab; the key is saved by keyCallback in main.cpp)
}

// Walks right unless a soda blocks the way, and attacks all the time
// (even with no soda around): the lollipop hits, the bonbon shoots a pellet.
static void WalkAndAttack(Candy& c, float dt)
{
    if (!SodaInFront(c, TOUCH))
        c.x += CANDY_SPEED[c.type] * dt;
    // LAB CODE (Lab 4 Code/Lab4Camera.cpp):
    //   float speed = MOVE_SPEED * deltaTime;
    // LAB CODE (Lab 3 Notes/main.cpp):
    //   if (key == GLFW_KEY_D && action == GLFW_REPEAT) posX += moveSpeed;
    // (moving = position + speed; times deltaTime so it is the same speed on every computer)

    c.cooldown -= dt;
    if (c.cooldown > 0) return;             // not time to attack yet
    c.attack = ATTACK_TIME;                 // start the attack animation
    if (c.type == LOLLIPOP) {
        c.cooldown = LOLLIPOP_COOLDOWN;
        Soda* s = SodaInFront(c, TOUCH + CANE_REACH);
        if (s) { s->hp -= 1; s->hurt = SODA_FLASH; PlaySound("whack"); }
    } else {
        c.cooldown = BONBON_COOLDOWN;
        shots.push_back({ c.lane, c.x + LAUNCHER_X, FloorY(c.lane) + LAUNCHER_Y, PELLET_SPEED, false, 1, 0.0f });
        PlaySound("pellet");
    }
    // no lab code (the attack rules)
}

// Picks up a sugar cube the candy touches (the chocolate bar too).
static void PickUpSugar(Candy& c)
{
    for (size_t i = 0; i < cubes.size(); ++i)
        if (cubes[i].lane == c.lane && std::fabs(cubes[i].x - c.x) < PICKUP_DISTANCE) {
            sugar += cubes[i].value;
            sugarCollected += cubes[i].value;
            cubes.erase(cubes.begin() + i);
            PlaySound("sugar");
            return;
        }
}
// no lab code

// Defeated candies are removed; candies that reach the fridge are done too.
static void RemoveDefeated()
{
    for (size_t i = 0; i < candies.size(); )
    {
        if (candies[i].hp <= 0 || candies[i].x > LANE_RIGHT) {
            if (candies[i].hp <= 0) PlaySound("defeated");
            candies.erase(candies.begin() + i);
        }
        else ++i;
    }
}
// no lab code

void UpdateCandies(float dt)
{
    JumpAndDuck();
    for (Candy& c : candies)
    {
        c.jump -= dt; c.duck -= dt; c.attack -= dt;     // the timers count down
        if (c.type != CHOCOLATE) WalkAndAttack(c, dt);  // the chocolate bar only stands and blocks
        PickUpSugar(c);
    }
    RemoveDefeated();
}
// no lab code (it only calls the four functions above)

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
        // the candy cane jabs forward and comes back when attacking
        float poke = 0.05f * std::sin(3.14159f * attack);
        // LAB CODE (Lab 2 Template/main.cpp):
        //   float offsetX = sin(t) * 0.5f;
        //   //transform = glm::translate(transform, glm::vec3(offsetX, 0.0f, 0.0f));
        // (like offsetX in Lab 2: the cane moves along x with sin; its x goes through glm::translate in Prepare())
        Rect(0.07f + poke, 0.13f, 0.15f, 0.024f, pink);           // candy cane
    }
    else if (type == BONBON)   // long range: an orange bonbon in a yellow wrapper, with a launcher
    {
        glm::vec3 orange(1.0f, 0.6f, 0.1f), yellow(1.0f, 0.9f, 0.3f);
        Rect(-0.04f, 0.05f, 0.02f, 0.1f, orange);                 // two short legs
        Rect(0.04f, 0.05f, 0.02f, 0.1f, orange);
        Tri(-0.14f, 0.2f, 0.11f, 0.09f, yellow, -1.5708f);        // wrapper ends
        Tri(0.14f, 0.2f, 0.11f, 0.09f, yellow, 1.5708f);
        Circle(0.0f, 0.2f, 0.11f, 0.105f, orange);                // candy
        Eyes(-0.005f, 0.24f);
        Rect(0.1f, 0.15f, 0.11f, 0.035f, yellow);                 // candy launcher
    }
    else                       // defense: a brown chocolate bar of 4 pieces
    {
        glm::vec3 brown(0.4f, 0.22f, 0.1f);
        Rect(-0.045f, 0.075f, 0.07f, 0.14f, brown);               // bottom left piece
        Rect(0.045f, 0.075f, 0.07f, 0.14f, brown);                // bottom right piece
        Rect(-0.045f, 0.235f, 0.07f, 0.14f, brown);               // top left piece
        Rect(0.045f, 0.235f, 0.07f, 0.14f, brown);                // top right piece
        Eyes(-0.0175f, 0.25f);
    }
    model = glm::mat4(1.0f);   // back to "no change" for whatever is drawn next
    // no lab code (the shapes of the candies)
}

void DrawCandies()
{
    for (Candy& c : candies)
    {
        float lift = c.jump > 0 ? JUMP_HEIGHT * std::sin(3.14159f * (1 - c.jump / JUMP_TIME)) : 0.0f;
        // LAB CODE (Lab 2 Template/main.cpp):
        //   float offsetX = sin(t) * 0.5f;
        // (the jump goes up and down with sin)
        float squash = c.duck > 0 ? DUCK_SQUASH : 1.0f;
        float attack = c.attack > 0 ? 1.0f - c.attack / ATTACK_TIME : 0.0f;
        float health = (float)c.hp / CANDY_HP[c.type];   // 1 = full health ... 0 = no health left
        redness = 0.6f * (1.0f - health);                 // the more hurt, the more red
        // LAB CODE (Lab 3 Notes/main.cpp):
        //   glUniform1f(locHighlight, triangleHighlight ? 1.0f : 0.0f);
        // (redness goes to u_Red in the shader, the same way the lab sends u_Highlight - see Shapes.cpp)
        DrawCandy(c.type, c.x, FloorY(c.lane) + lift, 1.0f, attack, squash);
        redness = 0;
        DrawHealth(c.x, FloorY(c.lane) - 0.05f, c.hp, CANDY_HP[c.type], true);
    }
    // no lab code (ducking, attack timing and the health bars)
}
