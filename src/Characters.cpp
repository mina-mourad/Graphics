#include "Draw.h"
#include "Renderer2D.h"

#include <algorithm>
#include <cmath>

// Character models. Every model is assembled from rectangles, triangles and
// ellipses so each one reads as a real figure (head, limbs, clothing,
// equipment) and the three players / three enemies differ in shape, equipment
// and silhouette - not only in colour.

namespace
{
    using glm::vec2;
    using glm::vec3;
    using Draw::Fx;
    using Draw::Pose;

    const float PI = 3.14159265358979f;

    float EaseOut(float t) { t = std::min(1.0f, std::max(0.0f, t)); return 1.0f - (1.0f - t) * (1.0f - t); }
    float Lerp(float a, float b, float t) { return a + (b - a) * t; }

    // Draws in the model's local space (origin = centre of the feet, +X = the
    // direction the character faces, +Y = up).
    struct Pen
    {
        vec2 o;
        float f;     // +1 faces right, -1 faces left
        float s;     // overall scale
        float sy;    // vertical squash (ducking)
        Fx fx;

        explicit Pen(const Pose& p)
            : o(p.foot), f(p.facing >= 0.0f ? 1.0f : -1.0f), s(p.scale),
              sy(1.0f - 0.38f * p.duck), fx(p.fx) {}

        vec2 W(vec2 l) const { return o + vec2(f * l.x * s, l.y * s * sy); }
        float A(float a) const { return f > 0.0f ? a : -a; }
        vec3 C(vec3 c) const { return Draw::Apply(c, fx); }

        void Rect(vec2 c, vec2 size, vec3 col, float ang = 0.0f) const
        {
            Renderer2D::DrawQuadRot(W(c), vec2(size.x * s, size.y * s * sy), A(ang), C(col), fx.alpha, fx.flash);
        }
        void Ell(vec2 c, vec2 r, vec3 col, float ang = 0.0f) const
        {
            Renderer2D::DrawEllipse(W(c), vec2(r.x * s, r.y * s * sy), A(ang), C(col), fx.alpha, fx.flash);
        }
        void Tri(vec2 a, vec2 b, vec2 c, vec3 col) const
        {
            Renderer2D::DrawTriangle(W(a), W(b), W(c), C(col), fx.alpha, fx.flash);
        }
        // A limb hanging from `pivot` along angle `ang` (radians, local space).
        // Returns the far end so limbs can be chained.
        vec2 Limb(vec2 pivot, float len, float width, float ang, vec3 col) const
        {
            vec2 d(std::cos(ang), std::sin(ang));
            Rect(pivot + d * (len * 0.5f), vec2(len, width), col, ang);
            Ell(pivot, vec2(width * 0.5f), col);
            return pivot + d * len;
        }
        void Line(vec2 a, vec2 b, float width, vec3 col) const
        {
            vec2 d = b - a;
            float len = std::sqrt(d.x * d.x + d.y * d.y);
            if (len < 1e-5f) return;
            Rect((a + b) * 0.5f, vec2(len, width), col, std::atan2(d.y, d.x));
        }
        // Thin bands across a limb (bandages).
        void Bands(vec2 pivot, float len, float width, float ang, vec3 col, int n) const
        {
            vec2 d(std::cos(ang), std::sin(ang));
            for (int i = 0; i < n; ++i) {
                float t = (static_cast<float>(i) + 0.6f) / static_cast<float>(n);
                Rect(pivot + d * (len * t), vec2(0.035f, width * 1.02f), col, ang + PI * 0.5f);
            }
        }
    };

    const vec3 SKIN(0.74f, 0.50f, 0.32f);
    const vec3 SKIN_DARK(0.50f, 0.32f, 0.20f);
    const vec3 GOLD(0.96f, 0.76f, 0.20f);
    const vec3 LAPIS(0.12f, 0.24f, 0.62f);
    const vec3 STEEL(0.86f, 0.90f, 0.95f);
    const vec3 LINEN(0.96f, 0.94f, 0.86f);
}

// ===========================================================================
//  PLAYERS
// ===========================================================================

void Draw::Melee(const Pose& p)
{
    Pen g(p);
    float sw = std::sin(p.walk) * p.moving;
    float bob = std::fabs(std::sin(p.walk)) * 0.03f * p.moving;
    if (p.fx.alpha > 0.5f && p.air < 0.1f) Draw::Shadow(p.foot, 0.30f * p.scale, 0.35f * p.fx.alpha);

    // legs + sandals
    for (int side = -1; side <= 1; side += 2) {
        float sgn = static_cast<float>(side);
        float ang = -PI * 0.5f + sgn * sw * 0.55f + sgn * p.air * 0.45f;
        vec2 end = g.Limb(vec2(0.05f * sgn, 0.50f + bob), 0.46f, 0.11f, ang, SKIN);
        g.Rect(end + vec2(0.05f, 0.02f), vec2(0.17f, 0.06f), vec3(0.55f, 0.35f, 0.15f));
        g.Rect(end + vec2(0.0f, 0.10f), vec2(0.12f, 0.03f), GOLD);   // ankle band
    }

    // kilt (shendyt) with pleats and apron
    g.Rect(vec2(0.0f, 0.57f + bob), vec2(0.36f, 0.22f), LINEN);
    g.Tri(vec2(0.18f, 0.68f + bob), vec2(0.18f, 0.40f + bob), vec2(0.32f, 0.42f + bob), vec3(0.90f, 0.87f, 0.78f));
    for (int i = 0; i < 3; ++i)
        g.Rect(vec2(-0.09f + 0.09f * i, 0.55f + bob), vec2(0.015f, 0.18f), vec3(0.78f, 0.74f, 0.64f));
    g.Rect(vec2(0.0f, 0.68f + bob), vec2(0.38f, 0.055f), GOLD);                 // belt
    g.Rect(vec2(0.04f, 0.68f + bob), vec2(0.07f, 0.045f), vec3(0.15f, 0.65f, 0.65f));  // turquoise buckle

    // torso + usekh collar
    g.Rect(vec2(0.0f, 0.86f + bob), vec2(0.30f, 0.36f), SKIN);
    g.Rect(vec2(0.0f, 0.78f + bob), vec2(0.30f, 0.03f), SKIN_DARK);
    g.Ell(vec2(0.02f, 0.98f + bob), vec2(0.19f, 0.10f), GOLD);
    g.Ell(vec2(0.02f, 0.97f + bob), vec2(0.14f, 0.065f), vec3(0.12f, 0.60f, 0.62f));
    g.Ell(vec2(0.02f, 0.965f + bob), vec2(0.09f, 0.04f), GOLD);

    // shield arm (held in front of the body)
    vec2 hand = g.Limb(vec2(-0.02f, 0.98f + bob), 0.27f, 0.095f, -0.95f + sw * 0.12f, SKIN_DARK);
    vec2 sc = hand + vec2(0.07f, 0.0f);
    g.Ell(sc, vec2(0.155f, 0.215f), vec3(0.40f, 0.22f, 0.06f));
    g.Ell(sc, vec2(0.125f, 0.185f), vec3(0.80f, 0.56f, 0.18f));
    g.Ell(sc, vec2(0.085f, 0.13f), vec3(0.68f, 0.44f, 0.12f));
    g.Ell(sc, vec2(0.045f, 0.045f), GOLD);
    g.Rect(sc, vec2(0.02f, 0.30f), vec3(0.40f, 0.22f, 0.06f));

    // head, nemes headdress, uraeus
    vec2 hc(0.03f, 1.13f + bob);
    g.Rect(hc + vec2(-0.12f, -0.07f), vec2(0.11f, 0.30f), LAPIS);
    g.Rect(hc + vec2(-0.12f, -0.07f), vec2(0.11f, 0.02f), GOLD);
    g.Rect(hc + vec2(-0.12f, -0.12f), vec2(0.11f, 0.02f), GOLD);
    g.Ell(hc, vec2(0.115f, 0.125f), SKIN);
    g.Ell(hc + vec2(-0.01f, 0.07f), vec2(0.155f, 0.105f), LAPIS);
    for (int i = 0; i < 3; ++i)
        g.Rect(hc + vec2(-0.01f, 0.03f + 0.045f * i), vec2(0.30f, 0.016f), GOLD);
    g.Rect(hc + vec2(0.11f, -0.07f), vec2(0.045f, 0.22f), LAPIS);   // front lappet
    g.Tri(hc + vec2(0.10f, 0.10f), hc + vec2(0.16f, 0.10f), hc + vec2(0.13f, 0.20f), GOLD);   // uraeus
    g.Ell(hc + vec2(0.07f, 0.0f), vec2(0.03f, 0.02f), vec3(1.0f));
    g.Ell(hc + vec2(0.08f, 0.0f), vec2(0.016f, 0.016f), vec3(0.05f));
    g.Rect(hc + vec2(0.085f, 0.0f), vec2(0.07f, 0.012f), vec3(0.05f));        // eyeliner
    g.Tri(hc + vec2(0.11f, -0.02f), hc + vec2(0.11f, -0.07f), hc + vec2(0.16f, -0.06f), SKIN_DARK);   // nose

    // sword arm + khopesh
    float swing = p.attack < 0.0f ? 0.85f : Lerp(2.15f, -0.85f, EaseOut(p.attack));
    vec2 shoulder(0.12f, 1.0f + bob);
    vec2 wrist = g.Limb(shoulder, 0.28f, 0.095f, swing, SKIN);
    float bladeAng = swing + 0.35f;
    vec2 d(std::cos(bladeAng), std::sin(bladeAng)), n(-d.y, d.x);
    g.Rect(wrist, vec2(0.11f, 0.045f), GOLD, bladeAng + PI * 0.5f);                  // grip guard
    vec2 tip = g.Limb(wrist, 0.60f, 0.075f, bladeAng, STEEL);
    g.Limb(wrist, 0.60f, 0.025f, bladeAng, vec3(1.0f));                               // blade shine
    g.Tri(tip - d * 0.20f - n * 0.04f, tip - d * 0.20f + n * 0.04f, tip + d * 0.06f - n * 0.13f, STEEL);   // khopesh hook
    if (p.attack > 0.45f) {
        // slash arc
        float k = (p.attack - 0.45f) / 0.55f;
        g.Tri(wrist + d * 0.55f, tip + n * 0.10f - d * 0.35f, tip + d * 0.10f - n * 0.25f, vec3(1.0f, 0.97f, 0.8f));
        (void)k;
    }
}

void Draw::Archer(const Pose& p)
{
    Pen g(p);
    float sw = std::sin(p.walk) * p.moving;
    float bob = std::fabs(std::sin(p.walk)) * 0.03f * p.moving;
    if (p.fx.alpha > 0.5f && p.air < 0.1f) Draw::Shadow(p.foot, 0.27f * p.scale, 0.35f * p.fx.alpha);

    const vec3 DSKIN(0.46f, 0.28f, 0.17f);
    const vec3 TEAL(0.10f, 0.60f, 0.58f);

    // quiver on the back (behind the body)
    g.Rect(vec2(-0.17f, 0.93f + bob), vec2(0.11f, 0.42f), vec3(0.42f, 0.24f, 0.10f), 0.25f);
    g.Rect(vec2(-0.17f, 0.93f + bob), vec2(0.11f, 0.03f), GOLD, 0.25f);
    for (int i = 0; i < 3; ++i) {
        vec2 t(-0.205f + 0.04f * i, 1.17f + bob);
        g.Rect(t + vec2(0.012f * i, 0.04f), vec2(0.014f, 0.12f), vec3(0.55f, 0.40f, 0.20f), 0.25f);
        g.Tri(t + vec2(-0.03f, 0.10f), t + vec2(0.03f, 0.10f), t + vec2(0.0f, 0.20f), vec3(0.88f, 0.18f, 0.18f));
    }

    // legs with leather wraps
    for (int side = -1; side <= 1; side += 2) {
        float sgn = static_cast<float>(side);
        float ang = -PI * 0.5f + sgn * sw * 0.6f + sgn * p.air * 0.45f;
        vec2 end = g.Limb(vec2(0.045f * sgn, 0.52f + bob), 0.47f, 0.085f, ang, DSKIN);
        g.Bands(vec2(0.045f * sgn, 0.52f + bob), 0.47f, 0.09f, ang, vec3(0.45f, 0.28f, 0.12f), 4);
        g.Rect(end + vec2(0.05f, 0.02f), vec2(0.15f, 0.05f), vec3(0.40f, 0.25f, 0.10f));
    }

    // short tunic + sash
    g.Rect(vec2(0.0f, 0.58f + bob), vec2(0.28f, 0.16f), vec3(0.08f, 0.45f, 0.45f));
    g.Rect(vec2(0.0f, 0.82f + bob), vec2(0.25f, 0.46f), TEAL);
    g.Rect(vec2(0.0f, 0.84f + bob), vec2(0.33f, 0.06f), vec3(0.85f, 0.16f, 0.16f), 0.75f);   // red sash
    g.Rect(vec2(0.0f, 0.68f + bob), vec2(0.28f, 0.04f), GOLD);

    // head: hair, headband, feather
    vec2 hc(0.02f, 1.12f + bob);
    g.Tri(hc + vec2(-0.08f, 0.04f), hc + vec2(-0.08f, -0.06f), hc + vec2(-0.28f, -0.12f), vec3(0.05f));   // ponytail
    g.Ell(hc, vec2(0.105f, 0.12f), DSKIN);
    g.Ell(hc + vec2(-0.02f, 0.06f), vec2(0.125f, 0.09f), vec3(0.06f, 0.05f, 0.05f));
    g.Rect(hc + vec2(0.0f, 0.07f), vec2(0.23f, 0.035f), vec3(0.85f, 0.16f, 0.16f));
    g.Tri(hc + vec2(-0.08f, 0.08f), hc + vec2(-0.02f, 0.08f), hc + vec2(-0.12f, 0.26f), LINEN);   // feather
    g.Ell(hc + vec2(0.07f, 0.0f), vec2(0.028f, 0.02f), vec3(1.0f));
    g.Ell(hc + vec2(0.078f, 0.0f), vec2(0.015f, 0.015f), vec3(0.05f));
    g.Tri(hc + vec2(0.10f, -0.01f), hc + vec2(0.10f, -0.06f), hc + vec2(0.145f, -0.055f), vec3(0.40f, 0.24f, 0.14f));

    // bow arm + bow
    float pull = p.attack < 0.0f ? 0.0f : EaseOut(p.attack);
    vec2 shoulder(0.10f, 0.99f + bob);
    vec2 grip = g.Limb(shoulder, 0.30f, 0.085f, 0.06f, DSKIN);
    vec2 bc = grip - vec2(0.26f, 0.0f);   // bow arc centre
    vec2 prev;
    for (int i = 0; i <= 10; ++i) {
        float th = -1.15f + 2.30f * static_cast<float>(i) / 10.0f;
        vec2 pt = bc + vec2(std::cos(th), std::sin(th)) * 0.40f;
        if (i > 0) g.Line(prev, pt, 0.055f, vec3(0.50f, 0.29f, 0.10f));
        prev = pt;
    }
    vec2 top = bc + vec2(std::cos(1.15f), std::sin(1.15f)) * 0.40f;
    vec2 bot = bc + vec2(std::cos(-1.15f), std::sin(-1.15f)) * 0.40f;
    g.Ell(top, vec2(0.03f), GOLD);
    g.Ell(bot, vec2(0.03f), GOLD);
    vec2 nock = vec2(grip.x - 0.06f - 0.30f * pull, grip.y);
    g.Line(top, nock, 0.014f, vec3(0.95f, 0.92f, 0.80f));
    g.Line(bot, nock, 0.014f, vec3(0.95f, 0.92f, 0.80f));
    g.Rect(grip + vec2(-0.02f, 0.0f), vec2(0.07f, 0.11f), vec3(0.30f, 0.16f, 0.06f));    // grip wrap
    if (p.attack >= 0.0f) {
        // arrow being drawn, plus the drawing hand
        g.Line(nock, grip + vec2(0.30f, 0.0f), 0.032f, vec3(0.45f, 0.28f, 0.12f));
        g.Tri(grip + vec2(0.28f, 0.06f), grip + vec2(0.28f, -0.06f), grip + vec2(0.44f, 0.0f), STEEL);
        g.Tri(nock + vec2(0.0f, 0.0f), nock + vec2(0.0f, 0.07f) + vec2(0.04f, 0.0f), nock + vec2(0.10f, 0.0f), vec3(0.85f, 0.16f, 0.16f));
        g.Tri(nock + vec2(0.0f, 0.0f), nock + vec2(0.0f, -0.07f) + vec2(0.04f, 0.0f), nock + vec2(0.10f, 0.0f), vec3(0.85f, 0.16f, 0.16f));
    }
    // string hand
    vec2 sh(-0.02f, 0.98f + bob);
    vec2 dv = nock - sh;
    float la = std::atan2(dv.y, dv.x);
    float ll = std::sqrt(dv.x * dv.x + dv.y * dv.y);
    g.Limb(sh, std::min(ll, 0.34f), 0.08f, la, DSKIN);
}

void Draw::Guardian(const Pose& p)
{
    Pen g(p);
    float breathe = std::sin(p.time * 2.0f) * 0.008f;
    if (p.fx.alpha > 0.5f) Draw::Shadow(p.foot, 0.46f * p.scale, 0.38f * p.fx.alpha);

    const vec3 STONE(0.66f, 0.60f, 0.48f), STONE_D(0.50f, 0.45f, 0.35f), STONE_L(0.78f, 0.72f, 0.58f);
    const vec3 GRANITE(0.36f, 0.34f, 0.34f);

    // plinth and legs (carved block)
    g.Rect(vec2(0.0f, 0.07f), vec2(0.84f, 0.14f), STONE_D);
    g.Rect(vec2(0.0f, 0.16f), vec2(0.72f, 0.05f), STONE_L);
    g.Rect(vec2(0.0f, 0.38f), vec2(0.58f, 0.40f), STONE);
    g.Rect(vec2(0.0f, 0.38f), vec2(0.015f, 0.38f), STONE_D);
    g.Rect(vec2(0.0f, 0.60f), vec2(0.62f, 0.06f), GOLD);                        // gold belt
    for (int i = 0; i < 4; ++i)
        g.Rect(vec2(-0.21f + 0.14f * i, 0.36f), vec2(0.035f, 0.07f), STONE_D);    // pleats

    // torso, shoulders, arms crossed
    g.Rect(vec2(0.0f, 0.84f + breathe), vec2(0.54f, 0.42f), STONE_L);
    g.Ell(vec2(-0.27f, 0.99f + breathe), vec2(0.13f, 0.12f), STONE);
    g.Ell(vec2(0.27f, 0.99f + breathe), vec2(0.13f, 0.12f), STONE);
    g.Rect(vec2(0.0f, 0.86f + breathe), vec2(0.52f, 0.11f), STONE_D, 0.38f);
    g.Rect(vec2(0.0f, 0.86f + breathe), vec2(0.52f, 0.11f), STONE, -0.38f);
    g.Ell(vec2(-0.21f, 0.74f + breathe), vec2(0.07f), STONE_D);
    g.Ell(vec2(0.21f, 0.74f + breathe), vec2(0.07f), STONE_D);

    // usekh collar
    g.Ell(vec2(0.0f, 1.03f + breathe), vec2(0.27f, 0.11f), GOLD);
    g.Ell(vec2(0.0f, 1.02f + breathe), vec2(0.21f, 0.075f), vec3(0.12f, 0.58f, 0.60f));
    g.Ell(vec2(0.0f, 1.015f + breathe), vec2(0.15f, 0.05f), GOLD);

    // head: nemes with stripes, face, uraeus, false beard
    vec2 hc(0.0f, 1.16f + breathe);
    g.Rect(hc + vec2(-0.185f, -0.08f), vec2(0.085f, 0.34f), LAPIS);
    g.Rect(hc + vec2(0.185f, -0.08f), vec2(0.085f, 0.34f), LAPIS);
    for (int i = 0; i < 5; ++i) {
        g.Rect(hc + vec2(-0.185f, -0.20f + 0.07f * i), vec2(0.085f, 0.018f), GOLD);
        g.Rect(hc + vec2(0.185f, -0.20f + 0.07f * i), vec2(0.085f, 0.018f), GOLD);
    }
    g.Ell(hc + vec2(0.0f, 0.06f), vec2(0.22f, 0.13f), LAPIS);
    for (int i = 0; i < 4; ++i)
        g.Rect(hc + vec2(0.0f, 0.0f + 0.045f * i), vec2(0.40f, 0.016f), GOLD);
    g.Ell(hc, vec2(0.125f, 0.15f), vec3(0.72f, 0.65f, 0.52f));
    g.Rect(hc + vec2(0.0f, 0.02f), vec2(0.17f, 0.015f), vec3(0.10f));            // brow
    g.Ell(hc + vec2(-0.05f, -0.01f), vec2(0.028f, 0.016f), vec3(0.10f));
    g.Ell(hc + vec2(0.05f, -0.01f), vec2(0.028f, 0.016f), vec3(0.10f));
    g.Rect(hc + vec2(0.0f, -0.08f), vec2(0.07f, 0.012f), vec3(0.35f, 0.28f, 0.20f));
    g.Tri(hc + vec2(-0.03f, 0.16f), hc + vec2(0.03f, 0.16f), hc + vec2(0.0f, 0.27f), GOLD);    // uraeus
    g.Ell(hc + vec2(0.0f, 0.215f), vec2(0.014f), vec3(0.85f, 0.12f, 0.12f));
    g.Rect(hc + vec2(0.0f, -0.20f), vec2(0.06f, 0.15f), LAPIS);                   // beard
    g.Rect(hc + vec2(0.0f, -0.20f), vec2(0.06f, 0.015f), GOLD);

    // tower shield in front, with a gold ankh and carved glyphs
    vec2 sc(0.40f, 0.62f);
    g.Rect(sc, vec2(0.24f, 1.02f), vec3(0.22f, 0.20f, 0.20f));
    g.Rect(sc, vec2(0.20f, 0.98f), GRANITE);
    g.Rect(sc, vec2(0.14f, 0.88f), vec3(0.47f, 0.44f, 0.42f));
    g.Tri(sc + vec2(-0.12f, 0.51f), sc + vec2(0.12f, 0.51f), sc + vec2(0.0f, 0.67f), vec3(0.22f, 0.20f, 0.20f));
    g.Tri(sc + vec2(-0.10f, 0.51f), sc + vec2(0.10f, 0.51f), sc + vec2(0.0f, 0.62f), GRANITE);
    g.Rect(sc + vec2(0.0f, -0.05f), vec2(0.028f, 0.30f), GOLD);
    g.Rect(sc + vec2(0.0f, 0.04f), vec2(0.16f, 0.028f), GOLD);
    g.Ell(sc + vec2(0.0f, 0.20f), vec2(0.055f, 0.07f), GOLD);
    g.Ell(sc + vec2(0.0f, 0.20f), vec2(0.03f, 0.042f), vec3(0.47f, 0.44f, 0.42f));
    for (int i = 0; i < 3; ++i) {
        g.Rect(sc + vec2(-0.045f, -0.28f + 0.075f * i), vec2(0.03f, 0.04f), vec3(0.30f, 0.28f, 0.27f));
        g.Rect(sc + vec2(0.045f, -0.28f + 0.075f * i), vec2(0.03f, 0.03f), vec3(0.30f, 0.28f, 0.27f));
    }

    // cracks grow as the statue is damaged (fx.red is damage taken, 0..1)
    if (p.fx.red > 0.25f) {
        vec3 crack(0.12f, 0.08f, 0.06f);
        g.Line(vec2(-0.20f, 0.95f), vec2(-0.12f, 0.80f), 0.02f, crack);
        g.Line(vec2(-0.12f, 0.80f), vec2(-0.20f, 0.66f), 0.02f, crack);
    }
    if (p.fx.red > 0.55f) {
        vec3 crack(0.12f, 0.08f, 0.06f);
        g.Line(vec2(0.10f, 1.00f), vec2(0.02f, 0.84f), 0.02f, crack);
        g.Line(vec2(0.02f, 0.84f), vec2(0.12f, 0.70f), 0.02f, crack);
        g.Line(vec2(0.10f, 0.40f), vec2(0.0f, 0.28f), 0.02f, crack);
    }
}

// ===========================================================================
//  ENEMIES  (the cursed army of Set; they face left, so the pose's facing is -1)
// ===========================================================================

void Draw::Vulture(const Pose& p)
{
    Pen g(p);
    float flap = std::sin(p.time * 9.0f);
    float bob = std::sin(p.time * 3.0f) * 0.04f;
    float y0 = 0.98f + bob;

    const vec3 PLUM(0.22f, 0.12f, 0.30f), PLUM_L(0.34f, 0.18f, 0.44f), PINK(0.76f, 0.40f, 0.42f);
    const vec3 BEAK(0.97f, 0.78f, 0.16f);

    // a faint shadow on the ground far below shows how high it hovers
    if (p.fx.alpha > 0.5f) Draw::Shadow(vec2(p.foot.x, p.foot.y - Draw::VULTURE_HOVER * p.scale), 0.26f * p.scale, 0.22f * p.fx.alpha);

    // far wing (behind the body)
    g.Tri(vec2(-0.02f, y0 + 0.08f), vec2(-0.62f, y0 + 0.40f * flap + 0.18f), vec2(-0.40f, y0 - 0.10f), PLUM);
    for (int i = 0; i < 3; ++i)
        g.Line(vec2(-0.02f, y0 + 0.06f), vec2(-0.62f + 0.08f * i, y0 + 0.40f * flap + 0.18f - 0.10f * i), 0.03f, vec3(0.12f, 0.06f, 0.18f));

    // tail feathers
    g.Tri(vec2(-0.22f, y0 + 0.04f), vec2(-0.62f, y0 + 0.14f), vec2(-0.55f, y0 - 0.02f), PLUM);
    g.Tri(vec2(-0.22f, y0 - 0.02f), vec2(-0.58f, y0 - 0.12f), vec2(-0.48f, y0 - 0.20f), PLUM_L);

    // talons
    for (int side = 0; side < 2; ++side) {
        float x = -0.02f + 0.12f * side;
        g.Line(vec2(x, y0 - 0.14f), vec2(x + 0.04f, y0 - 0.38f), 0.04f, vec3(0.55f, 0.50f, 0.30f));
        g.Tri(vec2(x + 0.04f, y0 - 0.36f), vec2(x - 0.04f, y0 - 0.44f), vec2(x + 0.12f, y0 - 0.44f), vec3(0.25f, 0.22f, 0.15f));
    }

    // body, white ruff, neck, head
    g.Ell(vec2(0.0f, y0), vec2(0.31f, 0.21f), PLUM);
    g.Ell(vec2(0.04f, y0 + 0.03f), vec2(0.22f, 0.12f), PLUM_L);
    g.Ell(vec2(0.22f, y0 - 0.02f), vec2(0.14f, 0.15f), vec3(0.93f, 0.89f, 0.80f));
    vec2 head = g.Limb(vec2(0.24f, y0 + 0.08f), 0.24f, 0.09f, 0.95f, PINK);
    g.Ell(head, vec2(0.105f, 0.095f), PINK);
    g.Tri(head + vec2(0.06f, 0.05f), head + vec2(0.07f, -0.04f), head + vec2(0.30f, -0.09f), BEAK);      // hooked beak
    g.Tri(head + vec2(0.24f, -0.06f), head + vec2(0.31f, -0.10f), head + vec2(0.26f, -0.15f), vec3(0.45f, 0.32f, 0.08f));
    g.Ell(head + vec2(0.03f, 0.03f), vec2(0.045f), vec3(0.95f, 0.15f, 0.08f));            // glowing red eye
    g.Ell(head + vec2(0.04f, 0.03f), vec2(0.018f), vec3(1.0f, 0.9f, 0.5f));
    g.Tri(head + vec2(-0.04f, 0.09f), head + vec2(0.02f, 0.10f), head + vec2(-0.08f, 0.22f), PLUM);   // crest

    // near wing
    g.Tri(vec2(0.05f, y0 + 0.08f), vec2(-0.40f, y0 + 0.46f * flap + 0.24f), vec2(-0.16f, y0 - 0.14f), PLUM_L);
    for (int i = 0; i < 4; ++i)
        g.Line(vec2(0.05f, y0 + 0.06f), vec2(-0.40f + 0.06f * i, y0 + 0.46f * flap + 0.24f - 0.10f * i), 0.025f, PLUM);

    // cast-magic orb charging before it throws
    if (p.attack >= 0.0f) {
        float r = 0.05f + 0.14f * p.attack;
        g.Ell(head + vec2(0.34f, -0.12f), vec2(r * 1.5f), vec3(0.55f, 0.15f, 0.85f));
        g.Ell(head + vec2(0.34f, -0.12f), vec2(r), vec3(0.90f, 0.65f, 1.0f));
    }
}

void Draw::Scorpion(const Pose& p)
{
    Pen g(p);
    float mv = p.moving;
    float bob = std::fabs(std::sin(p.walk)) * 0.02f * mv;
    float k = p.attack < 0.0f ? 0.0f : EaseOut(p.attack);
    if (p.fx.alpha > 0.5f) Draw::Shadow(p.foot, 0.46f * p.scale, 0.35f * p.fx.alpha);

    const vec3 SHELL(0.48f, 0.12f, 0.10f), SHELL_L(0.64f, 0.20f, 0.13f), SHELL_D(0.26f, 0.06f, 0.05f);
    const vec3 GLOW(0.55f, 1.0f, 0.20f);

    // eight legs
    for (int i = 0; i < 4; ++i) {
        float x = -0.20f + 0.15f * static_cast<float>(i);
        float ph = std::sin(p.walk * 1.5f + 1.3f * static_cast<float>(i)) * 0.35f * mv;
        for (int side = 0; side < 2; ++side) {
            float dir = side == 0 ? 1.0f : -1.0f;
            vec2 knee = g.Limb(vec2(x, 0.27f + bob), 0.20f, 0.05f, -0.45f + ph * dir + (side == 0 ? 0.0f : 0.25f), SHELL_D);
            g.Limb(knee, 0.20f, 0.04f, -1.45f + ph * dir * 0.5f, SHELL_D);
        }
    }

    // tail: a chain of segments arcing up and over the back
    vec2 seg[6] = { vec2(-0.30f, 0.32f), vec2(-0.47f, 0.50f), vec2(-0.50f, 0.74f),
                    vec2(-0.37f, 0.96f), vec2(-0.14f, 1.04f), vec2(0.08f, 0.99f) };
    float sway = std::sin(p.time * 3.0f) * 0.02f;
    seg[5] += vec2(0.28f, -0.30f) * k;
    seg[4] += vec2(0.14f, -0.10f) * k;
    for (int i = 0; i < 6; ++i) {
        seg[i].y += sway * static_cast<float>(i);
        float r = 0.115f - 0.012f * static_cast<float>(i);
        g.Ell(seg[i] + vec2(0.0f, bob), vec2(r), (i % 2) ? SHELL_L : SHELL);
    }
    vec2 tip = seg[5] + vec2(0.0f, bob);
    g.Tri(tip + vec2(0.0f, 0.08f), tip + vec2(0.0f, -0.06f), tip + vec2(0.26f, -0.15f), vec3(0.12f, 0.04f, 0.03f));
    g.Ell(tip + vec2(0.09f, -0.04f), vec2(0.045f + 0.05f * k), GLOW);

    // body plates
    g.Ell(vec2(0.0f, 0.34f + bob), vec2(0.36f, 0.18f), SHELL);
    g.Ell(vec2(-0.10f, 0.39f + bob), vec2(0.22f, 0.12f), SHELL_L);
    for (int i = 0; i < 4; ++i)
        g.Rect(vec2(-0.18f + 0.10f * i, 0.36f + bob), vec2(0.018f, 0.24f), SHELL_D, 0.2f);
    g.Ell(vec2(0.24f, 0.33f + bob), vec2(0.17f, 0.14f), SHELL_D);

    // pincers
    float open = 0.22f + std::sin(p.time * 4.0f) * 0.10f;
    for (int arm = 0; arm < 2; ++arm) {
        float yo = arm == 0 ? 0.10f : -0.04f;
        vec2 el = g.Limb(vec2(0.34f, 0.34f + yo + bob), 0.17f, 0.07f, 0.35f + 0.1f * arm, SHELL);
        vec2 wr = g.Limb(el, 0.15f, 0.07f, 0.05f - 0.2f * arm, SHELL);
        g.Tri(wr + vec2(0.0f, 0.06f), wr + vec2(0.0f, -0.02f), wr + vec2(0.20f, open * 0.5f), SHELL_L);
        g.Tri(wr + vec2(0.0f, 0.02f), wr + vec2(0.0f, -0.06f), wr + vec2(0.20f, -open * 0.5f), SHELL_L);
    }

    // glowing eyes
    g.Ell(vec2(0.32f, 0.42f + bob), vec2(0.035f), GLOW);
    g.Ell(vec2(0.27f, 0.45f + bob), vec2(0.03f), GLOW);
}

void Draw::Mummy(const Pose& p)
{
    Pen g(p);
    float sw = std::sin(p.walk) * p.moving;
    float bob = std::fabs(std::sin(p.walk)) * 0.05f * p.moving;
    float lean = 0.10f * p.moving;
    if (p.fx.alpha > 0.5f) Draw::Shadow(p.foot, 0.36f * p.scale, 0.36f * p.fx.alpha);

    const vec3 WRAP(0.86f, 0.80f, 0.64f), WRAP_D(0.58f, 0.52f, 0.40f), RAG(0.70f, 0.62f, 0.46f);

    // trailing rags behind the brute
    g.Tri(vec2(-0.10f, 0.72f + bob), vec2(-0.45f - sw * 0.05f, 0.58f), vec2(-0.12f, 0.48f), RAG);
    g.Tri(vec2(-0.05f, 0.66f + bob), vec2(-0.40f, 0.34f), vec2(0.0f, 0.42f), WRAP_D);

    // legs
    for (int side = -1; side <= 1; side += 2) {
        float sgn = static_cast<float>(side);
        float ang = -PI * 0.5f + sgn * sw * 0.75f + sgn * 0.12f * p.moving;
        vec2 hip(0.06f * sgn, 0.54f + bob);
        vec2 end = g.Limb(hip, 0.50f, 0.17f, ang, WRAP);
        g.Bands(hip, 0.50f, 0.17f, ang, WRAP_D, 4);
        g.Rect(end + vec2(0.07f, 0.02f), vec2(0.24f, 0.08f), WRAP_D);
    }

    // torso with diagonal wraps, a rotten hole and a scarab amulet
    g.Rect(vec2(lean * 0.5f, 0.90f + bob), vec2(0.50f, 0.50f), WRAP);
    g.Rect(vec2(lean * 0.5f, 0.68f + bob), vec2(0.54f, 0.07f), WRAP_D);
    for (int i = 0; i < 4; ++i)
        g.Rect(vec2(lean * 0.5f, 0.76f + 0.11f * i + bob), vec2(0.56f, 0.04f), WRAP_D, 0.45f);
    g.Ell(vec2(lean * 0.5f - 0.10f, 0.95f + bob), vec2(0.08f, 0.10f), vec3(0.10f, 0.14f, 0.08f));
    g.Ell(vec2(lean * 0.5f + 0.10f, 0.84f + bob), vec2(0.06f, 0.05f), GOLD);
    g.Ell(vec2(lean * 0.5f + 0.10f, 0.84f + bob), vec2(0.03f, 0.025f), vec3(0.10f, 0.45f, 0.45f));

    // head: wraps, dark gap with glowing eyes, Set-animal ears
    vec2 hc(0.12f + lean, 1.32f + bob);
    g.Tri(hc + vec2(-0.05f, 0.12f), hc + vec2(0.10f, 0.12f), hc + vec2(0.03f, 0.42f), vec3(0.14f, 0.10f, 0.12f));
    g.Tri(hc + vec2(-0.12f, 0.10f), hc + vec2(-0.02f, 0.14f), hc + vec2(-0.12f, 0.36f), vec3(0.14f, 0.10f, 0.12f));
    g.Tri(hc + vec2(-0.08f, 0.16f), hc + vec2(0.01f, 0.16f), hc + vec2(-0.10f, 0.28f), vec3(0.45f, 0.10f, 0.10f));
    g.Tri(hc + vec2(-0.12f, 0.12f), hc + vec2(-0.12f, 0.02f), hc + vec2(-0.38f, 0.04f + sw * 0.05f), RAG);   // trailing strip
    g.Ell(hc, vec2(0.16f, 0.17f), WRAP);
    g.Rect(hc + vec2(0.06f, -0.02f), vec2(0.17f, 0.075f), vec3(0.10f, 0.04f, 0.04f));
    g.Ell(hc + vec2(0.09f, -0.01f), vec2(0.045f), vec3(1.0f, 0.15f, 0.08f));
    g.Ell(hc + vec2(0.09f, -0.01f), vec2(0.018f), vec3(1.0f, 0.9f, 0.5f));
    for (int i = 0; i < 3; ++i)
        g.Rect(hc + vec2(0.0f, -0.10f + 0.08f * i), vec2(0.34f, 0.025f), WRAP_D, 0.25f);

    // arms: swing while running, slam down when attacking
    for (int side = 0; side < 2; ++side) {
        float sgn = side == 0 ? 1.0f : -1.0f;
        vec2 sh(0.18f + lean * 0.7f - 0.04f * side, 1.10f + bob);
        float a1 = p.attack < 0.0f ? (-0.95f + sgn * sw * 0.85f) : Lerp(1.45f, -1.25f, EaseOut(p.attack));
        vec2 el = g.Limb(sh, 0.40f, 0.18f, a1, WRAP);
        g.Bands(sh, 0.40f, 0.18f, a1, WRAP_D, 3);
        float a2 = a1 + (p.attack < 0.0f ? 0.5f : 0.3f);
        vec2 fist = g.Limb(el, 0.32f, 0.16f, a2, WRAP);
        g.Ell(fist, vec2(0.13f), WRAP);
        g.Ell(fist + vec2(0.02f, 0.0f), vec2(0.07f), WRAP_D);
    }
}
