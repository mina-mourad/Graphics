#include "Draw.h"
#include "Renderer2D.h"

#include <algorithm>
#include <cmath>

namespace Draw
{
    glm::vec3 Apply(glm::vec3 c, const Fx& fx)
    {
        if (fx.red > 0.0f)
            c = glm::mix(c, glm::vec3(0.88f, 0.06f, 0.06f), std::min(1.0f, fx.red) * 0.8f);
        if (fx.gray > 0.0f) {
            float l = c.r * 0.299f + c.g * 0.587f + c.b * 0.114f;
            c = glm::mix(c, glm::vec3(l * 0.9f), fx.gray);
        }
        return c;
    }

    void DrawLine(glm::vec2 a, glm::vec2 b, float width, glm::vec3 color, float alpha)
    {
        glm::vec2 d = b - a;
        float len = std::sqrt(d.x * d.x + d.y * d.y);
        if (len < 1e-5f) return;
        float ang = std::atan2(d.y, d.x);
        Renderer2D::DrawQuadRot((a + b) * 0.5f, glm::vec2(len, width), ang, color, alpha);
    }

    void Shadow(glm::vec2 foot, float radius, float alpha)
    {
        Renderer2D::DrawEllipse(foot + glm::vec2(0.0f, -0.02f), glm::vec2(radius, radius * 0.22f), 0.0f,
                                glm::vec3(0.12f, 0.07f, 0.03f), alpha);
    }

    void Plaque(glm::vec2 c, glm::vec2 s, float highlight, const Fx& fx)
    {
        // Carved sandstone slab: dark frame, lighter face, gold trim.
        Renderer2D::DrawQuadRot(c + glm::vec2(0.04f, -0.05f), s, 0.0f, glm::vec3(0.10f, 0.06f, 0.03f), 0.45f * fx.alpha);
        Renderer2D::DrawQuadRot(c, s, 0.0f, Apply(glm::vec3(0.30f, 0.20f, 0.10f), fx), fx.alpha, highlight);
        Renderer2D::DrawQuadRot(c, s - glm::vec2(0.10f), 0.0f, Apply(glm::vec3(0.73f, 0.58f, 0.34f), fx), fx.alpha, highlight);
        Renderer2D::DrawQuadRot(c, s - glm::vec2(0.20f), 0.0f, Apply(glm::vec3(0.66f, 0.50f, 0.27f), fx), fx.alpha, highlight);
        glm::vec3 gold = Apply(glm::vec3(0.95f, 0.75f, 0.20f), fx);
        float hx = s.x * 0.5f - 0.05f, hy = s.y * 0.5f - 0.05f;
        Renderer2D::DrawQuadRot(c + glm::vec2(0.0f,  hy), glm::vec2(s.x - 0.10f, 0.03f), 0.0f, gold, fx.alpha);
        Renderer2D::DrawQuadRot(c + glm::vec2(0.0f, -hy), glm::vec2(s.x - 0.10f, 0.03f), 0.0f, gold, fx.alpha);
        Renderer2D::DrawQuadRot(c + glm::vec2( hx, 0.0f), glm::vec2(0.03f, s.y - 0.10f), 0.0f, gold, fx.alpha);
        Renderer2D::DrawQuadRot(c + glm::vec2(-hx, 0.0f), glm::vec2(0.03f, s.y - 0.10f), 0.0f, gold, fx.alpha);
    }

    // ---- Projectiles / items ------------------------------------------------

    void Arrow(glm::vec2 p, float)
    {
        glm::vec3 wood(0.45f, 0.28f, 0.12f), steel(0.85f, 0.88f, 0.92f), fletch(0.85f, 0.15f, 0.15f);
        DrawLine(p + glm::vec2(-0.30f, 0.0f), p + glm::vec2(0.22f, 0.0f), 0.045f, wood);
        Renderer2D::DrawTriangle(p + glm::vec2(0.20f, 0.07f), p + glm::vec2(0.20f, -0.07f), p + glm::vec2(0.38f, 0.0f), steel);
        Renderer2D::DrawTriangle(p + glm::vec2(-0.30f, 0.0f), p + glm::vec2(-0.40f, 0.09f), p + glm::vec2(-0.22f, 0.0f), fletch);
        Renderer2D::DrawTriangle(p + glm::vec2(-0.30f, 0.0f), p + glm::vec2(-0.40f, -0.09f), p + glm::vec2(-0.22f, 0.0f), fletch);
    }

    void SpiritBolt(glm::vec2 p, float t)
    {
        float pulse = 0.5f + 0.5f * std::sin(t * 18.0f);
        for (int i = 3; i >= 1; --i)
            Renderer2D::DrawEllipse(p + glm::vec2(0.16f * i, 0.0f), glm::vec2(0.16f - 0.03f * i, 0.10f - 0.02f * i), 0.0f,
                                    glm::vec3(0.55f, 0.15f, 0.80f), 0.35f);
        Renderer2D::DrawEllipse(p, glm::vec2(0.20f, 0.20f), 0.0f, glm::vec3(0.45f, 0.10f, 0.65f));
        Renderer2D::DrawEllipse(p, glm::vec2(0.13f + 0.02f * pulse, 0.13f + 0.02f * pulse), 0.0f, glm::vec3(0.80f, 0.45f, 1.00f));
        Renderer2D::DrawEllipse(p, glm::vec2(0.06f, 0.06f), 0.0f, glm::vec3(1.0f, 0.95f, 1.0f));
    }

    void StingerBolt(glm::vec2 p, float t)
    {
        float pulse = 0.5f + 0.5f * std::sin(t * 20.0f);
        for (int i = 3; i >= 1; --i)
            Renderer2D::DrawEllipse(p + glm::vec2(0.15f * i, 0.0f), glm::vec2(0.14f - 0.025f * i, 0.07f), 0.0f,
                                    glm::vec3(0.35f, 0.85f, 0.15f), 0.35f);
        Renderer2D::DrawTriangle(p + glm::vec2(0.0f, 0.12f), p + glm::vec2(0.0f, -0.12f), p + glm::vec2(-0.34f, 0.0f), glm::vec3(0.22f, 0.12f, 0.08f));
        Renderer2D::DrawTriangle(p + glm::vec2(0.0f, 0.07f), p + glm::vec2(0.0f, -0.07f), p + glm::vec2(-0.24f, 0.0f), glm::vec3(0.55f + 0.2f * pulse, 0.95f, 0.20f));
        Renderer2D::DrawEllipse(p + glm::vec2(0.04f, 0.0f), glm::vec2(0.09f, 0.09f), 0.0f, glm::vec3(0.30f, 0.80f, 0.12f));
    }

    void Coin(glm::vec2 p, float t, float scale, const Fx& fx)
    {
        // A spinning gold ankh-coin: edge-on squash driven by time.
        float spin = std::fabs(std::cos(t * 3.0f));
        float rx = (0.06f + 0.18f * spin) * scale, ry = 0.24f * scale;
        Renderer2D::DrawEllipse(p + glm::vec2(0.0f, -0.04f), glm::vec2(rx, ry * 0.25f), 0.0f, glm::vec3(0.1f, 0.06f, 0.02f), 0.35f * fx.alpha);
        Renderer2D::DrawEllipse(p, glm::vec2(rx, ry), 0.0f, Apply(glm::vec3(0.62f, 0.42f, 0.05f), fx), fx.alpha, fx.flash);
        Renderer2D::DrawEllipse(p, glm::vec2(rx * 0.84f, ry * 0.84f), 0.0f, Apply(glm::vec3(1.0f, 0.82f, 0.18f), fx), fx.alpha, fx.flash);
        Renderer2D::DrawEllipse(p, glm::vec2(rx * 0.62f, ry * 0.62f), 0.0f, Apply(glm::vec3(0.88f, 0.66f, 0.08f), fx), fx.alpha, fx.flash);
        // ankh mark
        glm::vec3 mark = Apply(glm::vec3(0.55f, 0.36f, 0.04f), fx);
        float k = scale * spin;
        Renderer2D::DrawQuadRot(p + glm::vec2(0.0f, -0.03f * scale), glm::vec2(0.035f * k + 0.005f, 0.17f * scale), 0.0f, mark, fx.alpha);
        Renderer2D::DrawQuadRot(p + glm::vec2(0.0f, 0.0f), glm::vec2(0.11f * k + 0.005f, 0.03f * scale), 0.0f, mark, fx.alpha);
        Renderer2D::DrawEllipse(p + glm::vec2(0.0f, 0.075f * scale), glm::vec2(0.03f * k + 0.005f, 0.045f * scale), 0.0f, mark, fx.alpha);
        Renderer2D::DrawEllipse(p + glm::vec2(-0.04f * rx / 0.24f, 0.09f * ry), glm::vec2(0.012f * scale, 0.012f * scale), 0.0f, glm::vec3(1.0f, 0.97f, 0.7f), fx.alpha);
    }

    void HitSpark(glm::vec2 p, float k, glm::vec3 col)
    {
        float r = 0.10f + 0.30f * k, a = 1.0f - k;
        Renderer2D::DrawEllipse(p, glm::vec2(r, r), 0.0f, col, 0.55f * a);
        for (int i = 0; i < 8; ++i) {
            float ang = 0.785398f * static_cast<float>(i) + 0.3f;
            glm::vec2 d(std::cos(ang), std::sin(ang));
            DrawLine(p + d * (r * 0.8f), p + d * (r * 1.7f), 0.045f, glm::mix(col, glm::vec3(1.0f), 0.5f), a);
        }
    }

    void HealthBar(glm::vec2 c, float w, int hp, int maxHp, bool friendly)
    {
        float h = 0.11f;
        Renderer2D::DrawQuadRot(c, glm::vec2(w + 0.05f, h + 0.05f), 0.0f, glm::vec3(0.05f, 0.03f, 0.02f), 0.85f);
        float frac = maxHp > 0 ? static_cast<float>(std::max(hp, 0)) / static_cast<float>(maxHp) : 0.0f;
        glm::vec3 col = friendly ? glm::mix(glm::vec3(0.90f, 0.20f, 0.10f), glm::vec3(0.20f, 0.85f, 0.35f), frac)
                                 : glm::mix(glm::vec3(0.60f, 0.05f, 0.05f), glm::vec3(0.85f, 0.25f, 0.85f), frac);
        float fw = w * frac;
        if (fw > 0.0f)
            Renderer2D::DrawQuadRot(glm::vec2(c.x - w * 0.5f + fw * 0.5f, c.y), glm::vec2(fw, h), 0.0f, col);
        // one notch per life so the exact remaining health can be counted
        for (int i = 1; i < maxHp; ++i) {
            float x = c.x - w * 0.5f + w * static_cast<float>(i) / static_cast<float>(maxHp);
            Renderer2D::DrawQuadRot(glm::vec2(x, c.y), glm::vec2(0.012f, h), 0.0f, glm::vec3(0.05f, 0.03f, 0.02f), 0.9f);
        }
    }

    void CardPortrait(PlayerType type, glm::vec2 c, float time, const Fx& fx)
    {
        Pose p;
        p.foot = c + glm::vec2(0.0f, -0.55f);
        p.scale = 0.88f;
        p.time = time;
        p.fx = fx;
        switch (type) {
        case PlayerType::Melee:   Melee(p); break;
        case PlayerType::Ranged:  Archer(p); break;
        case PlayerType::Defense: Guardian(p); break;
        }
    }
}
