// [LAB] Drawing: Rect / Tri / Circle from Shapes; sin() wobble (Lab 2).
// [NOT LAB] rand() for random shelves, game rules in plain C++.
#include "Sodas.h"
#include "Shapes.h"
#include "Sound.h"
#include "Hud.h"
#include <cstdlib>
#include <cmath>

const int WAVE_SIZE[3] = { 3, 5, 10 };
static int spawned = 0;
static float spawnTimer = 2.0f;
static bool betweenWaves = false;   // the last wave is beaten, the next one has not started yet
const float ATTACK_TIME = 0.35f;
const glm::vec3 BLACK(0.1f), WHITE(1.0f);

void ResetWaves() { spawned = 0; spawnTimer = 2.0f; wave = 1; betweenWaves = false; }

bool UpdateWaves(float dt)
{
    if (betweenWaves)                           // short break, then the next wave starts
    {                                           // together with its first soda
        spawnTimer -= dt;
        if (spawnTimer > 0) return false;
        betweenWaves = false; wave++; spawned = 0;   // the new wave starts now, with its first soda below
    }
    if (spawned < WAVE_SIZE[wave - 1])
    {
        spawnTimer -= dt;
        if (spawnTimer <= 0)
        {
            // wave 1: bottles only, wave 2: bottles and cans, wave 3: all three
            SodaType type = BOTTLE;
            if (wave == 2) type = (spawned % 2 == 0) ? BOTTLE : CAN;
            if (wave == 3) type = (SodaType)(spawned % 3);
            sodas.push_back({ type, rand() % LANES, SPAWN_X, SODA_HP[type], 0.3f, 0, 0 });
            if (spawned == 0) PlaySound("wave");
            spawned++;
            spawnTimer = 4.0f + rand() % 3;   // 4 to 6 seconds
        }
    }
    else if (sodas.empty())                     // the whole wave is beaten
    {
        if (wave == 3) return true;
        betweenWaves = true;
        spawnTimer = 3.0f;
    }
    return false;
}

// The first candy in front of this soda (the candy with the biggest x to its left).
static Candy* CandyInFront(const Soda& s, float reach)
{
    Candy* best = nullptr;
    for (Candy& c : candies)
        if (c.lane == s.lane && s.x - c.x >= -0.05f && s.x - c.x <= reach && (!best || c.x > best->x))
            best = &c;
    return best;
}

bool UpdateSodas(float dt)
{
    bool reachedJar = false;
    for (Soda& s : sodas)
    {
        s.hurt -= dt; s.attack -= dt; s.cooldown -= dt;
        Candy* blocker = CandyInFront(s, TOUCH);
        if (!blocker)
            s.x -= SODA_SPEED[s.type] * dt;      // walk left unless a candy blocks the way
        if (s.x <= BASE_X) reachedJar = true;

        if (s.cooldown > 0) continue;
        if (s.type == CUP)                        // hard: swings its straw all the time, it only hurts up close
        {
            s.attack = ATTACK_TIME; s.cooldown = 1.6f;
            if (blocker) {
                blocker->hp -= SODA_DAMAGE[CUP]; blocker->hurt = 0.2f;
                PlaySound("whack");
            }
        }
        else                                      // bottle: fizz up high, can: syrup down low
        {
            s.attack = ATTACK_TIME; s.cooldown = 5.0f;
            bool high = (s.type == BOTTLE);
            shots.push_back({ s.lane, s.x - 0.08f, FloorY(s.lane) + (high ? 0.27f : 0.03f), -0.6f, high, 1, 0.0f });
            PlaySound("fizz");
        }
    }
    // defeated sodas drop sugar cubes
    for (size_t i = 0; i < sodas.size(); )
    {
        if (sodas[i].hp <= 0) {
            cubes.push_back({ sodas[i].lane, sodas[i].x, SODA_SUGAR[sodas[i].type] });
            PlaySound("defeated");
            sodas.erase(sodas.begin() + i);
        }
        else ++i;
    }
    return reachedJar;
}

void UpdateShots(float dt)
{
    for (size_t i = 0; i < shots.size(); )
    {
        Shot& sh = shots[i];
        bool remove = false;
        if (sh.pop > 0) {                         // candy pellet popping after a hit
            sh.pop -= dt;
            remove = sh.pop <= 0;
        }
        else {
            sh.x += sh.speed * dt;
            if (sh.speed > 0) {                   // candy pellet: hits the first soda, never candies
                for (Soda& s : sodas)
                    if (s.lane == sh.lane && std::fabs(s.x - sh.x) < 0.06f) {
                        s.hp -= sh.damage; s.hurt = 0.2f; sh.pop = 0.2f;
                        break;
                    }
                remove = sh.x > LANE_RIGHT;
            }
            else {                                // soda shot: hits the first candy that does not dodge
                Candy* target = nullptr;
                for (Candy& c : candies) {
                    if (c.lane != sh.lane || std::fabs(c.x - sh.x) > 0.06f) continue;
                    bool dodged = c.type != CHOCOLATE && ((sh.high && c.duck > 0) || (!sh.high && c.jump > 0));
                    if (!dodged && (!target || c.x > target->x)) target = &c;
                }
                if (target) {
                    target->hp -= sh.damage; target->hurt = 0.2f;
                    PlaySound("hurt");
                    remove = true;
                }
                if (sh.x < BASE_X) remove = true;
            }
        }
        if (remove) shots.erase(shots.begin() + i);
        else ++i;
    }
}

static void AngryEyes(float x, float y, float s)
{
    Circle(x, y, 0.014f * s, 0.016f * s, BLACK);
    Circle(x - 0.035f * s, y, 0.014f * s, 0.016f * s, BLACK);
    Rect(x - 0.017f * s, y + 0.025f * s, 0.07f * s, 0.012f * s, BLACK, -0.3f);   // angry brows (facing left)
}

void DrawSoda(SodaType type, float x, float footY, float s, float attack)
{
    float shake = 0.08f * std::sin(attack * 18.0f) * (attack > 0 ? 1.0f : 0.0f);
    if (type == BOTTLE)        // tall green glass bottle
    {
        glm::vec3 glass(0.25f, 0.65f, 0.35f);
        Rect(x, footY + 0.1f * s, 0.11f * s, 0.2f * s, glass, shake);
        Circle(x, footY + 0.2f * s, 0.055f * s, 0.03f * s, glass);                  // shoulders
        Rect(x, footY + 0.25f * s, 0.045f * s, 0.08f * s, glass);                   // neck
        Rect(x, footY + 0.295f * s, 0.055f * s, 0.025f * s, glm::vec3(0.85f, 0.15f, 0.15f));   // cap
        Rect(x, footY + 0.08f * s, 0.11f * s, 0.06f * s, WHITE, shake);             // label
        Rect(x, footY + 0.08f * s, 0.11f * s, 0.015f * s, glm::vec3(0.85f, 0.15f, 0.15f), shake);
        AngryEyes(x - 0.005f * s, footY + 0.15f * s, s);
    }
    else if (type == CAN)      // short red can
    {
        glm::vec3 red(0.8f, 0.1f, 0.12f), metal(0.8f, 0.8f, 0.85f);
        Rect(x, footY + 0.075f * s, 0.13f * s, 0.14f * s, red, shake);
        Circle(x, footY + 0.15f * s, 0.065f * s, 0.018f * s, metal);                // top
        Circle(x, footY + 0.005f * s, 0.065f * s, 0.015f * s, metal);               // bottom
        Rect(x + 0.02f * s, footY + 0.165f * s, 0.03f * s, 0.012f * s, glm::vec3(0.6f));   // ring pull
        Rect(x, footY + 0.05f * s, 0.13f * s, 0.02f * s, WHITE, shake);             // wave stripe
        AngryEyes(x, footY + 0.1f * s, s);
    }
    else                       // big fountain cup with a lid and a straw
    {
        glm::vec3 cupColor(0.95f, 0.95f, 0.95f), stripe(0.2f, 0.4f, 0.85f);
        Rect(x, footY + 0.12f * s, 0.17f * s, 0.24f * s, cupColor);
        Tri(x - 0.085f * s, footY + 0.12f * s, 0.04f * s, 0.24f * s, glm::vec3(0.96f, 0.9f, 0.8f), 3.14159f);  // cup gets narrower at the bottom
        Tri(x + 0.085f * s, footY + 0.12f * s, 0.04f * s, 0.24f * s, glm::vec3(0.96f, 0.9f, 0.8f), 3.14159f);
        Rect(x, footY + 0.08f * s, 0.15f * s, 0.035f * s, stripe);
        Rect(x, footY + 0.15f * s, 0.16f * s, 0.035f * s, stripe);
        Rect(x, footY + 0.25f * s, 0.2f * s, 0.03f * s, glm::vec3(0.85f));          // lid
        float straw = 0.3f + 1.2f * attack;                                         // the straw whacks forward
        Rect(x - std::sin(straw) * 0.07f * s, footY + 0.27f * s + std::cos(straw) * 0.07f * s,
             0.022f * s, 0.15f * s, glm::vec3(0.9f, 0.2f, 0.2f), straw);
        AngryEyes(x, footY + 0.2f * s, s);
    }
}

void DrawSodas()
{
    for (int lane = LANES - 1; lane >= 0; --lane)
        for (Soda& s : sodas)
        {
            if (s.lane != lane) continue;
            float attack = s.attack > 0 ? 1.0f - s.attack / ATTACK_TIME : 0.0f;
            redness = 0.6f * (1.0f - (float)s.hp / SODA_HP[s.type]) + (s.hurt > 0 ? 0.3f : 0.0f);
            DrawSoda(s.type, s.x, FloorY(lane), 1.0f, attack);
            redness = 0;
            DrawHealth(s.x, FloorY(lane) - 0.05f, s.hp, SODA_HP[s.type], false);
        }
}

void DrawShots()
{
    for (Shot& sh : shots)
    {
        if (sh.speed > 0) {
            if (sh.pop > 0)                       // the pellet pops into pieces
                for (int k = 0; k < 6; ++k)
                    Circle(sh.x + std::cos(k * 1.05f) * (0.2f - sh.pop) * 0.4f,
                           sh.y + std::sin(k * 1.05f) * (0.2f - sh.pop) * 0.4f, 0.012f, 0.012f, glm::vec3(1.0f, 0.4f, 0.7f));
            else
                Circle(sh.x, sh.y, 0.022f, 0.022f, glm::vec3(1.0f, 0.4f, 0.7f));
        }
        else if (sh.high) {                       // fizz bubbles
            Circle(sh.x, sh.y, 0.025f, 0.025f, glm::vec3(0.7f, 0.9f, 1.0f));
            Circle(sh.x + 0.03f, sh.y + 0.015f, 0.012f, 0.012f, glm::vec3(0.7f, 0.9f, 1.0f));
        }
        else {                                    // syrup blob sliding on the shelf
            Circle(sh.x, sh.y, 0.035f, 0.018f, glm::vec3(0.45f, 0.25f, 0.1f));
        }
    }
}
