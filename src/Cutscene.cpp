#include "Cutscene.h"
#include "GameConfig.h"
#include "Renderer2D.h"
#include "CharacterRenderer.h"
#include "Environment.h"
#include "Hud.h"
#include "Audio.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace
{
    using glm::vec2;
    using glm::vec3;
    using CharacterRenderer::EnemyAnim;
    using CharacterRenderer::PlayerAnim;

    float T = 0.0f;
    float prevT = 0.0f;

    float Clamp01(float v) { return std::min(1.0f, std::max(0.0f, v)); }
    float Smooth(float t) { t = Clamp01(t); return t * t * (3.0f - 2.0f * t); }
    float Between(float t, float a, float b) { return Clamp01((t - a) / (b - a)); }   // 0..1 inside [a, b]
    float Lerp(float a, float b, float k) { return a + (b - a) * k; }
    bool Crossed(float at) { return prevT < at && T >= at; }

    // Speech bubble with a tail pointing at the speaker.
    void Bubble(vec2 tail, const std::string& line1, const std::string& line2, vec2 offset)
    {
        const float s = 0.11f;
        float w = std::max(Renderer2D::TextWidth(line1, s), Renderer2D::TextWidth(line2, s)) + 0.6f;
        vec2 c = tail + offset;
        c.x = std::min(WORLD_WIDTH - w * 0.5f - 0.1f, std::max(w * 0.5f + 0.1f, c.x));
        const vec3 ink(0.12f, 0.07f, 0.04f);
        Renderer2D::DrawTriangle(tail, vec2(c.x - 0.25f, c.y - 0.3f), vec2(c.x + 0.15f, c.y - 0.3f), ink);
        Renderer2D::DrawQuad(c, vec2(w + 0.08f, 1.0f), ink);
        Renderer2D::DrawQuad(c, vec2(w, 0.92f), vec3(1.0f, 0.98f, 0.92f));
        Renderer2D::DrawTriangle(tail + vec2(0.03f, 0.08f), vec2(c.x - 0.2f, c.y - 0.42f), vec2(c.x + 0.1f, c.y - 0.42f), vec3(1.0f, 0.98f, 0.92f));
        Renderer2D::DrawText(line1, c + vec2(0.0f, 0.18f), s, ink);
        Renderer2D::DrawText(line2, c + vec2(0.0f, -0.2f), s, ink);
    }

    // Narration in the black band at the bottom.
    void Subtitle(const std::string& s, float a, float b)
    {
        if (T < a || T > b) return;
        Renderer2D::DrawTextShadow(s, vec2(WORLD_WIDTH * 0.5f, 0.32f), 0.12f, vec3(1.0f, 0.94f, 0.75f));
    }
}

namespace Cutscene
{
    void Start() { T = 0.0f; prevT = -1.0f; }
    bool IsFinished() { return T >= DURATION; }
    float GetTime() { return T; }

    void Update(float dt)
    {
        prevT = T;
        T += dt;

        // sound cues on the timeline
        if (Crossed(0.3f))  Audio::Play(Audio::Sfx::TukTuk);
        if (Crossed(4.3f))  Audio::Play(Audio::Sfx::WaveHorn, 0.6f);
        if (Crossed(8.0f))  Audio::Play(Audio::Sfx::Knife, 0.7f);
        if (Crossed(11.2f)) Audio::Play(Audio::Sfx::SlipperThrow);
        if (Crossed(12.15f)) Audio::Play(Audio::Sfx::Duck);
        if (Crossed(12.3f)) Audio::Play(Audio::Sfx::Slingshot);
        if (Crossed(12.75f)) Audio::Play(Audio::Sfx::Jump);
        if (Crossed(13.1f)) Audio::Play(Audio::Sfx::SlipperHit, 0.6f);
        if (Crossed(13.25f)) Audio::Play(Audio::Sfx::TomatoThrow);
        if (Crossed(14.0f)) Audio::Play(Audio::Sfx::TomatoSplat);
        if (Crossed(14.4f)) Audio::Play(Audio::Sfx::StickHit);
    }

    void Render()
    {
        // fade in from black, fade out to black (mix toward black like u_Highlight)
        float fade = std::max(1.0f - Between(T, 0.0f, 1.0f), Between(T, DURATION - 0.9f, DURATION));
        Renderer2D::SetFade(fade);

        Environment::DrawBackground(T);
        for (int lane = NUM_LANES - 1; lane >= 0; --lane)
            Environment::DrawLane(lane, 0.0f);
        Environment::DrawHomeBase(T);

        const float L0 = LANE_Y[0], L1 = LANE_Y[1], L2 = LANE_Y[2];

        // ---------------- lane 2 (back): El Me'allem comes out with his shield
        if (T > 10.6f)
        {
            float x = Lerp(1.3f, 2.4f, Smooth(Between(T, 10.6f, 11.6f)));
            CharacterRenderer::DrawPlayer(x, L2, PlayerType::Defense, PlayerAnim::Idle, T);
        }

        // ---------------- lane 1 (middle): El Bawab vs the knife boss, slipper thrower behind
        float bawabX = 2.6f;
        PlayerAnim bawabAnim = PlayerAnim::Idle;
        float bawabTime = T;
        if (T > 10.4f && T < 11.6f) { bawabX = Lerp(2.6f, 4.2f, Smooth(Between(T, 10.4f, 11.6f))); bawabAnim = PlayerAnim::Walk; }
        else if (T >= 11.6f) bawabX = 4.2f;
        if (T > 13.9f) { bawabX = Lerp(4.2f, 5.6f, Smooth(Between(T, 13.9f, 14.3f))); bawabAnim = PlayerAnim::Attack; bawabTime = std::fmod(T - 13.9f, 0.7f); }
        float bawabDuck = (T > 12.1f && T < 12.95f) ? std::min(1.0f, std::min((T - 12.1f) * 6.0f, (12.95f - T) * 6.0f)) : 0.0f;

        float bossX = (T < 7.4f) ? Lerp(17.5f, 7.0f, Smooth(Between(T, 4.2f, 7.4f))) : 7.0f;
        EnemyAnim bossAnim = (T > 4.2f && T < 7.4f) ? EnemyAnim::Run : EnemyAnim::Idle;
        float bossTime = T;
        if (T > 7.6f && T < 8.5f) { bossAnim = EnemyAnim::Attack; bossTime = T - 7.6f; }   // waves the knife
        if (T > 14.3f) bossX = Lerp(7.0f, 6.6f, Smooth(Between(T, 14.3f, 14.6f)));

        float slipperX = (T < 7.8f) ? Lerp(19.0f, 9.6f, Smooth(Between(T, 4.6f, 7.8f))) : 9.6f;
        EnemyAnim slipperAnim = (T > 4.6f && T < 7.8f) ? EnemyAnim::Run : EnemyAnim::Idle;
        float slipperTime = T;
        if (T > 10.7f && T < 11.6f) { slipperAnim = EnemyAnim::Attack; slipperTime = T - 10.7f; }

        CharacterRenderer::Draw(slipperX, L1, EnemyType::EasyRanged, slipperAnim, slipperTime);
        float bossHit = (T > 14.35f && T < 14.7f) ? 0.7f : 0.0f;
        Renderer2D::SetDamage(bossHit);
        CharacterRenderer::Draw(bossX, L1, EnemyType::HardMelee, bossAnim, bossTime);
        Renderer2D::SetDamage(0.0f);
        CharacterRenderer::DrawPlayer(bawabX, L1, PlayerType::Melee, bawabAnim, bawabTime, 0.0f, bawabDuck);

        // the slipper flies at head height... El Bawab ducks, it hits the koshk
        if (T > 11.2f && T < 13.1f)
        {
            float k = Between(T, 11.2f, 13.1f);
            float x = Lerp(slipperX - 0.5f, HOME_BASE_RIGHT, k);
            CharacterRenderer::DrawSlipper(x, L1 + 0.62f + 0.2f * (1.0f - k), T * 14.0f);
        }

        // ---------------- lane 0 (front): the tomato seller vs the slingshot kid
        float vendorX = (T < 10.8f) ? 2.3f : Lerp(2.3f, 3.6f, Smooth(Between(T, 10.8f, 11.8f)));
        PlayerAnim vendorAnim = (T > 10.8f && T < 11.8f) ? PlayerAnim::Walk : PlayerAnim::Idle;
        float vendorTime = T;
        if (T > 12.9f && T < 13.6f) { vendorAnim = PlayerAnim::Attack; vendorTime = T - 12.9f; }
        float vendorJump = 0.0f;
        if (T > 12.75f && T < 13.35f) vendorJump = 0.7f * std::sin(3.14159f * Between(T, 12.75f, 13.35f));

        float kidX = (T < 8.0f) ? Lerp(18.0f, 10.4f, Smooth(Between(T, 5.0f, 8.0f))) : 10.4f;
        EnemyAnim kidAnim = (T > 5.0f && T < 8.0f) ? EnemyAnim::Run : EnemyAnim::Idle;
        float kidTime = T;
        if (T > 11.8f && T < 12.7f) { kidAnim = EnemyAnim::Attack; kidTime = T - 11.8f; }

        // the passing tuk-tuk at the start
        if (T > 0.2f && T < 4.6f)
            Environment::DrawTukTuk(vec2(Lerp(-1.5f, 18.0f, Between(T, 0.3f, 4.5f)), L0 - FOOT_OFFSET), 1.1f, 1.0f, T);

        float kidHit = (T > 14.0f && T < 14.4f) ? 0.7f : 0.0f;
        Renderer2D::SetDamage(kidHit);
        CharacterRenderer::Draw(kidX, L0, EnemyType::MediumRanged, kidAnim, kidTime);
        Renderer2D::SetDamage(0.0f);
        CharacterRenderer::DrawPlayer(vendorX, L0, PlayerType::Ranged, vendorAnim, vendorTime, vendorJump, 0.0f);

        // the pebble skims the ground... the vendor jumps over it
        if (T > 12.3f && T < 13.8f)
        {
            float k = Between(T, 12.3f, 13.8f);
            CharacterRenderer::DrawPebble(Lerp(kidX - 0.4f, HOME_BASE_RIGHT, k), L0 - 0.42f);
        }
        // the tomato flies and splats on the kid
        if (T > 13.25f && T < 14.0f)
        {
            float k = Between(T, 13.25f, 14.0f);
            CharacterRenderer::DrawTomato(Lerp(vendorX + 0.4f, kidX - 0.2f, k), L0 + 0.5f + 0.5f * std::sin(3.14159f * k), T * 9.0f);
        }
        if (T > 14.0f && T < 14.35f)
            CharacterRenderer::DrawSplat(kidX - 0.2f, L0 + 0.4f, Between(T, 14.0f, 14.35f));

        Environment::DrawForeground(T);

        // ---------------- dialogue
        if (T > 7.6f && T < 10.3f)
            Bubble(vec2(bossX - 0.2f, L1 + 1.0f), "THIS KOSHK IS OURS NOW,", "YA AM SAYED!", vec2(0.2f, 1.1f));
        if (T > 10.5f && T < 13.0f)
            Bubble(vec2(bawabX + 0.2f, L1 + 1.0f), "OVER MY DEAD BODY!", "YA SHABAB, TO THE KOSHK!", vec2(1.6f, 1.1f));

        // ---------------- letterbox bands, narration, title
        Renderer2D::DrawQuad(vec2(WORLD_WIDTH * 0.5f, WORLD_HEIGHT - 0.3f), vec2(WORLD_WIDTH, 0.6f), vec3(0.02f));
        Renderer2D::DrawQuad(vec2(WORLD_WIDTH * 0.5f, 0.3f), vec2(WORLD_WIDTH, 0.6f), vec3(0.02f));

        Subtitle("IN AN OLD CAIRO HARA, AM SAYED'S KOSHK KEEPS THE WHOLE STREET GOING.", 0.8f, 4.2f);
        Subtitle("BUT THE SLIPPER GANG WANTS IT FOR THEMSELVES...", 4.3f, 7.5f);
        Subtitle("THE SHABAB OF THE HARA ANSWER THE CALL.", 10.4f, 12.0f);
        Subtitle("DUCK UNDER SLIPPERS  -  JUMP OVER PEBBLES!", 12.05f, 13.8f);

        if (T > 13.8f)
        {
            float pulse = 1.0f + 0.04f * std::sin(T * 12.0f);
            Hud::DrawPanel(vec2(WORLD_WIDTH * 0.5f, 7.2f), vec2(9.2f, 1.2f));
            Renderer2D::DrawTextShadow("DEFEND THE KOSHK!", vec2(WORLD_WIDTH * 0.5f, 7.2f), 0.3f * pulse, vec3(1.0f, 0.85f, 0.3f));
        }
        Renderer2D::DrawTextAligned("SPACE: SKIP", vec2(WORLD_WIDTH - 0.25f, WORLD_HEIGHT - 0.3f), 0.105f, vec3(0.8f), Renderer2D::Align::Right);

        Renderer2D::SetFade(0.0f);
    }
}
