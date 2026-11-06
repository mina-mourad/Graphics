// [LAB] Drawing: Rect / Tri / Circle from Shapes; sin() wobble (Lab 2).
// [NOT LAB] rand() for random shelves, game rules in plain C++.
// Below every part that uses lab code there is a "LAB CODE" comment with the
// lab lines exactly as they are written in the lab files.
// Parts with no lab code say "no lab code".
// (All drawing uses Rect / Tri / Circle; their lab code is in Shapes.cpp.)
#include "Sodas.h"
#include "Shapes.h"
#include "Sound.h"
#include "Hud.h"
#include <cstdlib>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

const int WaveSize[3] = { 3, 5, 10 };
static int spawned = 0;
static float spawnTimer = 2.0f;
static bool betweenWaves = false;   // the last wave is beaten, the next one has not started yet
const float AttackTime = 0.35f;
const glm::vec3 Black(0.1f), White(1.0f);

void ResetWaves() { 
    spawned = 0; 
    spawnTimer = 2.0f; 
    wave = 1; 
    betweenWaves = false; 
}
// no lab code

bool UpdateWaves(float dt)
{
    if (betweenWaves)                           // short break, then the next wave starts
    {                                           // together with its first soda
        spawnTimer -= dt;
        if (spawnTimer > 0) 
        {
            return false;
        }
        betweenWaves = false; 
        wave++; 
        spawned = 0;   // the new wave starts now, with its first soda below
    }
    if (spawned < WaveSize[wave - 1])
    {
        spawnTimer -= dt;
        if (spawnTimer <= 0)
        {
            // wave 1: bottles only, wave 2: bottles and cans, wave 3: all three
            SodaType type = Bottle;
            if (wave == 2) {
                if (spawned % 2 == 0) {
                    type = Bottle;
                }
                else {
                    type = Can;
                }
            }
            if (wave == 3) 
            {
                type = (SodaType)(spawned % 3);
            }
            sodas.push_back({ type, rand() % Lanes, SpawnX, SodaHp[type], 0.3f, 0, 0 });
            if (spawned == 0) 
            {
                PlaySound("wave");
            }
            spawned++;
            spawnTimer = 4.0f + rand() % 3;   // 4 to 6 seconds
        }
    }
    else if (sodas.empty())                     // the whole wave is beaten
    {
        if (wave == 3) 
        {
            return true;
        }
        betweenWaves = true;
        spawnTimer = 3.0f;
    }
    return false;
}
// no lab code

// The first candy in front of this soda (the candy with the biggest x to its left).
static Candy* CandyInFront(const Soda& s, float reach)
{
    Candy* best = nullptr;
    for (int i = 0; i < candies.size(); i++)
    {
        Candy& c = candies[i];
        if (c.lane == s.lane)
        {
            if (s.x - c.x >= -0.05f && s.x - c.x <= reach)
            {
                if (best == nullptr)
                {
                    best = &c;
                }
                else
                {
                    Candy& bestCandy = *best;
                    if (c.x > bestCandy.x)
                    {
                        best = &c;
                    }
                }
            }
        }
    }

    return best;
}
// no lab code

bool UpdateSodas(float dt)
{
    bool reachedJar = false;
    for (int i = 0; i < sodas.size(); i++)
    {
        Soda& s = sodas[i];
        s.hurt -= dt;
        s.attack -= dt;
        s.cooldown -= dt;
        Candy* blocker = CandyInFront(s, Touch);
        if (blocker == nullptr)
        {
            s.x -= SodaSpeed[s.type] * dt;
        }

        // LAB CODE (Lab 4 Code/Lab4Camera.cpp):
        //   float speed = MOVE_SPEED * deltaTime;
        // LAB CODE (Lab 3 Notes/main.cpp):
        //   if (key == GLFW_KEY_A && action == GLFW_REPEAT) posX -= moveSpeed;
        // (moving = position - speed; times deltaTime so it is the same speed on every computer)

        if (s.x <= BaseX)
        {
            reachedJar = true;
        }

        if (s.cooldown <= 0)
        {
            if (s.type == Cup)
            {
                s.attack = AttackTime;
                s.cooldown = 1.6f;
                if (blocker != nullptr)
                {
                    Candy& candy = *blocker;
                    candy.hp -= SodaDamage[Cup];
                    candy.hurt = 0.2f;
                    PlaySound("whack");
                }
            }
            else
            {
                s.attack = AttackTime;
                s.cooldown = 5.0f;
                bool high = (s.type == Bottle);
                float shotHeight;
                if (high)
                {
                    shotHeight = 0.27f;
                }
                else
                {
                    shotHeight = 0.03f;
                }

                shots.push_back({s.lane,s.x - 0.08f,FloorY(s.lane) + shotHeight,-0.6f, high, 1, 0.0f});
                PlaySound("fizz");
            }
        }
    }

    // Defeated sodas drop sugar cubes
    for (int i = 0; i < sodas.size(); i++)
    {
        if (sodas[i].hp <= 0)
        {
            cubes.push_back({sodas[i].lane,sodas[i].x, SodaSugar[sodas[i].type]});
            PlaySound("defeated");
            sodas.erase(sodas.begin() + i);
            i--;
        }
    }

    return reachedJar;

    // no lab code (attacks, shots and sugar drops are game rules)
}

void UpdateShots(float dt)
{
    for (int i = 0; i < shots.size(); i++)
    {
        Shot& sh = shots[i];
        bool remove = false;
        if (sh.pop > 0)
        {
            sh.pop -= dt;
            if (sh.pop <= 0)
            {
                remove = true;
            }
        }
        else
        {
            sh.x += sh.speed * dt;

            // LAB CODE (Lab 4 Code/Lab4Camera.cpp):
            //   float speed = MOVE_SPEED * deltaTime;
            // LAB CODE (Lab 3 Notes/main.cpp):
            //   if (key == GLFW_KEY_D && action == GLFW_REPEAT) posX += moveSpeed;

            if (sh.speed > 0)
            {
                // Candy pellet: hits the first soda, never candies
                for (int j = 0; j < sodas.size(); j++)
                {
                    Soda& s = sodas[j];
                    if (s.lane == sh.lane && std::fabs(s.x - sh.x) < 0.06f)
                    {
                        s.hp -= sh.damage;
                        s.hurt = 0.2f;
                        sh.pop = 0.2f;
                        break;
                    }
                }

                if (sh.x > LaneRight)
                {
                    remove = true;
                }
            }
            else
            {
                // Soda shot: hits the first candy that does not dodge
                Candy* target = nullptr;
                for (int j = 0; j < candies.size(); j++)
                {
                    Candy& c = candies[j];
                    if (c.lane == sh.lane && std::fabs(c.x - sh.x) <= 0.06f)
                    {
                        bool dodged = false;
                        if (c.type != Chocolate)
                        {
                            if (sh.high && c.duck > 0)
                            {
                                dodged = true;
                            }
                            else if (!sh.high && c.jump > 0)
                            {
                                dodged = true;
                            }
                        }

                        if (dodged == false)
                        {
                            if (target == nullptr)
                            {
                                target = &c;
                            }
                            else
                            {
                                Candy& targetCandy = *target;
                                if (c.x > targetCandy.x)
                                {
                                    target = &c;
                                }
                            }
                        }
                    }
                }

                if (target != nullptr)
                {
                    Candy& targetCandy = *target;
                    targetCandy.hp -= sh.damage;
                    targetCandy.hurt = 0.2f;
                    PlaySound("hurt");
                    remove = true;
                }

                if (sh.x < BaseX)
                {
                    remove = true;
                }
            }
        }

        if (remove)
        {
            shots.erase(shots.begin() + i);
            i--;
        }
    }

    // no lab code (hitting, dodging and removing shots are game rules)
}

static void Eyes(float x, float y)          // two eyes looking left, at the candies
{
    Circle(x, y, 0.014f, 0.016f, Black);
    Circle(x - 0.035f, y, 0.014f, 0.016f, Black);
}
// no lab code

void DrawSoda(SodaType type, float x, float footY, float s, float attack)
{
    // Like Lab 2: one transform moves the whole soda to its place and scales it.
    // The parts below are drawn around (0, 0) = its feet.
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, footY, 0.0f));
    model = glm::scale(model, glm::vec3(s, s, 1.0f));
    // LAB CODE (Lab 2 Template/main.cpp):
    //   glm::mat4 transform = glm::mat4(1.0f);
    //   //transform = glm::translate(transform, glm::vec3(offsetX, 0.0f, 0.0f));
    //   //transform = glm::scale(transform, glm::vec3(pulse, pulse, 1.0f));
    // (model is used by every shape below - see Prepare() in Shapes.cpp)

    if (type == Bottle)        // easy: a green bottle (fizz up high)
    {
        glm::vec3 green(0.25f, 0.65f, 0.35f);
        Rect(0.0f, 0.11f, 0.11f, 0.22f, green);                   // body
        Rect(0.0f, 0.25f, 0.045f, 0.08f, green);                  // neck
        Rect(0.0f, 0.295f, 0.06f, 0.025f, green);                 // cap
        Eyes(-0.005f, 0.15f);
    }
    else if (type == Can)      // medium: a red can (syrup down low)
    {
        glm::vec3 red(0.8f, 0.1f, 0.12f);
        Rect(0.0f, 0.075f, 0.13f, 0.15f, red);                    // can
        Rect(0.0f, 0.16f, 0.1f, 0.02f, red);                      // narrower top
        Eyes(0.0f, 0.1f);
    }
    else                       // hard: a blue cup with a lid and a straw (hits up close)
    {
        glm::vec3 blue(0.2f, 0.4f, 0.85f);
        Rect(0.0f, 0.12f, 0.17f, 0.24f, blue);                    // cup
        Rect(0.0f, 0.25f, 0.2f, 0.03f, blue);                     // lid
        float straw = 0.3f + 1.2f * attack;                       // the straw whacks forward
        Rect(-std::sin(straw) * 0.07f, 0.27f + std::cos(straw) * 0.07f, 0.022f, 0.15f, blue, straw);
        // LAB CODE (Lab 3 Notes/main.cpp):
        //   cosf(angle), sinf(angle), 0.0f,
        // (the straw turns around the lid: a point on a circle, the same math as the lab circle)
        Eyes(0.0f, 0.2f);
    }
    model = glm::mat4(1.0f);   // back to "no change" for whatever is drawn next
    // no lab code (the shapes of the sodas)
}

void DrawSodas()
{
    for (int lane = Lanes - 1; lane >= 0; lane--)
    {
        for (int i = 0; i < sodas.size(); i++)
        {
            Soda& s = sodas[i];
            if (s.lane == lane)
            {
                float attack;
                if (s.attack > 0)
                {
                    attack = 1.0f - s.attack / AttackTime;
                }
                else
                {
                    attack = 0.0f;
                }

                float health = (float)s.hp / SodaHp[s.type];
                redness = 0.6f * (1.0f - health);
                if (s.hurt > 0)
                {
                    redness += 0.3f;
                }

                // LAB CODE (Lab 3 Notes/main.cpp):
                //   glUniform1f(locHighlight, triangleHighlight ? 1.0f : 0.0f);
                // (redness goes to u_Red in the shader, the same way the lab sends u_Highlight - see Shapes.cpp)

                DrawSoda(s.type, s.x, FloorY(lane), 1.0f, attack);
                redness = 0;
                DrawHealth(s.x, FloorY(lane) - 0.05f, s.hp, SodaHp[s.type], false);
            }
        }
    }
}
// no lab code

void DrawShots()
{
    for (int i = 0; i < shots.size(); i++)
    {
        Shot& sh = shots[i];
        if (sh.speed > 0)
        {
            if (sh.pop > 0)
            {
                // The pellet pops into pieces
                for (int k = 0; k < 6; k++)
                {
                    Circle(sh.x + std::cos(k * 1.05f) * (0.2f - sh.pop) * 0.4f, sh.y + std::sin(k * 1.05f) * (0.2f - sh.pop) * 0.4f,0.012f, 0.012f, glm::vec3(1.0f, 0.4f, 0.7f)
                    );
                }

                // LAB CODE (Lab 3 Notes/main.cpp):
                //   cosf(angle), sinf(angle), 0.0f,
                // (6 pieces fly out on a growing circle, the same math as the lab circle)
            }
            else
            {
                Circle(sh.x, sh.y, 0.022f, 0.022f, glm::vec3(1.0f, 0.4f, 0.7f));
            }
        }
        else
        {
            if (sh.high)
            {
                // A fizz bubble up high
                Circle(sh.x, sh.y, 0.025f, 0.025f, glm::vec3(0.7f, 0.9f, 1.0f));
            }
            else
            {
                // Syrup blob sliding on the shelf
                Circle(sh.x, sh.y, 0.035f, 0.018f, glm::vec3(0.45f, 0.25f, 0.1f));
            }
        }
    }

    // no lab code (the shapes of the shots)
}
