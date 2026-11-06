#include "Candies.h"
#include "Shapes.h"
#include "Sound.h"
#include "Hud.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

const float FirstAttack = 0.3f;
const float AttackTime = 0.35f;
const float LollipopCooldown = 0.8f;
const float BonbonCooldown = 5.0f;
const float CaneReach = 0.06f;
const float PelletSpeed = 0.6f;
const float SodaFlash = 0.2f;
const float PickupDistance = 0.08f;
const float JumpTime = 0.7f;
const float JumpHeight = 0.25f;
const float DuckTime = 0.7f;
const float DuckSquash = 0.55f;
const float LauncherX = 0.16f;
const float LauncherY = 0.15f;
const float FridgeGap = 0.1f;
const float Behind = 0.05f;
const glm::vec3 Black(0.1f);

static void Buy(CandyType type, int lane, float x)
{
    candies.push_back({ type, lane, x, CandyHp[type], FirstAttack, 0, 0, 0, 0 });
    sugar -= CandyCost[type];
}

void PlaceCandies(CandyType type)
{
    bool placed = false;
    for (int lane = 0; lane < Lanes; ++lane)
    {
        if (laneSelected[lane] && CardUnlocked(type)){
            Buy(type, lane, StartX);
            placed = true;
        }
    }
    if (placed) {
        PlaySound("place");
    }
    else {
        PlaySound("locked");
    }
}

void PlaceChocolate(int lane, float x)
{
    placingChocolate = false;
    if (CardUnlocked(Chocolate))
    {
        if (x < StartX)
        {
            x = StartX;
        }
        else if (x > LaneRight - FridgeGap)
        {
            x = LaneRight - FridgeGap;
        }

        Buy(Chocolate, lane, x);
        PlaySound("place");
    }
    else
    {
        PlaySound("locked");
    }
}

static Soda* SodaInFront(const Candy& c, float reach)
{
    for (int i = 0; i < sodas.size(); i++)
    {
        Soda& s = sodas[i];
        if (s.lane == c.lane)
        {
            if (s.x - c.x >= -Behind && s.x - c.x <= reach)
            {
                return &s;
            }
        }
    }

    return nullptr;
}

static void JumpAndDuck()
{
    bool up = keyPressed[GLFW_KEY_W] || keyPressed[GLFW_KEY_UP];
    bool down = keyPressed[GLFW_KEY_S] || keyPressed[GLFW_KEY_DOWN];
    for (int i = 0; i < candies.size(); i++)
    {
        Candy& c = candies[i];
        if (c.type != Chocolate && laneSelected[c.lane])
        {
            if (c.jump <= 0 && c.duck <= 0)
            {
                if (up)
                {
                    c.jump = JumpTime;
                }
                else if (down)
                {
                    c.duck = DuckTime;
                }
            }
        }
    }

    if (up)
    {
        PlaySound("jump");
    }

    if (down)
    {
        PlaySound("duck");
    }
}

static void WalkAndAttack(Candy& c, float dt)
{
    if (!SodaInFront(c, Touch))
        c.x += CandySpeed[c.type] * dt;

    c.cooldown -= dt;
    if (c.cooldown > 0) 
    {
        return;
    }
    c.attack = AttackTime;
    if (c.type == Lollipop) {
        c.cooldown = LollipopCooldown;
        Soda* s = SodaInFront(c, Touch + CaneReach);
        if (s != nullptr) { 
            Soda& soda = *s;
            soda.hp -= 1; 
            soda.hurt = SodaFlash; 
            PlaySound("whack"); 
        }
    } else {
        c.cooldown = BonbonCooldown;
        shots.push_back({ c.lane, c.x + LauncherX, FloorY(c.lane) + LauncherY, PelletSpeed, false, 1, 0.0f });
        PlaySound("pellet");
    }
}

static void PickUpSugar(Candy& c)
{
    for (size_t i = 0; i < cubes.size(); ++i)
        if (cubes[i].lane == c.lane && std::fabs(cubes[i].x - c.x) < PickupDistance) {
            sugar += cubes[i].value;
            sugarCollected += cubes[i].value;
            cubes.erase(cubes.begin() + i);
            PlaySound("sugar");
            return;
        }
}

static void RemoveDefeated()
{
    for (size_t i = 0; i < candies.size(); )
    {
        if (candies[i].hp <= 0 || candies[i].x > LaneRight) {
            if (candies[i].hp <= 0) PlaySound("defeated");
            candies.erase(candies.begin() + i);
        }
        else 
        {
            ++i;
        }
    }
}

void UpdateCandies(float dt)
{
    JumpAndDuck();
    for (int i = 0; i < candies.size(); i++)
    {
        Candy& c = candies[i];
        c.jump -= dt;
        c.duck -= dt;
        c.attack -= dt;
        if (c.type != Chocolate)
        {
            WalkAndAttack(c, dt);
        }

        PickUpSugar(c);
    }

    RemoveDefeated();
}

static void Eyes(float x, float y)
{
    Circle(x, y, 0.014f, 0.016f, Black);
    Circle(x + 0.035f, y, 0.014f, 0.016f, Black);
}

void DrawCandy(CandyType type, float x, float footY, float s, float attack, float squash)
{
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, footY, 0.0f));
    model = glm::scale(model, glm::vec3(s, s * squash, 1.0f));

    if (type == Lollipop)
    {
        glm::vec3 pink(0.95f, 0.35f, 0.6f);
        Rect(0.0f, 0.09f, 0.022f, 0.18f, glm::vec3(1.0f));
        Circle(0.0f, 0.24f, 0.085f, 0.085f, pink);
        Eyes(-0.005f, 0.25f);
        float poke = 0.05f * std::sin(3.14159f * attack);
        Rect(0.07f + poke, 0.13f, 0.15f, 0.024f, glm::vec3(0.85f, 0.1f, 0.1f));
    }
    else if (type == Bonbon)
    {
        glm::vec3 orange(1.0f, 0.6f, 0.1f), yellow(1.0f, 0.9f, 0.3f);
        Rect(-0.04f, 0.05f, 0.02f, 0.1f, orange);
        Rect(0.04f, 0.05f, 0.02f, 0.1f, orange);
        Tri(-0.14f, 0.2f, 0.11f, 0.09f, yellow, -1.5708f);
        Tri(0.14f, 0.2f, 0.11f, 0.09f, yellow, 1.5708f);
        Circle(0.0f, 0.2f, 0.11f, 0.105f, orange);
        Eyes(-0.005f, 0.24f);
        Rect(0.1f, 0.15f, 0.11f, 0.035f, glm::vec3(0.4f));
    }
    else
    {
        glm::vec3 brown(0.4f, 0.22f, 0.1f), lightBrown(0.6f, 0.38f, 0.2f);
        Rect(0.0f, 0.155f, 0.16f, 0.3f, brown);
        Rect(0.0f, 0.155f, 0.012f, 0.3f, lightBrown);
        Rect(0.0f, 0.155f, 0.16f, 0.012f, lightBrown);
        Eyes(-0.0175f, 0.25f);
    }
    model = glm::mat4(1.0f);
}

void DrawCandies()
{
    for (int i = 0; i < candies.size(); i++)
    {
        Candy& c = candies[i];
        float lift;
        if (c.jump > 0)
        {
            lift = JumpHeight * std::sin(3.14159f * (1 - c.jump / JumpTime));
        }
        else
        {
            lift = 0.0f;
        }

        float squash;
        if (c.duck > 0)
        {
            squash = DuckSquash;
        }
        else
        {
            squash = 1.0f;
        }

        float attack;
        if (c.attack > 0)
        {
            attack = 1.0f - c.attack / AttackTime;
        }
        else
        {
            attack = 0.0f;
        }

        float health = (float)c.hp / CandyHp[c.type];
        redness = 0.6f * (1.0f - health);
        DrawCandy(c.type, c.x, FloorY(c.lane) + lift, 1.0f, attack, squash);
        redness = 0;
        DrawHealth(c.x, FloorY(c.lane) - 0.05f, c.hp, CandyHp[c.type], true);
    }
}
