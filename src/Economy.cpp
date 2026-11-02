#include "Economy.h"
#include "GameConfig.h"
#include "Renderer2D.h"
#include "Hud.h"
#include "Audio.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace
{
    struct FieldCoin
    {
        int lane;
        float x;
        int value;
        float age;
        bool taken;
    };

    int coins = START_COINS;
    std::vector<FieldCoin> field;

    const float PICKUP_DIST = 0.45f;
}

namespace Economy
{
    void Init()
    {
        coins = START_COINS;
        field.clear();
    }

    int GetCoins() { return coins; }
    bool CanAfford(int cost) { return coins >= cost; }
    void Spend(int cost) { coins -= cost; }

    void DropCoins(int lane, float x, int value)
    {
        x = std::min(WORLD_WIDTH - 0.5f, std::max(LANE_START_X + 0.3f, x));
        field.push_back({ lane, x, value, 0.0f, false });
    }

    void CollectAt(int lane, float x)
    {
        for (FieldCoin& c : field)
        {
            if (c.taken || c.lane != lane || c.age < 0.4f || std::fabs(c.x - x) > PICKUP_DIST)
                continue;
            c.taken = true;
            coins += c.value;
            Audio::Play(Audio::Sfx::Coin);
            Hud::AddFloatText(glm::vec2(c.x, LANE_Y[lane] + 0.3f), "+" + std::to_string(c.value), glm::vec3(1.0f, 0.86f, 0.3f));
            Hud::AddSpark(glm::vec2(c.x, LANE_Y[lane] - 0.3f), glm::vec3(1.0f, 0.85f, 0.25f));
        }
        field.erase(std::remove_if(field.begin(), field.end(), [](const FieldCoin& c) { return c.taken; }), field.end());
    }

    void Update(float deltaTime)
    {
        for (FieldCoin& c : field)
            c.age += deltaTime;
    }

    void DrawCoin(glm::vec2 center, float radius, float spin)
    {
        float sx = std::max(0.15f, std::fabs(spin));
        const glm::vec3 gold(0.93f, 0.74f, 0.25f), goldDark(0.62f, 0.45f, 0.10f), silver(0.82f, 0.84f, 0.86f);
        Renderer2D::DrawEllipse(center, glm::vec2(radius * sx + 0.02f, radius + 0.02f), glm::vec3(0.25f, 0.17f, 0.05f));
        Renderer2D::DrawEllipse(center, glm::vec2(radius * sx, radius), gold);                        // brass ring
        Renderer2D::DrawEllipse(center, glm::vec2(radius * 0.66f * sx, radius * 0.66f), silver);     // silver center
        if (sx > 0.6f)
        {
            // the "1" of the one-pound coin
            Renderer2D::DrawQuad(center, glm::vec2(radius * 0.14f, radius * 0.7f), goldDark);
            Renderer2D::DrawQuad(center + glm::vec2(-radius * 0.1f, radius * 0.25f), glm::vec2(radius * 0.2f, radius * 0.12f), goldDark);
        }
        Renderer2D::DrawEllipse(center + glm::vec2(-radius * 0.45f * sx, radius * 0.45f), glm::vec2(radius * 0.12f * sx, radius * 0.12f), glm::vec3(1.0f, 0.95f, 0.75f));
    }

    void RenderLane(int lane)
    {
        for (const FieldCoin& c : field)
        {
            if (c.lane != lane)
                continue;
            // pops out of the defeated enemy, bounces, then lies there spinning
            float t = c.age;
            float bounce = (t < 0.6f) ? 0.9f * std::sin(3.14159f * t / 0.6f) * (1.0f - t / 0.6f * 0.4f) : 0.04f * std::sin(t * 4.0f);
            glm::vec2 pos(c.x, LANE_Y[lane] - FOOT_OFFSET + 0.18f + bounce);
            Renderer2D::DrawEllipse(glm::vec2(c.x, LANE_Y[lane] - FOOT_OFFSET + 0.02f), glm::vec2(0.15f, 0.04f), glm::vec3(0.45f, 0.36f, 0.25f));
            DrawCoin(pos, 0.15f, std::cos(t * 3.0f));
        }
    }
}
