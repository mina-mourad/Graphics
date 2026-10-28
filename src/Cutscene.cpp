#include "Cutscene.h"
#include "GameConfig.h"
#include "Renderer2D.h"
#include "Text.h"
#include "Draw.h"

#include <algorithm>
#include <cmath>
#include <string>

using glm::vec2;
using glm::vec3;

namespace
{
    float T = 0.0f;

    float Clamp01(float v) { return std::min(1.0f, std::max(0.0f, v)); }
    float Smooth(float t) { t = Clamp01(t); return t * t * (3.0f - 2.0f * t); }
    // progress 0..1 of time `t` inside [a, b]
    float Between(float t, float a, float b) { return Clamp01((t - a) / (b - a)); }
    float Mix(float a, float b, float k) { return a + (b - a) * k; }

    void Bubble(vec2 tail, const std::string& line1, const std::string& line2, float alpha)
    {
        float px = 0.06f;
        float w = std::max(Text::Width(line1, px), Text::Width(line2, px)) + 0.8f;
        vec2 c = tail + vec2(0.0f, 0.95f);
        Draw::Fx fx; fx.alpha = alpha;
        Renderer2D::DrawTriangle(tail + vec2(-0.15f, 0.45f), tail + vec2(0.15f, 0.45f), tail + vec2(0.0f, 0.05f), vec3(0.30f, 0.20f, 0.10f), alpha);
        Draw::Plaque(c, vec2(w, 1.15f), 0.0f, fx);
        Text::Draw(line1, c + vec2(0.0f, 0.10f), px, vec3(0.12f, 0.06f, 0.02f), Text::Align::Center, alpha);
        Text::Draw(line2, c + vec2(0.0f, -0.40f), px, vec3(0.12f, 0.06f, 0.02f), Text::Align::Center, alpha);
    }

    // Narration in the lower letterbox bar.
    void Subtitle(const std::string& s, float a, float b)
    {
        if (T < a || T > b) return;
        float alpha = std::min(Between(T, a, a + 0.4f), 1.0f - Between(T, b - 0.4f, b));
        Text::DrawShadow(s, vec2(WORLD_WIDTH * 0.5f, 0.2f), 0.05f, vec3(1.0f, 0.94f, 0.75f), Text::Align::Center, alpha);
    }
}

void Cutscene::Start() { T = 0.0f; }
void Cutscene::Update(float dt) { T += dt; }
bool Cutscene::IsFinished() { return T >= DURATION; }
float Cutscene::Time() { return T; }

void Cutscene::Render()
{
    // ---- the world --------------------------------------------------------
    Draw::Sky(T);
    Draw::Foreground(T);
    for (int l = 0; l < NUM_LANES; ++l) Draw::LaneFloor(l, 0.0f, false);
    Draw::HomeBase(T);

    // ---- actor timelines --------------------------------------------------
    // Hero (Medjay guard): stands at the temple gate, steps forward to answer,
    // swings his khopesh, then charges at the end.
    float heroX = 3.1f;
    float heroMove = 0.0f;
    if (T > 8.4f && T < 10.4f) { heroX = Mix(3.1f, 4.4f, Smooth(Between(T, 8.4f, 10.4f))); heroMove = 1.0f; }
    else if (T >= 10.4f)       { heroX = 4.4f; }
    if (T > 13.4f)             { heroX = Mix(4.4f, 7.2f, Smooth(Between(T, 13.4f, 15.4f))); heroMove = 1.0f; }

    float heroAtk = -1.0f;
    if (T > 10.9f && T < 13.4f) heroAtk = std::fmod((T - 10.9f) * 1.5f, 1.0f);
    if (T >= 13.4f)             heroAtk = std::fmod((T - 13.4f) * 2.2f, 1.0f);

    // Mummy brute: marches in, taunts, then charges.
    float mummyX = 11.4f, mummyMove = 0.0f;
    if (T < 7.4f)       { mummyX = Mix(18.2f, 11.4f, Smooth(Between(T, 4.2f, 7.4f))); mummyMove = (T > 4.2f) ? 1.0f : 0.0f; }
    else if (T > 13.6f) { mummyX = Mix(11.4f, 8.6f, Smooth(Between(T, 13.6f, 15.4f))); mummyMove = 1.0f; }
    float mummyAtk = (T > 7.6f && T < 10.4f) ? std::fmod((T - 7.6f) * 0.9f, 1.0f) : -1.0f;

    float scorpX = (T < 7.8f) ? Mix(19.5f, 13.4f, Smooth(Between(T, 4.6f, 7.8f))) : 13.4f;
    float scorpMove = (T > 4.6f && T < 7.8f) ? 1.0f : 0.0f;
    float scorpAtk = (T > 12.0f) ? std::fmod((T - 12.0f) * 0.8f, 1.0f) : -1.0f;

    float vultX = (T < 7.4f) ? Mix(19.0f, 9.6f, Smooth(Between(T, 4.0f, 7.4f))) : 9.6f;
    float vultAtk = (T > 12.8f && T < 14.1f) ? Between(T, 12.8f, 14.0f) : -1.0f;

    // Archer ally behind the hero.
    float archAtk = -1.0f;
    if (T > 10.9f) archAtk = std::fmod((T - 10.9f) * 1.4f, 1.0f);

    // ---- hit flashes caused by the arrows ----
    float arrowFire1 = 11.4f, arrowFire2 = 13.0f, arrowSpeed = 10.0f;
    float arrowStartX = 2.7f;
    float scorpHitT = arrowFire1 + (scorpX - arrowStartX) / arrowSpeed;
    float mummyHitT = arrowFire2 + (mummyX - arrowStartX) / arrowSpeed;
    float scorpFlash = (T > scorpHitT && T < scorpHitT + 0.4f) ? 1.0f - Between(T, scorpHitT, scorpHitT + 0.4f) : 0.0f;
    float mummyFlash = (T > mummyHitT && T < mummyHitT + 0.4f) ? 1.0f - Between(T, mummyHitT, mummyHitT + 0.4f) : 0.0f;
    float boltT = 14.1f;
    float heroFlash = (T > boltT && T < boltT + 0.4f) ? 1.0f - Between(T, boltT, boltT + 0.4f) : 0.0f;

    // ---- draw actors back to front (higher on screen = further away) ------
    const float S = 1.65f;
    {   // vulture (flying, highest)
        Draw::Pose p; p.foot = vec2(vultX, 3.75f); p.facing = -1.0f; p.scale = 1.35f; p.time = T; p.attack = vultAtk;
        Draw::Vulture(p);
    }
    {   // scorpion
        Draw::Pose p; p.foot = vec2(scorpX, 3.0f); p.facing = -1.0f; p.scale = 1.5f; p.time = T; p.walk = T * 8.0f; p.moving = scorpMove; p.attack = scorpAtk;
        p.fx.flash = scorpFlash * 0.6f; p.fx.red = scorpFlash * 0.4f;
        Draw::Scorpion(p);
    }
    {   // mummy brute
        Draw::Pose p; p.foot = vec2(mummyX, 2.35f); p.facing = -1.0f; p.scale = S; p.time = T; p.walk = T * 6.0f; p.moving = mummyMove; p.attack = mummyAtk;
        p.fx.flash = mummyFlash * 0.6f; p.fx.red = mummyFlash * 0.4f;
        Draw::Mummy(p);
    }
    {   // archer ally
        Draw::Pose p; p.foot = vec2(2.55f, 2.25f); p.facing = 1.0f; p.scale = 1.5f; p.time = T; p.attack = archAtk;
        Draw::Archer(p);
    }
    {   // hero
        Draw::Pose p; p.foot = vec2(heroX, 1.75f); p.facing = 1.0f; p.scale = S; p.time = T; p.walk = T * 8.0f; p.moving = heroMove; p.attack = heroAtk;
        p.fx.flash = heroFlash * 0.6f;
        Draw::Melee(p);
    }

    // ---- projectiles ------------------------------------------------------
    for (int shot = 0; shot < 2; ++shot) {
        float fire = shot == 0 ? arrowFire1 : arrowFire2;
        float targetX = shot == 0 ? scorpX : mummyX;
        float targetY = shot == 0 ? 3.9f : 3.85f;
        float hit = fire + (targetX - arrowStartX) / arrowSpeed;
        if (T > fire && T < hit) {
            float k = (T - fire) / (hit - fire);
            vec2 pos(Mix(arrowStartX, targetX, k), Mix(3.2f, targetY, k));
            Draw::Arrow(pos, T);
        }
        if (T >= hit && T < hit + 0.35f) Draw::HitSpark(vec2(targetX - 0.2f, targetY), (T - hit) / 0.35f, vec3(1.0f, 0.85f, 0.3f));
    }
    if (T > 13.5f && T < boltT) {                          // the vulture's spirit bolt flying at the hero
        float k = Between(T, 13.5f, boltT);
        Draw::SpiritBolt(vec2(Mix(vultX - 0.9f, heroX + 0.7f, k), Mix(4.5f, 3.4f, k)), T);
    }
    if (T >= boltT && T < boltT + 0.4f) Draw::HitSpark(vec2(heroX + 0.7f, 3.4f), (T - boltT) / 0.4f, vec3(0.8f, 0.4f, 1.0f));

    // ---- dialogue ---------------------------------------------------------
    if (T > 7.6f && T < 10.4f) {
        float a = std::min(Between(T, 7.6f, 8.0f), 1.0f - Between(T, 10.0f, 10.4f));
        Bubble(vec2(mummyX - 0.2f, 4.85f), "GIVE US THE SUN DISC,", "GUARDIAN!", a);
    }
    if (T > 10.6f && T < 13.4f) {
        float a = std::min(Between(T, 10.6f, 11.0f), 1.0f - Between(T, 13.0f, 13.4f));
        Bubble(vec2(heroX + 0.4f, 4.6f), "NEVER! NOT WHILE A", "MEDJAY STANDS!", a);
    }

    // ---- cinematic bars, narration and fades ------------------------------
    Renderer2D::DrawQuadRot(vec2(WORLD_WIDTH * 0.5f, WORLD_HEIGHT - 0.35f), vec2(WORLD_WIDTH, 0.7f), 0.0f, vec3(0.02f, 0.01f, 0.02f));
    Renderer2D::DrawQuadRot(vec2(WORLD_WIDTH * 0.5f, 0.35f), vec2(WORLD_WIDTH, 0.7f), 0.0f, vec3(0.02f, 0.01f, 0.02f));

    Subtitle("THE GOLDEN SUN DISC OF RA BRINGS DAWN TO EGYPT.", 0.8f, 4.2f);
    Subtitle("BUT THE CURSED ARMY OF SET RISES FROM THE SANDS...", 4.3f, 7.5f);
    Subtitle("THE MEDJAY GUARDIANS ANSWER THE CALL.", 10.5f, 13.4f);

    if (T > 13.6f) {
        float k = Between(T, 13.6f, 14.1f);
        float pulse = 1.0f + 0.04f * std::sin(T * 14.0f);
        Text::DrawShadow("DEFEND THE SUN TEMPLE!", vec2(WORLD_WIDTH * 0.5f, 7.35f), 0.10f * pulse, vec3(1.0f, 0.86f, 0.30f), Text::Align::Center, k);
    }
    Text::DrawShadow("PRESS SPACE TO SKIP", vec2(WORLD_WIDTH - 0.3f, 8.55f), 0.035f, vec3(0.9f, 0.85f, 0.7f), Text::Align::Right, 0.7f);

    float fade = 1.0f - Between(T, 0.0f, 1.0f);                  // fade in from black
    fade = std::max(fade, Between(T, DURATION - 0.9f, DURATION)); // fade out to black
    if (fade > 0.0f)
        Renderer2D::DrawQuadRot(vec2(WORLD_WIDTH * 0.5f, WORLD_HEIGHT * 0.5f), vec2(WORLD_WIDTH, WORLD_HEIGHT), 0.0f, vec3(0.0f), fade);
}
