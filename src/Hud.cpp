#include "Hud.h"
#include "GameConfig.h"
#include "Renderer2D.h"
#include "CharacterRenderer.h"
#include "PlayerSystem.h"
#include "WaveManager.h"
#include "EnemySystem.h"
#include "Economy.h"
#include "Input.h"
#include "Audio.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
    using glm::vec2;
    using glm::vec3;

    float triSign(vec2 p1, vec2 p2, vec2 p3)
    {
        return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
    }

    bool pointInTriangle(vec2 pt, vec2 a, vec2 b, vec2 c)
    {
        float d1 = triSign(pt, a, b), d2 = triSign(pt, b, c), d3 = triSign(pt, c, a);
        bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
        return !(hasNeg && hasPos);
    }

    struct Spark { vec2 pos; vec3 color; float t; };
    struct FloatText { vec2 pos; std::string text; vec3 color; float t; };

    std::vector<Spark> sparks;
    std::vector<FloatText> floaters;
    std::string toast;
    float toastTime = 0.0f;
    float uiTime = 0.0f;
    float cardPress[3] = {};
    bool pauseClicked = false;

    const vec2 CARD_SIZE(1.45f, 1.55f);
    vec2 CardCenter(int i) { return vec2(0.95f + 1.6f * i, 8.15f); }
    const vec2 PAUSE_POS(15.5f, 8.3f);
    const vec2 PAUSE_SIZE(0.7f, 0.7f);

    const char* CARD_NAMES[3] = { "BAWAB", "TOMATO", "ME'ALLEM" };

    bool CardUnlocked(int i) { return Economy::CanAfford(PLAYER_COST[i]); }

    void UseCard(int i)
    {
        cardPress[i] = 1.0f;
        PlayerSystem::PlaceOnSelectedLanes(static_cast<PlayerType>(i));
    }

    void DrawCard(int i, bool hover)
    {
        const vec2 c = CardCenter(i) + vec2(0.0f, -0.06f * cardPress[i]);
        const bool unlocked = CardUnlocked(i);

        // Locked cards are drawn in grayscale and ignore the mouse.
        Renderer2D::SetGray(unlocked ? 0.0f : 1.0f);
        float hl = (unlocked && hover) ? 0.22f : 0.0f;

        // wooden frame like the koshk's sign, with a light inside
        Renderer2D::DrawQuad(c, CARD_SIZE + vec2(0.08f), vec3(0.18f, 0.10f, 0.05f));
        Renderer2D::DrawQuad(c, CARD_SIZE, vec3(0.62f, 0.38f, 0.18f), hl);
        Renderer2D::DrawQuad(c + vec2(0.0f, 0.12f), vec2(CARD_SIZE.x - 0.16f, CARD_SIZE.y - 0.52f), vec3(0.95f, 0.86f, 0.62f), hl);
        // little sun rays behind the portrait
        for (int k = 0; k < 6; ++k)
        {
            float a = uiTime * 0.3f + k * 1.047f;
            Renderer2D::DrawTriangle(c + vec2(0.0f, 0.2f), c + vec2(0.0f, 0.2f) + vec2(std::cos(a), std::sin(a)) * 0.5f,
                                     c + vec2(0.0f, 0.2f) + vec2(std::cos(a + 0.35f), std::sin(a + 0.35f)) * 0.5f, vec3(0.98f, 0.92f, 0.70f));
        }

        // portrait (the real player model, small)
        float scale = (i == 2) ? 0.36f : 0.40f;
        CharacterRenderer::DrawPlayerAt(c + vec2(i == 2 ? -0.12f : -0.05f, -0.30f), scale, static_cast<PlayerType>(i),
                                        CharacterRenderer::PlayerAnim::Idle, uiTime + i);

        // key number and name
        Renderer2D::DrawQuad(c + vec2(-0.56f, 0.6f), vec2(0.28f, 0.28f), vec3(0.18f, 0.10f, 0.05f));
        Renderer2D::DrawText(std::to_string(i + 1), c + vec2(-0.56f, 0.6f), 0.11f, vec3(1.0f, 0.95f, 0.8f));
        Renderer2D::DrawQuad(c + vec2(0.0f, -0.42f), vec2(CARD_SIZE.x - 0.16f, 0.2f), vec3(0.30f, 0.17f, 0.08f));
        Renderer2D::DrawText(CARD_NAMES[i], c + vec2(0.0f, -0.42f), 0.105f, vec3(1.0f, 0.95f, 0.85f));

        // cost
        Economy::DrawCoin(c + vec2(-0.30f, -0.63f), 0.1f);
        Renderer2D::DrawTextAligned(std::to_string(PLAYER_COST[i]), c + vec2(-0.14f, -0.63f), 0.11f, vec3(1.0f, 0.95f, 0.8f), Renderer2D::Align::Left);

        if (!unlocked)
        {
            // padlock
            vec2 l = c + vec2(0.48f, 0.55f);
            Renderer2D::DrawCircle(l + vec2(0.0f, 0.08f), 0.09f, vec3(0.2f));
            Renderer2D::DrawCircle(l + vec2(0.0f, 0.08f), 0.05f, vec3(0.62f, 0.38f, 0.18f));
            Renderer2D::DrawQuad(l + vec2(0.0f, -0.02f), vec2(0.22f, 0.16f), vec3(0.25f));
        }
        Renderer2D::SetGray(0.0f);
    }
}

namespace Hud
{
    void Init()
    {
        sparks.clear();
        floaters.clear();
        toast.clear();
        toastTime = 0.0f;
        pauseClicked = false;
    }

    bool InBox(vec2 p, vec2 c, vec2 size)
    {
        vec2 h = size * 0.5f;
        vec2 a(c.x - h.x, c.y - h.y), b(c.x + h.x, c.y - h.y), cc(c.x + h.x, c.y + h.y), d(c.x - h.x, c.y + h.y);
        return pointInTriangle(p, a, b, cc) || pointInTriangle(p, a, cc, d);
    }

    void DrawPanel(vec2 c, vec2 size, float highlight)
    {
        Renderer2D::DrawQuad(c + vec2(0.06f, -0.06f), size, vec3(0.10f, 0.06f, 0.03f));
        Renderer2D::DrawQuad(c, size, vec3(0.20f, 0.11f, 0.05f), highlight);
        Renderer2D::DrawQuad(c, size - vec2(0.12f), vec3(0.58f, 0.36f, 0.17f), highlight);
        // wood grain
        for (int i = 0; i < 3; ++i)
            Renderer2D::DrawQuad(c + vec2(0.0f, (i - 1) * size.y * 0.28f), vec2(size.x - 0.2f, 0.025f), vec3(0.50f, 0.30f, 0.14f), highlight);
        // nails
        for (int sx = -1; sx <= 1; sx += 2)
            for (int sy = -1; sy <= 1; sy += 2)
                Renderer2D::DrawCircle(c + vec2(sx * (size.x * 0.5f - 0.13f), sy * (size.y * 0.5f - 0.13f)), 0.035f, vec3(0.75f, 0.72f, 0.65f));
    }

    bool HandleInput()
    {
        pauseClicked = false;
        const vec2 m = Input::GetMouseWorldPos();
        bool used = false;

        if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            for (int i = 0; i < 3; ++i)
                if (InBox(m, CardCenter(i), CARD_SIZE))
                {
                    if (CardUnlocked(i))   // locked cards do not respond at all
                        UseCard(i);
                    used = true;
                }
            if (InBox(m, PAUSE_POS, PAUSE_SIZE))
            {
                pauseClicked = true;
                used = true;
            }
        }

        const int keys[3] = { GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3 };
        for (int i = 0; i < 3; ++i)
            if (Input::IsKeyPressed(keys[i]) && CardUnlocked(i))
                UseCard(i);

        return used;
    }

    bool PauseClicked() { return pauseClicked; }

    void Update(float dt)
    {
        uiTime += dt;
        toastTime = std::max(0.0f, toastTime - dt);
        for (float& p : cardPress) p = std::max(0.0f, p - dt * 5.0f);
        for (Spark& s : sparks) s.t += dt;
        for (FloatText& f : floaters) { f.t += dt; f.pos.y += dt * 0.6f; }
        sparks.erase(std::remove_if(sparks.begin(), sparks.end(), [](const Spark& s) { return s.t > 0.35f; }), sparks.end());
        floaters.erase(std::remove_if(floaters.begin(), floaters.end(), [](const FloatText& f) { return f.t > 1.0f; }), floaters.end());
    }

    void ShowToast(const std::string& text) { toast = text; toastTime = 1.6f; }
    void AddSpark(vec2 pos, vec3 color) { sparks.push_back({ pos, color, 0.0f }); }
    void AddFloatText(vec2 pos, const std::string& text, vec3 color) { floaters.push_back({ pos, text, color, 0.0f }); }

    void RenderEffects()
    {
        for (const Spark& s : sparks)
        {
            float k = s.t / 0.35f;
            for (int i = 0; i < 8; ++i)
            {
                float a = i * 0.785f + 0.3f;
                vec2 d(std::cos(a), std::sin(a));
                Renderer2D::DrawQuadRotated(s.pos + d * (0.08f + 0.3f * k), vec2(0.16f * (1.0f - k), 0.05f * (1.0f - k)), a, s.color);
            }
        }
        for (const FloatText& f : floaters)
            Renderer2D::DrawTextShadow(f.text, f.pos, 0.12f, f.color);
    }

    void Render()
    {
        const vec2 m = Input::GetMouseWorldPos();

        // ---- player cards (top left) ----
        for (int i = 0; i < 3; ++i)
            DrawCard(i, InBox(m, CardCenter(i), CARD_SIZE));

        // ---- wave counter (top middle) ----
        int wave = std::min(WaveManager::GetCurrentWave(), NUM_WAVES);
        int left = WaveManager::GetEnemiesLeftToSpawn() + EnemySystem::GetEnemyCount();
        DrawPanel(vec2(8.3f, 8.3f), vec2(3.6f, 1.0f));
        Renderer2D::DrawTextShadow("WAVE " + std::to_string(wave) + " / " + std::to_string(NUM_WAVES), vec2(8.3f, 8.47f), 0.15f, vec3(1.0f, 0.93f, 0.7f));
        Renderer2D::DrawTextShadow("THUGS LEFT: " + std::to_string(left), vec2(8.3f, 8.05f), 0.105f, vec3(1.0f, 0.88f, 0.6f));

        // ---- coins (top right) ----
        DrawPanel(vec2(13.55f, 8.3f), vec2(2.8f, 1.0f));
        Economy::DrawCoin(vec2(12.6f, 8.3f), 0.3f, std::cos(uiTime * 1.5f));
        Renderer2D::DrawTextShadow("POUNDS", vec2(13.75f, 8.58f), 0.105f, vec3(1.0f, 0.88f, 0.6f));
        Renderer2D::DrawTextShadow(std::to_string(Economy::GetCoins()), vec2(14.75f, 8.17f), 0.2f, vec3(1.0f, 0.86f, 0.3f), Renderer2D::Align::Right);

        // ---- pause button ----
        bool ph = InBox(m, PAUSE_POS, PAUSE_SIZE);
        DrawPanel(PAUSE_POS, PAUSE_SIZE, ph ? 0.3f : 0.0f);
        Renderer2D::DrawQuad(PAUSE_POS + vec2(-0.1f, 0.0f), vec2(0.1f, 0.34f), vec3(1.0f, 0.95f, 0.8f));
        Renderer2D::DrawQuad(PAUSE_POS + vec2(0.1f, 0.0f), vec2(0.1f, 0.34f), vec3(1.0f, 0.95f, 0.8f));

        // ---- break between waves: big banner ----
        float br = WaveManager::GetBreakTime();
        if (br > 0.0f && WaveManager::GetCurrentWave() <= NUM_WAVES)
        {
            float pulse = 1.0f + 0.05f * std::sin(uiTime * 8.0f);
            DrawPanel(vec2(8.8f, 4.0f), vec2(6.4f, 1.9f));
            Renderer2D::DrawTextShadow("WAVE " + std::to_string(WaveManager::GetCurrentWave()), vec2(8.8f, 4.35f), 0.32f * pulse, vec3(1.0f, 0.85f, 0.3f));
            const char* sub[3] = { "THE SLIPPER GANG IS COMING!", "THE KIDS BROUGHT SLINGSHOTS!", "THE KNIFE BOSS IS HERE!" };
            Renderer2D::DrawTextShadow(sub[std::min(2, WaveManager::GetCurrentWave() - 1)], vec2(8.8f, 3.5f), 0.1f, vec3(1.0f, 0.95f, 0.85f));
        }

        // ---- toast message ----
        if (toastTime > 0.0f)
        {
            float w = Renderer2D::TextWidth(toast, 0.11f) + 0.6f;
            DrawPanel(vec2(8.8f, 6.45f), vec2(w, 0.55f));
            Renderer2D::DrawTextShadow(toast, vec2(8.8f, 6.45f), 0.11f, vec3(1.0f, 0.95f, 0.85f));
        }

        // ---- controls hint along the bottom ----
        Renderer2D::DrawQuad(vec2(8.8f, 0.2f), vec2(Renderer2D::TextWidth("CLICK LANES TO SELECT - CARD OR 1/2/3: PLACE - W/S: JUMP/DUCK - ESC: PAUSE", 0.105f) + 0.4f, 0.32f), vec3(0.16f, 0.13f, 0.11f));
        Renderer2D::DrawTextShadow("CLICK LANES TO SELECT - CARD OR 1/2/3: PLACE - W/S: JUMP/DUCK - ESC: PAUSE",
                                   vec2(8.8f, 0.2f), 0.105f, vec3(1.0f, 0.95f, 0.85f));
    }
}
