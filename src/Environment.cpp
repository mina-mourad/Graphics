#include "Environment.h"
#include "GameConfig.h"
#include "Renderer2D.h"

#include <cmath>
#include <vector>

namespace
{
    using glm::vec2;
    using glm::vec3;

    // ------------------------------------------------------------------
    // Small helpers
    // ------------------------------------------------------------------

    // Rectangle from its bottom-left (x0, y0) to its top-right (x1, y1).
    void Rect(float x0, float y0, float x1, float y1, vec3 c)
    {
        Renderer2D::DrawQuad(vec2((x0 + x1) * 0.5f, (y0 + y1) * 0.5f), vec2(x1 - x0, y1 - y0), c);
    }

    void Tri(vec2 a, vec2 b, vec2 c, vec3 color) { Renderer2D::DrawTriangle(a, b, c, color); }

    // Fixed "random" number in 0..1 for an integer seed, so the street looks
    // the same every frame.
    float Hash(int n)
    {
        float s = std::sin(n * 12.9898f + 78.233f) * 43758.5453f;
        return s - std::floor(s);
    }

    // A sagging cable between two points, as a chain of short lines.
    void Cable(vec2 a, vec2 b, float sag, float width, vec3 color)
    {
        const int steps = 14;
        vec2 prev = a;
        for (int i = 1; i <= steps; ++i)
        {
            float t = (float)i / steps;
            vec2 p = glm::mix(a, b, t);
            p.y -= sag * 4.0f * t * (1.0f - t);
            Renderer2D::DrawLine(prev, p, width, color);
            prev = p;
        }
    }

    // ------------------------------------------------------------------
    // Colors
    // ------------------------------------------------------------------
    const vec3 SKY_TOP   (0.62f, 0.74f, 0.82f);
    const vec3 SKY_LOW   (0.93f, 0.86f, 0.70f);
    const vec3 BRICK     (0.66f, 0.33f, 0.22f);
    const vec3 BRICK_DARK(0.50f, 0.24f, 0.16f);
    const vec3 MORTAR    (0.78f, 0.62f, 0.48f);
    const vec3 CONCRETE  (0.86f, 0.78f, 0.64f);
    const vec3 PEACH     (0.90f, 0.70f, 0.52f);
    const vec3 SAND      (0.86f, 0.70f, 0.47f);
    const vec3 SAND_DARK (0.74f, 0.58f, 0.37f);
    const vec3 STONE     (0.66f, 0.63f, 0.58f);
    const vec3 STONE_DARK(0.46f, 0.44f, 0.41f);
    const vec3 CABLE     (0.12f, 0.10f, 0.10f);
    const vec3 TIRE      (0.14f, 0.13f, 0.13f);
    const vec3 WOOD      (0.62f, 0.42f, 0.22f);
    const vec3 WOOD_DARK (0.44f, 0.29f, 0.15f);
    const vec3 GLASS     (0.30f, 0.42f, 0.48f);

    // ------------------------------------------------------------------
    // Props
    // ------------------------------------------------------------------
    void SatelliteDish(vec2 base, float s, float facing)
    {
        Renderer2D::DrawLine(base, base + vec2(0.0f, 0.22f * s), 0.04f * s, vec3(0.5f));
        vec2 c = base + vec2(0.06f * facing * s, 0.36f * s);
        Renderer2D::DrawEllipse(c, vec2(0.10f * s, 0.20f * s), vec3(0.55f, 0.56f, 0.58f));
        Renderer2D::DrawEllipse(c + vec2(0.02f * facing * s, 0.0f), vec2(0.08f * s, 0.17f * s), vec3(0.86f, 0.87f, 0.88f));
        Renderer2D::DrawLine(c, c + vec2(0.22f * facing * s, 0.0f), 0.025f * s, vec3(0.35f));
        Renderer2D::DrawCircle(c + vec2(0.22f * facing * s, 0.0f), 0.03f * s, vec3(0.3f));
    }

    void Tire(vec2 c, float r)
    {
        Renderer2D::DrawCircle(c, r, TIRE);
        Renderer2D::DrawCircle(c, r * 0.55f, vec3(0.30f, 0.28f, 0.26f));
        Renderer2D::DrawCircle(c, r * 0.35f, TIRE);
        for (int i = 0; i < 8; ++i)
        {
            float a = i * 0.785f;
            Renderer2D::DrawQuadRotated(c + vec2(std::cos(a), std::sin(a)) * r * 0.82f, vec2(r * 0.18f, r * 0.08f), a, vec3(0.22f));
        }
    }

    // Tire lying flat (seen from the side): a flattened ellipse ring.
    void FlatTire(vec2 c, float w)
    {
        Renderer2D::DrawEllipse(c, vec2(w, w * 0.32f), TIRE);
        Renderer2D::DrawEllipse(c + vec2(0.0f, w * 0.05f), vec2(w * 0.55f, w * 0.14f), vec3(0.06f));
    }

    void Crate(vec2 bl, float w, float h)
    {
        Rect(bl.x, bl.y, bl.x + w, bl.y + h, WOOD_DARK);
        Rect(bl.x + 0.03f, bl.y + 0.03f, bl.x + w - 0.03f, bl.y + h - 0.03f, WOOD);
        for (int i = 1; i < 3; ++i)
            Rect(bl.x + 0.03f, bl.y + h * i / 3.0f - 0.012f, bl.x + w - 0.03f, bl.y + h * i / 3.0f + 0.012f, WOOD_DARK);
        Renderer2D::DrawLine(bl + vec2(0.05f, 0.05f), bl + vec2(w - 0.05f, h - 0.05f), 0.03f, WOOD_DARK);
    }

    // A loose pile of bricks and broken concrete.
    void Rubble(vec2 c, float s, int seed)
    {
        Renderer2D::DrawEllipse(c + vec2(0.0f, -0.02f), vec2(0.45f * s, 0.10f * s), SAND_DARK);
        for (int i = 0; i < 9; ++i)
        {
            float x = (Hash(seed + i) - 0.5f) * 0.8f * s;
            float y = (Hash(seed + i * 7) * 0.18f) * s * (1.0f - std::fabs(x) / s);
            float a = (Hash(seed + i * 3) - 0.5f) * 1.2f;
            bool brick = Hash(seed + i * 5) > 0.35f;
            vec3 col = brick ? glm::mix(BRICK, BRICK_DARK, Hash(seed + i * 11)) : glm::mix(STONE, CONCRETE, Hash(seed + i * 13));
            Renderer2D::DrawQuadRotated(c + vec2(x, y), vec2(0.22f, 0.10f) * s, a, col);
        }
    }

    // A roll-up metal garage shutter (like in the reference picture).
    void Shutter(float x0, float y0, float x1, float y1)
    {
        Rect(x0 - 0.05f, y0, x1 + 0.05f, y1 + 0.08f, vec3(0.35f, 0.33f, 0.32f));
        Rect(x0, y0, x1, y1, vec3(0.60f, 0.62f, 0.64f));
        for (float y = y0 + 0.06f; y < y1; y += 0.09f)
            Rect(x0, y, x1, y + 0.025f, vec3(0.48f, 0.50f, 0.53f));
        Rect(x0 + (x1 - x0) * 0.45f, y0 + 0.05f, x0 + (x1 - x0) * 0.55f, y0 + 0.1f, vec3(0.25f));
        // rust streak
        Rect(x0 + 0.1f, y0, x0 + 0.16f, y0 + (y1 - y0) * 0.4f, vec3(0.58f, 0.36f, 0.22f));
    }

    // A window with wooden shutters, sometimes a balcony with laundry.
    void Window(vec2 c, float w, float h, int seed, bool balcony, float time)
    {
        vec3 frame(0.92f, 0.88f, 0.80f);
        vec3 shutterCol = Hash(seed) > 0.5f ? vec3(0.32f, 0.48f, 0.42f) : vec3(0.52f, 0.36f, 0.22f);
        Rect(c.x - w * 0.5f - 0.04f, c.y - h * 0.5f - 0.04f, c.x + w * 0.5f + 0.04f, c.y + h * 0.5f + 0.04f, frame);
        Rect(c.x - w * 0.5f, c.y - h * 0.5f, c.x + w * 0.5f, c.y + h * 0.5f, Hash(seed + 3) > 0.7f ? vec3(0.95f, 0.85f, 0.55f) : GLASS);
        Rect(c.x - 0.01f, c.y - h * 0.5f, c.x + 0.01f, c.y + h * 0.5f, frame);
        if (Hash(seed + 1) > 0.4f)
        {
            // one shutter open, hanging at the side
            Rect(c.x - w * 0.5f - w * 0.42f, c.y - h * 0.5f, c.x - w * 0.5f - 0.02f, c.y + h * 0.5f, shutterCol);
            for (int i = 0; i < 4; ++i)
                Rect(c.x - w * 0.5f - w * 0.40f, c.y - h * 0.4f + i * h * 0.22f, c.x - w * 0.5f - 0.04f, c.y - h * 0.4f + i * h * 0.22f + 0.02f, shutterCol * 0.75f);
        }
        if (balcony)
        {
            float by = c.y - h * 0.5f - 0.05f;
            Rect(c.x - w * 0.8f, by - 0.06f, c.x + w * 0.8f, by, vec3(0.70f, 0.66f, 0.60f));
            for (float x = c.x - w * 0.75f; x <= c.x + w * 0.76f; x += 0.08f)
                Rect(x, by, x + 0.02f, by + 0.26f, vec3(0.22f, 0.20f, 0.20f));
            Rect(c.x - w * 0.8f, by + 0.25f, c.x + w * 0.8f, by + 0.28f, vec3(0.22f, 0.20f, 0.20f));

            // laundry line with clothes moving in the wind
            if (Hash(seed + 2) > 0.35f)
            {
                float ly = by + 0.48f;
                Renderer2D::DrawLine(vec2(c.x - w * 0.8f, ly), vec2(c.x + w * 0.8f, ly), 0.015f, vec3(0.2f));
                const vec3 cloth[4] = { vec3(0.85f, 0.25f, 0.25f), vec3(0.25f, 0.45f, 0.80f), vec3(0.95f, 0.92f, 0.85f), vec3(0.95f, 0.75f, 0.25f) };
                for (int i = 0; i < 3; ++i)
                {
                    float x = c.x - w * 0.55f + i * w * 0.5f;
                    float sway = 0.03f * std::sin(time * 2.2f + i + seed);
                    vec3 col = cloth[(seed + i) % 4];
                    Renderer2D::DrawPolygon(std::vector<vec2>{ vec2(x - 0.08f, ly), vec2(x + 0.08f, ly),
                        vec2(x + 0.07f + sway, ly - 0.20f), vec2(x - 0.07f + sway, ly - 0.20f) }.data(), 4, col);
                }
            }
        }
    }

    // One house of the street. x0..x1 wide, from the street up to `top`.
    void House(float x0, float x1, float top, int seed, float time)
    {
        const float base = PLAYFIELD_TOP - 0.05f;
        const int style = static_cast<int>(Hash(seed) * 3.0f);   // 0 brick, 1 concrete, 2 peach plaster
        const vec3 wall = style == 0 ? BRICK : (style == 1 ? CONCRETE : PEACH);

        Rect(x0, base, x1, top, wall);

        if (style == 0)
        {
            // brick courses with mortar lines and the concrete frame
            for (float y = base + 0.1f; y < top; y += 0.12f)
                Rect(x0, y, x1, y + 0.018f, MORTAR);
            int row = 0;
            for (float y = base + 0.1f; y < top - 0.1f; y += 0.12f, ++row)
                for (float x = x0 + (row % 2) * 0.12f; x < x1; x += 0.24f)
                    Rect(x, y, x + 0.015f, y + 0.12f, MORTAR);
            Rect(x0, base, x0 + 0.12f, top, CONCRETE * 0.92f);
            Rect(x1 - 0.12f, base, x1, top, CONCRETE * 0.92f);
            for (float y = base + 1.15f; y < top; y += 1.15f)
                Rect(x0, y, x1, y + 0.1f, CONCRETE * 0.92f);
        }
        else
        {
            // stains and cracked plaster
            Rect(x0, base, x1, base + 0.35f, wall * 0.85f);
            Renderer2D::DrawEllipse(vec2(x0 + (x1 - x0) * Hash(seed + 4), base + 1.0f), vec2(0.25f, 0.4f), wall * 0.9f);
            Renderer2D::DrawLine(vec2(x0 + 0.3f, top - 0.2f), vec2(x0 + 0.45f, top - 0.6f), 0.015f, wall * 0.6f);
        }

        // rooftop: unfinished concrete columns with rebar sticking out
        Rect(x0 - 0.04f, top, x1 + 0.04f, top + 0.08f, CONCRETE * 0.85f);
        if (Hash(seed + 9) > 0.3f)
        {
            Rect(x0 + 0.1f, top + 0.08f, x0 + 0.25f, top + 0.42f, CONCRETE * 0.8f);
            Rect(x1 - 0.25f, top + 0.08f, x1 - 0.1f, top + 0.32f, CONCRETE * 0.8f);
            for (int i = 0; i < 3; ++i)
            {
                Renderer2D::DrawLine(vec2(x0 + 0.13f + i * 0.05f, top + 0.42f), vec2(x0 + 0.1f + i * 0.06f, top + 0.68f), 0.012f, vec3(0.35f, 0.25f, 0.2f));
                Renderer2D::DrawLine(vec2(x1 - 0.22f + i * 0.05f, top + 0.32f), vec2(x1 - 0.2f + i * 0.04f, top + 0.52f), 0.012f, vec3(0.35f, 0.25f, 0.2f));
            }
        }
        // water tank
        if (Hash(seed + 10) > 0.5f)
        {
            float tx = x0 + (x1 - x0) * 0.6f;
            Rect(tx - 0.2f, top + 0.08f, tx + 0.2f, top + 0.42f, vec3(0.20f, 0.22f, 0.25f));
            Renderer2D::DrawEllipse(vec2(tx, top + 0.42f), vec2(0.2f, 0.05f), vec3(0.30f, 0.32f, 0.36f));
        }

        // windows, two per floor
        const float floorH = 1.15f;
        int i = 0;
        for (float y = base + floorH + 0.55f; y < top - 0.35f; y += floorH, ++i)
        {
            float w = 0.34f;
            Window(vec2(x0 + (x1 - x0) * 0.3f, y), w, 0.48f, seed * 7 + i * 2, Hash(seed * 3 + i) > 0.45f, time);
            Window(vec2(x0 + (x1 - x0) * 0.72f, y), w, 0.48f, seed * 7 + i * 2 + 1, false, time);
            // AC box under a window
            if (Hash(seed * 5 + i) > 0.6f)
            {
                vec2 ac(x0 + (x1 - x0) * 0.72f, y - 0.42f);
                Rect(ac.x - 0.16f, ac.y - 0.1f, ac.x + 0.16f, ac.y + 0.1f, vec3(0.88f, 0.88f, 0.86f));
                for (int k = 0; k < 4; ++k)
                    Rect(ac.x - 0.12f, ac.y - 0.07f + k * 0.04f, ac.x + 0.12f, ac.y - 0.06f + k * 0.04f, vec3(0.6f));
            }
            if (Hash(seed * 13 + i) > 0.55f)
                SatelliteDish(vec2(x0 + (x1 - x0) * 0.5f, y + 0.3f), 0.8f, Hash(seed + i) > 0.5f ? 1.0f : -1.0f);
        }

        // ground floor: a garage shutter or a door
        if (Hash(seed + 5) > 0.45f)
            Shutter(x0 + 0.25f, base, x0 + 0.25f + std::min(1.1f, (x1 - x0) * 0.55f), base + 0.75f);
        else
        {
            float dx = x0 + (x1 - x0) * 0.4f;
            Rect(dx - 0.22f, base, dx + 0.22f, base + 0.7f, vec3(0.35f, 0.22f, 0.12f));
            Rect(dx - 0.17f, base + 0.05f, dx + 0.17f, base + 0.62f, vec3(0.45f, 0.29f, 0.16f));
            Renderer2D::DrawCircle(vec2(dx + 0.1f, base + 0.35f), 0.025f, vec3(0.85f, 0.75f, 0.3f));
        }

        // roof satellite dishes
        if (Hash(seed + 12) > 0.35f)
            SatelliteDish(vec2(x0 + (x1 - x0) * 0.35f, top + 0.08f), 1.0f, 1.0f);
    }

    void PowerPole(float x, float time)
    {
        const float base = PLAYFIELD_TOP - 0.1f;
        Rect(x - 0.06f, base, x + 0.06f, WORLD_HEIGHT + 0.2f, vec3(0.55f, 0.52f, 0.48f));
        Rect(x - 0.06f, base, x - 0.02f, WORLD_HEIGHT + 0.2f, vec3(0.45f, 0.42f, 0.38f));
        Rect(x - 0.45f, 7.6f, x + 0.45f, 7.68f, vec3(0.35f, 0.32f, 0.3f));
        Rect(x - 0.35f, 7.25f, x + 0.35f, 7.31f, vec3(0.35f, 0.32f, 0.3f));
        for (int i = 0; i < 3; ++i)
            Renderer2D::DrawCircle(vec2(x - 0.35f + i * 0.35f, 7.71f), 0.035f, vec3(0.85f, 0.85f, 0.8f));
        // a small transformer box
        Rect(x + 0.07f, 6.6f, x + 0.3f, 6.95f, vec3(0.40f, 0.42f, 0.40f));
        // bundle of cables hanging down the pole
        Cable(vec2(x + 0.05f, 7.25f), vec2(x + 0.15f, base + 0.4f), -0.1f, 0.02f, CABLE);
        (void)time;
    }
}

namespace Environment
{
    void DrawBackground(float time)
    {
        // ---- sky: bands from blue to dusty beige ----
        const int bands = 8;
        for (int i = 0; i < bands; ++i)
        {
            float y0 = PLAYFIELD_TOP + (WORLD_HEIGHT - PLAYFIELD_TOP) * i / bands;
            float y1 = PLAYFIELD_TOP + (WORLD_HEIGHT - PLAYFIELD_TOP) * (i + 1) / bands;
            Rect(0.0f, y0, WORLD_WIDTH, y1 + 0.01f, glm::mix(SKY_LOW, SKY_TOP, (float)i / (bands - 1)));
        }
        // hazy sun and far minaret silhouettes (old Cairo skyline)
        Renderer2D::DrawCircle(vec2(11.5f, 8.2f), 0.55f, vec3(0.98f, 0.92f, 0.75f));
        Renderer2D::DrawCircle(vec2(11.5f, 8.2f), 0.42f, vec3(1.0f, 0.97f, 0.86f));
        const vec3 far(0.80f, 0.74f, 0.66f);
        for (int i = 0; i < 3; ++i)
        {
            float mx = 3.6f + i * 5.3f;
            Rect(mx - 0.08f, 6.0f, mx + 0.08f, 8.5f + 0.2f * i, far);
            Rect(mx - 0.13f, 7.6f + 0.2f * i, mx + 0.13f, 7.68f + 0.2f * i, far);
            Renderer2D::DrawCircle(vec2(mx, 8.5f + 0.2f * i), 0.1f, far);
            Tri(vec2(mx - 0.05f, 8.55f + 0.2f * i), vec2(mx + 0.05f, 8.55f + 0.2f * i), vec2(mx, 8.85f + 0.2f * i), far);
            Renderer2D::DrawEllipse(vec2(mx + 0.9f, 6.6f), vec2(0.6f, 0.45f), far);   // dome
        }

        // ---- the houses ----
        float x = 1.2f;
        int seed = 3;
        while (x < WORLD_WIDTH)
        {
            float w = 1.7f + Hash(seed) * 1.1f;
            float top = 7.9f + Hash(seed + 1) * 1.3f;
            House(x, x + w, top, seed, time);
            x += w + 0.04f;
            seed += 17;
        }

        // ---- power poles with tangled cables across the street ----
        const float poles[3] = { 4.9f, 10.1f, 14.6f };
        for (float p : poles)
            PowerPole(p, time);
        float sway = 0.03f * std::sin(time * 0.9f);
        for (int i = 0; i < 2; ++i)
        {
            Cable(vec2(1.4f, 7.5f + i * 0.1f), vec2(poles[0], 7.64f), 0.35f + sway + i * 0.1f, 0.02f, CABLE);
            Cable(vec2(poles[0], 7.64f), vec2(poles[1], 7.64f), 0.55f - sway + i * 0.15f, 0.02f, CABLE);
            Cable(vec2(poles[1], 7.64f), vec2(poles[2], 7.64f), 0.45f + sway + i * 0.12f, 0.02f, CABLE);
            Cable(vec2(poles[2], 7.64f), vec2(16.2f, 7.4f), 0.3f + i * 0.1f, 0.02f, CABLE);
            Cable(vec2(poles[0], 7.28f), vec2(poles[1], 7.28f), 0.8f + sway, 0.015f, CABLE);
            Cable(vec2(poles[1], 7.28f), vec2(poles[2], 7.28f), 0.7f - sway, 0.015f, CABLE);
        }
        // loose cables from the poles into the windows
        Cable(vec2(poles[0], 7.28f), vec2(3.1f, 6.9f), 0.2f, 0.015f, CABLE);
        Cable(vec2(poles[1], 7.28f), vec2(8.6f, 7.1f), 0.25f, 0.015f, CABLE);
        Cable(vec2(poles[2], 7.28f), vec2(12.9f, 6.8f), 0.2f, 0.015f, CABLE);

        // ---- back of the street: sidewalk, parked tuk-tuk, crates, tires ----
        Rect(0.0f, PLAYFIELD_TOP - 0.05f, WORLD_WIDTH, PLAYFIELD_TOP + 0.12f, vec3(0.70f, 0.66f, 0.60f));
        for (float bx = 1.6f; bx < WORLD_WIDTH; bx += 0.5f)
            Rect(bx, PLAYFIELD_TOP - 0.05f, bx + 0.02f, PLAYFIELD_TOP + 0.12f, vec3(0.55f, 0.52f, 0.48f));

        DrawTukTuk(vec2(7.0f, PLAYFIELD_TOP + 0.05f), 0.75f, 1.0f, 0.0f);
        Crate(vec2(12.1f, PLAYFIELD_TOP + 0.05f), 0.45f, 0.4f);
        Crate(vec2(12.2f, PLAYFIELD_TOP + 0.45f), 0.35f, 0.32f);
        FlatTire(vec2(13.0f, PLAYFIELD_TOP + 0.12f), 0.24f);
        FlatTire(vec2(13.0f, PLAYFIELD_TOP + 0.24f), 0.24f);
        FlatTire(vec2(13.0f, PLAYFIELD_TOP + 0.36f), 0.24f);
        Rubble(vec2(9.4f, PLAYFIELD_TOP + 0.1f), 0.8f, 40);
        Rubble(vec2(15.4f, PLAYFIELD_TOP + 0.1f), 0.7f, 60);
        Crate(vec2(3.5f, PLAYFIELD_TOP + 0.05f), 0.42f, 0.36f);
    }

    void DrawLane(int lane, float highlight)
    {
        const float y0 = LANE_Y[lane] - LANE_HEIGHT * 0.5f;
        const float y1 = LANE_Y[lane] + LANE_HEIGHT * 0.5f;

        Renderer2D::SetHighlight(highlight);

        // packed dirt road, a little different in every lane
        vec3 dirt = (lane % 2 == 0) ? SAND : glm::mix(SAND, SAND_DARK, 0.25f);
        Rect(LANE_LEFT, y0, WORLD_WIDTH, y1, dirt);
        Rect(LANE_LEFT, y0, WORLD_WIDTH, y0 + 0.22f, dirt * 0.93f);

        // tire tracks and patches of darker earth
        Rect(LANE_LEFT, LANE_Y[lane] - 0.18f, WORLD_WIDTH, LANE_Y[lane] - 0.15f, SAND_DARK);
        Rect(LANE_LEFT, LANE_Y[lane] + 0.12f, WORLD_WIDTH, LANE_Y[lane] + 0.15f, SAND_DARK);
        for (int i = 0; i < 7; ++i)
        {
            float px = LANE_LEFT + 0.6f + Hash(lane * 31 + i) * (LANE_WIDTH - 1.0f);
            float py = y0 + 0.25f + Hash(lane * 17 + i * 3) * (LANE_HEIGHT - 0.5f);
            Renderer2D::DrawEllipse(vec2(px, py), vec2(0.35f + Hash(i) * 0.3f, 0.08f), SAND_DARK * 1.05f);
        }

        // pebbles and bits of brick on the road
        for (int i = 0; i < 26; ++i)
        {
            float px = LANE_LEFT + 0.3f + Hash(lane * 101 + i) * (LANE_WIDTH - 0.5f);
            float py = y0 + 0.15f + Hash(lane * 57 + i * 5) * (LANE_HEIGHT - 0.35f);
            float r = 0.025f + Hash(lane * 13 + i) * 0.035f;
            vec3 col = Hash(i * 7 + lane) > 0.7f ? BRICK : STONE;
            Renderer2D::DrawEllipse(vec2(px, py), vec2(r * 1.4f, r), col);
        }

        // the cobblestone patch from the picture, in the middle lane
        if (lane == 1)
        {
            for (int row = 0; row < 6; ++row)
                for (int col = 0; col < 18; ++col)
                {
                    float px = 7.2f + col * 0.24f + (row % 2) * 0.12f;
                    float py = y0 + 0.3f + row * 0.2f;
                    float edge = std::fabs(col - 8.5f) / 9.0f + std::fabs(row - 2.5f) / 6.0f;
                    if (edge + Hash(row * 31 + col) * 0.35f > 1.1f)
                        continue;   // ragged, broken edge of the patch
                    vec3 c = glm::mix(STONE, STONE_DARK, Hash(row * 7 + col * 13) * 0.6f);
                    Renderer2D::DrawQuad(vec2(px, py), vec2(0.2f, 0.16f), c * 0.8f);
                    Renderer2D::DrawQuad(vec2(px, py + 0.01f), vec2(0.18f, 0.13f), c);
                }
        }

        // a row of broken curb stones along the top edge separates the lanes
        if (lane < NUM_LANES - 1)
        {
            for (int i = 0; i < 40; ++i)
            {
                float px = LANE_LEFT + 0.2f + i * 0.37f;
                if (Hash(lane * 211 + i) < 0.12f)
                    continue;   // missing stone
                float tilt = (Hash(lane * 7 + i) - 0.5f) * 0.12f;
                Renderer2D::DrawQuadRotated(vec2(px, y1 - 0.02f), vec2(0.33f, 0.13f), tilt, STONE_DARK);
                Renderer2D::DrawQuadRotated(vec2(px, y1 - 0.005f), vec2(0.31f, 0.09f), tilt, STONE);
            }
        }

        Renderer2D::SetHighlight(0.0f);
    }

    void DrawHomeBase(float time)
    {
        // Am Sayed's koshk: a bright green wooden kiosk built into the ground
        // floor of the corner house, the heart of the street.
        const vec3 green(0.10f, 0.52f, 0.36f), greenDark(0.06f, 0.36f, 0.24f);
        const vec3 yellow(0.98f, 0.80f, 0.20f), red(0.85f, 0.18f, 0.15f), white(0.97f, 0.95f, 0.90f);
        const float x0 = 0.0f, x1 = HOME_BASE_RIGHT;

        // the corner house above the koshk
        Rect(x0, 4.6f, x1 + 0.05f, 9.0f, BRICK);
        for (float y = 4.7f; y < 9.0f; y += 0.12f)
            Rect(x0, y, x1 + 0.05f, y + 0.018f, MORTAR);
        Window(vec2(0.75f, 7.6f), 0.5f, 0.6f, 5, true, time);

        // main body
        Rect(x0, PLAYFIELD_BOTTOM - 0.2f, x1, 5.1f, greenDark);
        Rect(x0, PLAYFIELD_BOTTOM - 0.2f, x1 - 0.08f, 5.0f, green);

        // ---- top: the big sign with light bulbs ----
        Rect(x0, 5.1f, x1 + 0.1f, 6.25f, vec3(0.18f, 0.10f, 0.05f));
        Rect(x0 + 0.06f, 5.16f, x1 + 0.04f, 6.19f, yellow);
        Renderer2D::DrawTextShadow("KOSHK", vec2(0.78f, 5.85f), 0.21f, red);
        Renderer2D::DrawText("AM SAYED", vec2(0.78f, 5.4f), 0.105f, vec3(0.18f, 0.10f, 0.05f));
        for (int i = 0; i < 6; ++i)
        {
            bool on = std::fmod(time * 3.0f + i * 0.5f, 2.0f) < 1.4f;
            Renderer2D::DrawCircle(vec2(0.15f + i * 0.26f, 6.2f), 0.05f, on ? vec3(1.0f, 0.95f, 0.6f) : vec3(0.6f, 0.5f, 0.3f));
        }
        SatelliteDish(vec2(1.2f, 6.25f), 1.1f, 1.0f);

        // ---- striped awning over the shop window ----
        for (int i = 0; i < 6; ++i)
        {
            float ax = x0 + i * 0.28f;
            Renderer2D::DrawPolygon(std::vector<vec2>{ vec2(ax, 5.05f), vec2(ax + 0.28f, 5.05f),
                vec2(ax + 0.34f, 4.6f), vec2(ax + 0.06f, 4.6f) }.data(), 4, i % 2 ? white : red);
        }
        for (int i = 0; i < 6; ++i)
            Renderer2D::DrawCircle(vec2(0.2f + i * 0.28f, 4.58f), 0.07f, i % 2 ? white : red);

        // ---- shop window full of goods (snacks, juice, bread) ----
        Rect(x0 + 0.08f, 2.85f, x1 - 0.12f, 4.45f, vec3(0.30f, 0.20f, 0.12f));
        const vec3 goods[6] = { vec3(0.95f, 0.30f, 0.25f), vec3(0.25f, 0.55f, 0.90f), vec3(0.98f, 0.82f, 0.25f),
                                vec3(0.45f, 0.75f, 0.30f), vec3(0.95f, 0.55f, 0.15f), vec3(0.75f, 0.35f, 0.75f) };
        for (int shelf = 0; shelf < 3; ++shelf)
        {
            float sy = 2.95f + shelf * 0.52f;
            Rect(x0 + 0.08f, sy, x1 - 0.12f, sy + 0.05f, WOOD);
            for (int k = 0; k < 6; ++k)
            {
                float gx = 0.2f + k * 0.2f;
                vec3 c = goods[(k + shelf * 2) % 6];
                if (shelf == 2) // bottles
                {
                    Rect(gx - 0.05f, sy + 0.05f, gx + 0.05f, sy + 0.32f, c);
                    Rect(gx - 0.025f, sy + 0.32f, gx + 0.025f, sy + 0.4f, c * 0.8f);
                }
                else            // snack bags
                {
                    Rect(gx - 0.08f, sy + 0.05f, gx + 0.08f, sy + 0.36f, c);
                    Rect(gx - 0.08f, sy + 0.28f, gx + 0.08f, sy + 0.33f, white);
                }
            }
        }
        // chips bags hanging on strings under the awning
        for (int k = 0; k < 4; ++k)
        {
            float hx = 0.25f + k * 0.34f;
            float sw = 0.02f * std::sin(time * 2.0f + k);
            Renderer2D::DrawLine(vec2(hx, 4.6f), vec2(hx + sw, 4.38f), 0.012f, vec3(0.2f));
            Rect(hx - 0.09f + sw, 4.05f, hx + 0.09f + sw, 4.38f, goods[(k + 2) % 6]);
            Rect(hx - 0.09f + sw, 4.25f, hx + 0.09f + sw, 4.3f, white);
        }

        // ---- the counter ----
        Rect(x0, 2.55f, x1 + 0.05f, 2.85f, WOOD_DARK);
        Rect(x0, 2.62f, x1 + 0.05f, 2.8f, WOOD);
        // a glass jar of sweets and a tea glass on the counter
        Rect(0.25f, 2.85f, 0.55f, 3.2f, vec3(0.75f, 0.85f, 0.85f));
        for (int k = 0; k < 4; ++k)
            Renderer2D::DrawCircle(vec2(0.31f + k * 0.07f, 2.95f + (k % 2) * 0.07f), 0.04f, goods[k]);
        Rect(1.05f, 2.85f, 1.17f, 3.03f, vec3(0.75f, 0.4f, 0.2f));

        // ---- lower part: drinks fridge and crates ----
        Rect(x0 + 0.12f, 0.75f, x1 - 0.2f, 2.45f, vec3(0.85f, 0.15f, 0.12f));
        Rect(x0 + 0.2f, 0.85f, x1 - 0.28f, 2.25f, vec3(0.65f, 0.85f, 0.90f));
        for (int r = 0; r < 4; ++r)
            for (int k = 0; k < 4; ++k)
            {
                float bx = 0.32f + k * 0.24f, by = 0.95f + r * 0.33f;
                Rect(bx - 0.05f, by, bx + 0.05f, by + 0.22f, goods[(r + k) % 6]);
            }
        Rect(x0 + 0.12f, 2.25f, x1 - 0.2f, 2.45f, vec3(0.85f, 0.15f, 0.12f));
        Renderer2D::DrawText("COLD", vec2(0.67f, 2.35f), 0.105f, white);
        Rect(x1 - 0.3f, 1.2f, x1 - 0.24f, 1.8f, vec3(0.8f));   // fridge handle

        // hanging light bulb that flickers
        bool flicker = std::fmod(time * 7.3f, 5.0f) > 0.25f;
        Renderer2D::DrawLine(vec2(0.78f, 4.45f), vec2(0.78f, 4.15f), 0.015f, vec3(0.1f));
        Renderer2D::DrawCircle(vec2(0.78f, 4.1f), 0.07f, flicker ? vec3(1.0f, 0.95f, 0.65f) : vec3(0.7f, 0.65f, 0.45f));

        // yellow trim along the edge facing the street
        Rect(x1 - 0.1f, PLAYFIELD_BOTTOM - 0.2f, x1, 5.1f, yellow);
        for (float y = 0.6f; y < 5.0f; y += 0.4f)
            Rect(x1 - 0.1f, y, x1, y + 0.18f, red);
    }

    void DrawForeground(float time)
    {
        // broken asphalt edge and the near curb
        Rect(0.0f, 0.0f, WORLD_WIDTH, PLAYFIELD_BOTTOM, vec3(0.40f, 0.38f, 0.36f));
        Rect(0.0f, PLAYFIELD_BOTTOM - 0.08f, WORLD_WIDTH, PLAYFIELD_BOTTOM, vec3(0.62f, 0.60f, 0.56f));
        for (int i = 0; i < 12; ++i)
        {
            float x = Hash(i * 7) * WORLD_WIDTH;
            Renderer2D::DrawLine(vec2(x, 0.1f), vec2(x + 0.3f, 0.35f + Hash(i) * 0.15f), 0.02f, vec3(0.28f, 0.27f, 0.26f));
        }
        Rubble(vec2(3.0f, 0.2f), 0.7f, 5);
        Rubble(vec2(11.2f, 0.18f), 0.6f, 21);
        Tire(vec2(6.4f, 0.28f), 0.22f);
        Tire(vec2(14.6f, 0.26f), 0.2f);
        Crate(vec2(8.6f, 0.05f), 0.4f, 0.32f);
        // a sleepy street cat on a crate, its tail swinging
        vec2 cat(8.8f, 0.48f);
        const vec3 fur(0.85f, 0.55f, 0.25f);
        Renderer2D::DrawEllipse(cat, vec2(0.17f, 0.09f), fur);
        Renderer2D::DrawCircle(cat + vec2(0.16f, 0.06f), 0.07f, fur);
        Tri(cat + vec2(0.12f, 0.11f), cat + vec2(0.15f, 0.18f), cat + vec2(0.18f, 0.11f), fur);
        Tri(cat + vec2(0.17f, 0.11f), cat + vec2(0.21f, 0.18f), cat + vec2(0.22f, 0.1f), fur);
        float tail = std::sin(time * 2.5f) * 0.4f;
        Renderer2D::DrawLine(cat + vec2(-0.15f, 0.0f), cat + vec2(-0.3f, 0.06f + 0.08f * tail), 0.035f, fur);
    }

    void DrawTukTuk(vec2 feet, float s, float f, float time)
    {
        auto P = [&](float x, float y) { return feet + vec2(x * s * f, y * s); };
        const vec3 body(0.12f, 0.12f, 0.13f), trim(0.98f, 0.78f, 0.15f), canopy(0.16f, 0.16f, 0.18f);

        // wheels
        float spin = time * 8.0f;
        for (float wx : { -0.55f, 0.62f })
        {
            Renderer2D::DrawCircle(P(wx, 0.2f), 0.2f * s, vec3(0.08f));
            Renderer2D::DrawCircle(P(wx, 0.2f), 0.1f * s, vec3(0.7f));
            Renderer2D::DrawQuadRotated(P(wx, 0.2f), vec2(0.16f, 0.03f) * s, spin, vec3(0.3f));
        }
        // body: rounded passenger cabin and the pointed front
        Renderer2D::DrawPolygon(std::vector<vec2>{ P(-0.85f, 0.25f), P(0.55f, 0.25f), P(0.95f, 0.35f), P(0.85f, 0.75f),
            P(0.45f, 0.85f), P(-0.85f, 0.85f) }.data(), 6, body);
        Renderer2D::DrawPolygon(std::vector<vec2>{ P(-0.85f, 0.42f), P(0.9f, 0.42f), P(0.88f, 0.5f), P(-0.85f, 0.5f) }.data(), 4, trim);
        // canopy roof
        Renderer2D::DrawPolygon(std::vector<vec2>{ P(-0.9f, 0.85f), P(0.5f, 0.85f), P(0.6f, 1.45f), P(-0.7f, 1.5f), P(-0.92f, 1.3f) }.data(), 5, canopy);
        Renderer2D::DrawPolygon(std::vector<vec2>{ P(-0.7f, 1.5f), P(0.6f, 1.45f), P(0.62f, 1.38f), P(-0.72f, 1.42f) }.data(), 4, trim);
        // windscreen and passenger window
        Renderer2D::DrawPolygon(std::vector<vec2>{ P(0.42f, 0.88f), P(0.55f, 0.88f), P(0.6f, 1.38f), P(0.45f, 1.38f) }.data(), 4, vec3(0.6f, 0.75f, 0.8f));
        Renderer2D::DrawPolygon(std::vector<vec2>{ P(-0.6f, 0.92f), P(0.25f, 0.92f), P(0.25f, 1.32f), P(-0.55f, 1.36f) }.data(), 4, vec3(0.30f, 0.30f, 0.32f));
        // headlight and a hanging decoration
        Renderer2D::DrawCircle(P(0.86f, 0.62f), 0.07f * s, vec3(1.0f, 0.95f, 0.7f));
        Renderer2D::DrawLine(P(0.48f, 1.36f), P(0.48f, 1.18f), 0.012f * s, vec3(0.9f, 0.2f, 0.2f));
        Renderer2D::DrawCircle(P(0.48f, 1.15f), 0.03f * s, vec3(0.2f, 0.4f, 0.9f));
        // number plate
        Renderer2D::DrawQuad(P(-0.82f, 0.36f), vec2(0.14f, 0.1f) * s, vec3(0.9f, 0.9f, 0.85f));
    }
}
