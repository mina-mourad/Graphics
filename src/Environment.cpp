#include "Draw.h"
#include "Renderer2D.h"

#include <cmath>
#include <cstdint>

// The world of "Guardians of the Sun Disc": a desert valley at dusk with the
// Sun Temple (home base) at the left end of four sandstone lanes.
//
// Environmental elements (spec 4.2 asks for at least five): pyramids, a setting
// sun, palm trees, obelisks, sand dunes, stone pillars, braziers with flickering
// flames, cacti, rocks and the carved paving of the lanes.

namespace
{
    using glm::vec2;
    using glm::vec3;

    const float PI = 3.14159265358979f;

    // Deterministic pseudo-random number in [0,1) so decoration never jitters.
    float Hash01(int n)
    {
        uint32_t x = static_cast<uint32_t>(n) * 747796405u + 2891336453u;
        x = ((x >> ((x >> 28u) + 4u)) ^ x) * 277803737u;
        x = (x >> 22u) ^ x;
        return static_cast<float>(x & 0xFFFFFFu) / 16777216.0f;
    }

    void Pyramid(float cx, float by, float w, float h, vec3 light, vec3 dark)
    {
        Renderer2D::DrawTriangle(vec2(cx - w * 0.5f, by), vec2(cx, by + h), vec2(cx - w * 0.06f, by), light);
        Renderer2D::DrawTriangle(vec2(cx - w * 0.06f, by), vec2(cx, by + h), vec2(cx + w * 0.5f, by), dark);
    }

    void Palm(float x, float by, float h, float lean, float time, float seed)
    {
        vec3 trunk(0.38f, 0.25f, 0.13f), leaf(0.16f, 0.36f, 0.14f), leafL(0.24f, 0.48f, 0.18f);
        // slightly curved trunk made of segments
        vec2 prev(x, by);
        int segs = 5;
        for (int i = 1; i <= segs; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(segs);
            vec2 pt(x + lean * t * t * h, by + h * t);
            Draw::DrawLine(prev, pt, 0.16f - 0.06f * t, trunk);
            prev = pt;
        }
        vec2 top = prev;
        // fronds fan out from the crown and sway gently
        for (int i = 0; i < 7; ++i) {
            float a = PI * (0.08f + 0.84f * static_cast<float>(i) / 6.0f);
            float sway = std::sin(time * 0.9f + seed + static_cast<float>(i)) * 0.05f;
            vec2 dir(std::cos(a + sway), std::sin(a + sway));
            vec2 n(-dir.y, dir.x);
            float len = 0.95f;
            vec2 tip = top + dir * len + vec2(0.0f, -0.28f * std::fabs(dir.x));
            vec2 mid = top + dir * (len * 0.55f);
            Renderer2D::DrawTriangle(top, mid + n * 0.17f, tip, (i % 2) ? leaf : leafL);
            Renderer2D::DrawTriangle(top, mid - n * 0.17f, tip, (i % 2) ? leafL : leaf);
        }
        Renderer2D::DrawEllipse(top + vec2(0.0f, -0.05f), vec2(0.10f), 0.0f, vec3(0.30f, 0.18f, 0.08f));
        Renderer2D::DrawEllipse(top + vec2(-0.10f, -0.12f), vec2(0.07f), 0.0f, vec3(0.30f, 0.18f, 0.08f));
    }

    void Obelisk(float x, float by, float h)
    {
        float w = 0.24f;
        Renderer2D::DrawQuadRot(vec2(x, by + h * 0.5f), vec2(w, h), 0.0f, vec3(0.72f, 0.60f, 0.45f));
        Renderer2D::DrawQuadRot(vec2(x + w * 0.25f, by + h * 0.5f), vec2(w * 0.5f, h), 0.0f, vec3(0.62f, 0.50f, 0.36f));
        Renderer2D::DrawTriangle(vec2(x - w * 0.5f, by + h), vec2(x + w * 0.5f, by + h), vec2(x, by + h + 0.2f), vec3(0.95f, 0.76f, 0.22f));
        for (int i = 0; i < 4; ++i)
            Renderer2D::DrawQuadRot(vec2(x, by + h * (0.2f + 0.18f * i)), vec2(0.07f, 0.045f), 0.0f, vec3(0.38f, 0.28f, 0.18f));
        Renderer2D::DrawQuadRot(vec2(x, by + 0.05f), vec2(w + 0.12f, 0.10f), 0.0f, vec3(0.55f, 0.45f, 0.33f));
    }

    void Brazier(float x, float by, float time, float seed)
    {
        vec3 bronze(0.45f, 0.30f, 0.12f);
        Renderer2D::DrawEllipse(vec2(x, by), vec2(0.20f, 0.05f), 0.0f, vec3(0.1f, 0.06f, 0.03f), 0.4f);
        Renderer2D::DrawQuadRot(vec2(x, by + 0.25f), vec2(0.07f, 0.5f), 0.0f, bronze);
        Renderer2D::DrawQuadRot(vec2(x, by + 0.04f), vec2(0.22f, 0.06f), 0.0f, bronze);
        Renderer2D::DrawTriangle(vec2(x - 0.17f, by + 0.58f), vec2(x + 0.17f, by + 0.58f), vec2(x, by + 0.46f), vec3(0.62f, 0.42f, 0.14f));
        Renderer2D::DrawQuadRot(vec2(x, by + 0.58f), vec2(0.36f, 0.04f), 0.0f, vec3(0.85f, 0.62f, 0.20f));
        float f1 = std::sin(time * 11.0f + seed), f2 = std::sin(time * 17.0f + seed * 2.0f);
        Renderer2D::DrawEllipse(vec2(x, by + 0.78f), vec2(0.40f, 0.40f), 0.0f, vec3(1.0f, 0.55f, 0.10f), 0.14f);
        Renderer2D::DrawTriangle(vec2(x - 0.12f, by + 0.60f), vec2(x + 0.12f, by + 0.60f), vec2(x + 0.03f * f1, by + 0.90f + 0.05f * f2), vec3(0.95f, 0.35f, 0.05f));
        Renderer2D::DrawTriangle(vec2(x - 0.08f, by + 0.60f), vec2(x + 0.08f, by + 0.60f), vec2(x - 0.02f * f2, by + 0.80f + 0.04f * f1), vec3(1.0f, 0.70f, 0.12f));
        Renderer2D::DrawTriangle(vec2(x - 0.04f, by + 0.60f), vec2(x + 0.04f, by + 0.60f), vec2(x, by + 0.70f), vec3(1.0f, 0.95f, 0.55f));
    }

    void Cactus(float x, float by, float h)
    {
        vec3 g1(0.20f, 0.45f, 0.20f), g2(0.30f, 0.58f, 0.26f);
        Renderer2D::DrawQuadRot(vec2(x, by + h * 0.5f), vec2(0.17f, h), 0.0f, g1);
        Renderer2D::DrawEllipse(vec2(x, by + h), vec2(0.085f, 0.085f), 0.0f, g1);
        Renderer2D::DrawQuadRot(vec2(x - 0.02f, by + h * 0.5f), vec2(0.05f, h * 0.9f), 0.0f, g2);
        Renderer2D::DrawQuadRot(vec2(x - 0.17f, by + h * 0.55f), vec2(0.20f, 0.09f), 0.0f, g1);
        Renderer2D::DrawQuadRot(vec2(x - 0.25f, by + h * 0.72f), vec2(0.09f, 0.28f), 0.0f, g1);
        Renderer2D::DrawQuadRot(vec2(x + 0.15f, by + h * 0.40f), vec2(0.18f, 0.09f), 0.0f, g1);
        Renderer2D::DrawQuadRot(vec2(x + 0.22f, by + h * 0.55f), vec2(0.09f, 0.25f), 0.0f, g1);
        Renderer2D::DrawEllipse(vec2(x + 0.05f, by + h * 0.8f), vec2(0.018f), 0.0f, vec3(0.95f, 0.45f, 0.65f));
    }

    void Rock(float x, float by, float s)
    {
        Renderer2D::DrawEllipse(vec2(x, by - 0.02f), vec2(0.34f * s, 0.07f * s), 0.0f, vec3(0.1f, 0.06f, 0.03f), 0.35f);
        Renderer2D::DrawTriangle(vec2(x - 0.30f * s, by), vec2(x - 0.06f * s, by + 0.30f * s), vec2(x + 0.08f * s, by), vec3(0.50f, 0.44f, 0.38f));
        Renderer2D::DrawTriangle(vec2(x - 0.06f * s, by + 0.30f * s), vec2(x + 0.30f * s, by), vec2(x + 0.08f * s, by), vec3(0.38f, 0.33f, 0.29f));
        Renderer2D::DrawTriangle(vec2(x - 0.06f * s, by + 0.30f * s), vec2(x + 0.18f * s, by + 0.14f * s), vec2(x + 0.02f * s, by + 0.08f * s), vec3(0.62f, 0.56f, 0.48f));
    }

    void Pillar(float x, float by)
    {
        vec3 stone(0.74f, 0.66f, 0.52f), dark(0.50f, 0.42f, 0.30f);
        Renderer2D::DrawQuadRot(vec2(x, by + 0.05f), vec2(0.30f, 0.10f), 0.0f, dark);
        Renderer2D::DrawQuadRot(vec2(x, by + 0.28f), vec2(0.20f, 0.36f), 0.0f, stone);
        Renderer2D::DrawQuadRot(vec2(x + 0.06f, by + 0.28f), vec2(0.08f, 0.36f), 0.0f, vec3(0.62f, 0.54f, 0.42f));
        Renderer2D::DrawQuadRot(vec2(x, by + 0.49f), vec2(0.30f, 0.08f), 0.0f, vec3(0.88f, 0.72f, 0.24f));
        for (int i = 0; i < 3; ++i)
            Renderer2D::DrawQuadRot(vec2(x - 0.02f, by + 0.14f + 0.10f * i), vec2(0.07f, 0.035f), 0.0f, dark);
    }
}

namespace Draw
{
    void Sky(float time)
    {
        // whole screen starts as sand so nothing is ever left bare
        Renderer2D::DrawQuad(vec2(WORLD_WIDTH * 0.5f, WORLD_HEIGHT * 0.5f), vec2(WORLD_WIDTH, WORLD_HEIGHT), vec3(0.78f, 0.62f, 0.38f));

        // dusk gradient above the horizon
        const int bands = 14;
        float top = WORLD_HEIGHT, bottom = PLAYFIELD_TOP - 0.1f;
        for (int i = 0; i < bands; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(bands - 1);
            vec3 low(0.98f, 0.62f, 0.28f), mid(0.82f, 0.38f, 0.42f), high(0.22f, 0.16f, 0.38f);
            vec3 c = t < 0.5f ? glm::mix(low, mid, t * 2.0f) : glm::mix(mid, high, (t - 0.5f) * 2.0f);
            float y0 = bottom + (top - bottom) * static_cast<float>(i) / static_cast<float>(bands);
            float y1 = bottom + (top - bottom) * static_cast<float>(i + 1) / static_cast<float>(bands);
            Renderer2D::DrawQuad(vec2(WORLD_WIDTH * 0.5f, (y0 + y1) * 0.5f), vec2(WORLD_WIDTH, y1 - y0 + 0.01f), c);
        }

        // first stars
        for (int i = 0; i < 26; ++i) {
            float x = Hash01(i * 3 + 1) * WORLD_WIDTH;
            float y = 7.9f + Hash01(i * 3 + 2) * 1.1f;
            float tw = 0.5f + 0.5f * std::sin(time * 2.0f + static_cast<float>(i));
            Renderer2D::DrawEllipse(vec2(x, y), vec2(0.022f + 0.012f * tw), 0.0f, vec3(1.0f, 0.97f, 0.85f), 0.35f + 0.5f * tw);
        }

        // setting sun with glow rings
        vec2 sun(12.9f, 7.0f);
        for (int i = 4; i >= 1; --i)
            Renderer2D::DrawEllipse(sun, vec2(0.7f + 0.35f * i), 0.0f, vec3(1.0f, 0.78f, 0.35f), 0.10f);
        Renderer2D::DrawEllipse(sun, vec2(0.62f), 0.0f, vec3(1.0f, 0.93f, 0.55f));
        Renderer2D::DrawEllipse(sun, vec2(0.50f), 0.0f, vec3(1.0f, 0.98f, 0.78f));

        // far pyramids on the horizon
        Pyramid(9.6f, PLAYFIELD_TOP - 0.05f, 3.6f, 2.0f, vec3(0.80f, 0.55f, 0.42f), vec3(0.55f, 0.32f, 0.34f));
        Pyramid(12.1f, PLAYFIELD_TOP - 0.05f, 2.5f, 1.4f, vec3(0.86f, 0.60f, 0.42f), vec3(0.60f, 0.36f, 0.34f));
        Pyramid(7.6f, PLAYFIELD_TOP - 0.05f, 1.9f, 1.05f, vec3(0.76f, 0.52f, 0.42f), vec3(0.52f, 0.30f, 0.34f));
        Pyramid(14.8f, PLAYFIELD_TOP - 0.05f, 2.0f, 1.1f, vec3(0.78f, 0.53f, 0.42f), vec3(0.54f, 0.31f, 0.34f));

        // dunes along the horizon
        const vec3 dune(0.74f, 0.52f, 0.30f), duneL(0.86f, 0.64f, 0.38f);
        Renderer2D::DrawEllipse(vec2(4.2f, PLAYFIELD_TOP - 0.15f), vec2(3.6f, 0.62f), 0.0f, dune);
        Renderer2D::DrawEllipse(vec2(10.8f, PLAYFIELD_TOP - 0.22f), vec2(4.6f, 0.52f), 0.0f, duneL);
        Renderer2D::DrawEllipse(vec2(15.6f, PLAYFIELD_TOP - 0.15f), vec2(2.4f, 0.55f), 0.0f, dune);

        // obelisks and palms
        Obelisk(6.55f, PLAYFIELD_TOP - 0.02f, 1.0f);
        Obelisk(11.15f, PLAYFIELD_TOP - 0.02f, 0.8f);
        Palm(7.15f, PLAYFIELD_TOP - 0.04f, 1.35f, 0.10f, time, 1.0f);
        Palm(15.3f, PLAYFIELD_TOP - 0.04f, 1.25f, -0.12f, time, 3.0f);
        Palm(13.9f, PLAYFIELD_TOP - 0.04f, 0.95f, 0.08f, time, 5.0f);
    }

    void LaneFloor(int lane, float highlight, bool selected)
    {
        float y = LANE_Y[lane];
        vec3 base = (lane % 2 == 0) ? vec3(0.86f, 0.72f, 0.46f) : vec3(0.81f, 0.66f, 0.40f);
        Renderer2D::DrawQuad(vec2(WORLD_WIDTH * 0.5f, y), vec2(WORLD_WIDTH, LANE_HEIGHT), base, highlight);

        // paving joints and sand speckles (fixed positions, not random per frame)
        for (int k = 0; k < 10; ++k) {
            float x = HOME_BASE_RIGHT + 1.65f * static_cast<float>(k + 1);
            Renderer2D::DrawQuadRot(vec2(x, y), vec2(0.025f, LANE_HEIGHT - 0.1f), 0.0f, vec3(0.45f, 0.32f, 0.17f), 0.14f);
        }
        for (int i = 0; i < 22; ++i) {
            int seed = lane * 100 + i;
            float x = HOME_BASE_RIGHT + Hash01(seed * 2) * (WORLD_WIDTH - HOME_BASE_RIGHT);
            float yy = y - LANE_HEIGHT * 0.5f + 0.1f + Hash01(seed * 2 + 1) * (LANE_HEIGHT - 0.2f);
            bool light = (i % 2) == 0;
            Renderer2D::DrawEllipse(vec2(x, yy), vec2(0.035f + 0.02f * Hash01(seed + 7), 0.018f), 0.0f,
                                    light ? vec3(0.95f, 0.85f, 0.60f) : vec3(0.60f, 0.44f, 0.24f), 0.5f);
        }

        // border stones between lanes
        float bottom = y - LANE_HEIGHT * 0.5f, top = y + LANE_HEIGHT * 0.5f;
        vec3 edge(0.40f, 0.28f, 0.14f);
        Renderer2D::DrawQuadRot(vec2(WORLD_WIDTH * 0.5f, bottom), vec2(WORLD_WIDTH, 0.07f), 0.0f, edge);
        if (lane == NUM_LANES - 1)
            Renderer2D::DrawQuadRot(vec2(WORLD_WIDTH * 0.5f, top), vec2(WORLD_WIDTH, 0.10f), 0.0f, edge);

        if (selected) {
            vec3 gold(1.0f, 0.84f, 0.25f);
            Renderer2D::DrawQuadRot(vec2(WORLD_WIDTH * 0.5f, bottom + 0.05f), vec2(WORLD_WIDTH, 0.06f), 0.0f, gold);
            Renderer2D::DrawQuadRot(vec2(WORLD_WIDTH * 0.5f, top - 0.05f), vec2(WORLD_WIDTH, 0.06f), 0.0f, gold);
        }
    }

    void HomeBase(float time)
    {
        // The Sun Temple: a pale limestone wall with lapis banners, a golden
        // sun disc, and an arched gate on every lane. It is the only pale,
        // blue-and-gold structure in the scene, so it stands out from the sand.
        float cx = HOME_BASE_RIGHT * 0.5f;
        float cy = (PLAYFIELD_BOTTOM + PLAYFIELD_TOP) * 0.5f;
        float h = PLAYFIELD_TOP - PLAYFIELD_BOTTOM;
        vec3 lime(0.93f, 0.91f, 0.84f), limeD(0.74f, 0.71f, 0.62f), gold(0.97f, 0.78f, 0.22f), lapis(0.10f, 0.24f, 0.66f);

        Renderer2D::DrawQuadRot(vec2(cx + 0.07f, cy - 0.05f), vec2(HOME_BASE_RIGHT, h + 0.2f), 0.0f, vec3(0.1f, 0.06f, 0.03f), 0.35f);
        Renderer2D::DrawQuadRot(vec2(cx, cy), vec2(HOME_BASE_RIGHT, h + 0.2f), 0.0f, lime);
        Renderer2D::DrawQuadRot(vec2(HOME_BASE_RIGHT - 0.10f, cy), vec2(0.20f, h + 0.2f), 0.0f, limeD);
        Renderer2D::DrawQuadRot(vec2(HOME_BASE_RIGHT - 0.02f, cy), vec2(0.04f, h + 0.2f), 0.0f, gold);

        // top and bottom cornices
        Renderer2D::DrawQuadRot(vec2(cx, PLAYFIELD_TOP + 0.08f), vec2(HOME_BASE_RIGHT + 0.1f, 0.14f), 0.0f, gold);
        Renderer2D::DrawQuadRot(vec2(cx, PLAYFIELD_BOTTOM - 0.06f), vec2(HOME_BASE_RIGHT + 0.1f, 0.14f), 0.0f, gold);
        Renderer2D::DrawTriangle(vec2(0.0f, PLAYFIELD_TOP + 0.15f), vec2(HOME_BASE_RIGHT + 0.05f, PLAYFIELD_TOP + 0.15f), vec2(HOME_BASE_RIGHT * 0.5f, PLAYFIELD_TOP + 0.50f), limeD);

        // an arched gate on every lane
        for (int i = 0; i < NUM_LANES; ++i) {
            float y = LANE_Y[i];
            Renderer2D::DrawQuadRot(vec2(0.62f, y - 0.05f), vec2(0.66f, 0.95f), 0.0f, vec3(0.30f, 0.20f, 0.10f));
            Renderer2D::DrawEllipse(vec2(0.62f, y + 0.40f), vec2(0.33f, 0.20f), 0.0f, vec3(0.30f, 0.20f, 0.10f));
            Renderer2D::DrawQuadRot(vec2(0.62f, y - 0.05f), vec2(0.50f, 0.88f), 0.0f, vec3(0.12f, 0.08f, 0.05f));
            Renderer2D::DrawEllipse(vec2(0.62f, y + 0.39f), vec2(0.25f, 0.17f), 0.0f, vec3(0.12f, 0.08f, 0.05f));
            float glow = 0.5f + 0.5f * std::sin(time * 2.0f + static_cast<float>(i));
            Renderer2D::DrawEllipse(vec2(0.62f, y + 0.02f), vec2(0.24f, 0.34f), 0.0f, vec3(1.0f, 0.72f, 0.25f), 0.12f + 0.12f * glow);
            Renderer2D::DrawQuadRot(vec2(0.62f, y - 0.52f), vec2(0.74f, 0.05f), 0.0f, gold);
        }

        // lapis banners on the pillars between gates, with a gold sun disc on each
        for (int i = 1; i < NUM_LANES; ++i) {
            float by = PLAYFIELD_BOTTOM + LANE_HEIGHT * static_cast<float>(i);
            Renderer2D::DrawQuadRot(vec2(0.62f, by), vec2(0.34f, 0.46f), 0.0f, lapis);
            Renderer2D::DrawTriangle(vec2(0.45f, by - 0.23f), vec2(0.79f, by - 0.23f), vec2(0.62f, by - 0.40f), lapis);
            Renderer2D::DrawEllipse(vec2(0.62f, by + 0.04f), vec2(0.10f), 0.0f, gold);
            Renderer2D::DrawEllipse(vec2(0.62f, by + 0.04f), vec2(0.06f), 0.0f, vec3(1.0f, 0.93f, 0.55f));
        }

        // big glowing sun disc crowning the temple in the middle, with rays
        vec2 disc(0.62f, cy);
        float pulse = 0.5f + 0.5f * std::sin(time * 2.2f);
        Renderer2D::DrawEllipse(disc, vec2(0.75f + 0.05f * pulse), 0.0f, vec3(1.0f, 0.85f, 0.35f), 0.12f);
        for (int i = 0; i < 12; ++i) {
            float a = 2.0f * PI * static_cast<float>(i) / 12.0f + time * 0.25f;
            vec2 d(std::cos(a), std::sin(a)), n(-d.y, d.x);
            Renderer2D::DrawTriangle(disc + d * 0.34f + n * 0.04f, disc + d * 0.34f - n * 0.04f, disc + d * 0.54f, gold, 0.9f);
        }
        Renderer2D::DrawEllipse(disc, vec2(0.30f), 0.0f, vec3(0.75f, 0.48f, 0.08f));
        Renderer2D::DrawEllipse(disc, vec2(0.26f), 0.0f, gold);
        Renderer2D::DrawEllipse(disc, vec2(0.17f), 0.0f, vec3(1.0f, 0.93f, 0.55f));
        // wings of the sun disc
        Renderer2D::DrawTriangle(disc + vec2(-0.28f, 0.0f), disc + vec2(-0.60f, 0.10f), disc + vec2(-0.55f, -0.08f), lapis);
        Renderer2D::DrawTriangle(disc + vec2(0.28f, 0.0f), disc + vec2(0.60f, 0.10f), disc + vec2(0.55f, -0.08f), lapis);
    }

    void Foreground(float time)
    {
        // sandy strip beneath the lanes
        float stripH = PLAYFIELD_BOTTOM;
        Renderer2D::DrawQuad(vec2(WORLD_WIDTH * 0.5f, stripH * 0.5f), vec2(WORLD_WIDTH, stripH), vec3(0.66f, 0.50f, 0.28f));
        Renderer2D::DrawEllipse(vec2(3.0f, 0.12f), vec2(3.4f, 0.34f), 0.0f, vec3(0.74f, 0.57f, 0.32f));
        Renderer2D::DrawEllipse(vec2(10.5f, 0.10f), vec2(4.6f, 0.30f), 0.0f, vec3(0.70f, 0.53f, 0.30f));
        Renderer2D::DrawEllipse(vec2(15.5f, 0.14f), vec2(2.2f, 0.34f), 0.0f, vec3(0.76f, 0.58f, 0.33f));
        Renderer2D::DrawQuadRot(vec2(WORLD_WIDTH * 0.5f, stripH), vec2(WORLD_WIDTH, 0.10f), 0.0f, vec3(0.40f, 0.28f, 0.14f));

        Cactus(5.4f, 0.12f, 0.55f);
        Cactus(13.0f, 0.10f, 0.45f);
        Rock(3.2f, 0.12f, 1.0f);
        Rock(8.6f, 0.08f, 0.8f);
        Rock(15.2f, 0.10f, 1.1f);
        Brazier(2.4f, 0.0f, time, 0.0f);
        Brazier(7.2f, 0.0f, time, 2.0f);
        Brazier(10.9f, 0.0f, time, 4.0f);
        Brazier(15.6f, 0.0f, time, 6.0f);

        // carved stone pillars along the far border of the playfield
        for (int i = 0; i < 5; ++i)
            Pillar(3.6f + 3.0f * static_cast<float>(i), PLAYFIELD_TOP + 0.04f);
    }
}
