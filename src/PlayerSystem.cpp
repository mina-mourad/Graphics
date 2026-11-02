#include "PlayerSystem.h"
#include "EnemySystem.h"
#include "Economy.h"
#include "Hud.h"
#include "CharacterRenderer.h"
#include "Renderer2D.h"
#include "HealthBar.h"
#include "Input.h"
#include "Audio.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
    using CharacterRenderer::PlayerAnim;

    // ------------------------------------------------------------------
    // Lab 3 point-in-triangle test, used for the lane hover.
    // ------------------------------------------------------------------
    float triSign(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
    {
        return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
    }

    bool pointInTriangle(glm::vec2 pt, glm::vec2 a, glm::vec2 b, glm::vec2 c)
    {
        float d1 = triSign(pt, a, b);
        float d2 = triSign(pt, b, c);
        float d3 = triSign(pt, c, a);

        bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
        return !(hasNeg && hasPos);
    }

    // ------------------------------------------------------------------
    // Data
    // ------------------------------------------------------------------
    const float MELEE_REACH   = 0.95f;   // the nabboot reaches this far forward
    const float TOMATO_SPEED  = 5.5f;
    const float SPLAT_TIME    = 0.35f;
    const float JUMP_HEIGHT   = 0.75f;

    struct Player
    {
        int id;
        PlayerType type;
        int lane;
        float x;
        int health;
        int maxHealth;

        PlayerAnim anim;
        float animTime;
        float cooldown;
        bool hasHit;
        float flash;     // short flash after a hit
        float jumpT;     // time left in the air
        float duckT;     // time left ducking
        float walkTime;  // walking cycle (keeps going while attacking on the move)
        bool moving;
    };

    struct Tomato
    {
        float x, y;
        int lane;
        float spin;
        float splatT;    // < 0 while flying, then 0 -> SPLAT_TIME
    };

    std::vector<Player> players;
    std::vector<Tomato> tomatoes;
    int nextId = 1;

    int hoveredLane = -1;
    bool laneSelected[NUM_LANES] = {};

    int Index(PlayerType t) { return static_cast<int>(t); }

    Player* FindPlayer(int id)
    {
        for (Player& p : players)
            if (p.id == id) return &p;
        return nullptr;
    }

    // Jump height for a jump that has `t` seconds left (a smooth arc).
    float JumpHeight(const Player& p)
    {
        if (p.jumpT <= 0.0f) return 0.0f;
        float k = 1.0f - p.jumpT / JUMP_TIME;            // 0 -> 1 through the jump
        return JUMP_HEIGHT * std::sin(3.14159f * k);
    }

    float DuckAmount(const Player& p)
    {
        if (p.duckT <= 0.0f) return 0.0f;
        float k = 1.0f - p.duckT / DUCK_TIME;
        return std::min(1.0f, std::min(k * 6.0f, (1.0f - k) * 6.0f));   // quick down, hold, quick up
    }

    void SpawnPlayer(PlayerType type, int lane)
    {
        Player p{};
        p.id = nextId++;
        p.type = type;
        p.lane = lane;
        p.x = LANE_START_X;
        p.maxHealth = p.health = PLAYER_MAX_HP[Index(type)];
        p.anim = (type == PlayerType::Defense) ? PlayerAnim::Idle : PlayerAnim::Walk;
        p.animTime = 0.0f;
        p.cooldown = 0.3f;   // they start attacking right away
        players.push_back(p);
    }

    // The attack itself, at the moment the animation shows it.
    void PerformAttack(Player& p)
    {
        const float laneY = LANE_Y[p.lane];
        if (p.type == PlayerType::Melee)
        {
            glm::vec2 at;
            if (EnemySystem::HitEnemyInRange(p.lane, p.x - 0.2f, p.x + MELEE_REACH, PLAYER_DAMAGE, &at))
            {
                Audio::Play(Audio::Sfx::StickHit, 0.9f);
                Hud::AddSpark(at + glm::vec2(-0.2f, 0.25f), glm::vec3(1.0f, 0.85f, 0.4f));
            }
        }
        else if (p.type == PlayerType::Ranged)
        {
            glm::vec2 start = CharacterRenderer::GetPlayerThrowPoint(p.x, laneY);
            tomatoes.push_back({ start.x, start.y, p.lane, 0.0f, -1.0f });
            Audio::Play(Audio::Sfx::TomatoThrow, 0.6f);
        }
    }

    void UpdatePlayer(Player& p, float dt)
    {
        p.flash = std::max(0.0f, p.flash - dt * 4.0f);
        p.jumpT = std::max(0.0f, p.jumpT - dt);
        p.duckT = std::max(0.0f, p.duckT - dt);

        if (p.type == PlayerType::Defense)
        {
            // Stationary blocker: never moves, never attacks.
            p.animTime += dt;
            return;
        }

        p.cooldown -= dt;
        const bool touching = EnemySystem::EnemyInContact(p.lane, p.x);

        switch (p.anim)
        {
        case PlayerAnim::Walk:
        case PlayerAnim::Idle:
            if (p.cooldown <= 0.0f)
            {
                // Players attack from the moment they are placed, whether or
                // not an enemy is in front of them.
                p.anim = PlayerAnim::Attack;
                p.animTime = 0.0f;
                p.hasHit = false;
                if (p.type == PlayerType::Melee)
                    Audio::Play(Audio::Sfx::Swing, 0.35f);
            }
            else if (touching)
            {
                // Met an enemy: stop and keep fighting.
                if (p.anim != PlayerAnim::Idle) { p.anim = PlayerAnim::Idle; p.animTime = 0.0f; }
            }
            else
            {
                if (p.anim != PlayerAnim::Walk) { p.anim = PlayerAnim::Walk; p.animTime = p.walkTime; }
                p.x += PLAYER_SPEED[Index(p.type)] * dt;
            }
            break;

        case PlayerAnim::Attack:
            // keep advancing while swinging / throwing, unless an enemy blocks the way
            if (!touching)
                p.x += PLAYER_SPEED[Index(p.type)] * dt;
            if (!p.hasHit && p.animTime >= CharacterRenderer::PLAYER_HIT_TIME)
            {
                p.hasHit = true;
                PerformAttack(p);
            }
            if (p.animTime >= CharacterRenderer::PLAYER_ATTACK_DURATION)
            {
                p.cooldown = PLAYER_COOLDOWN[Index(p.type)];
                p.anim = touching ? PlayerAnim::Idle : PlayerAnim::Walk;
                p.animTime = touching ? 0.0f : p.walkTime;
            }
            break;
        }

        p.animTime += dt;
        p.moving = !touching;
        if (p.moving)
            p.walkTime += dt;
    }

    void UpdateTomatoes(float dt)
    {
        for (Tomato& t : tomatoes)
        {
            if (t.splatT >= 0.0f)
            {
                t.splatT += dt;
                continue;
            }
            t.x += TOMATO_SPEED * dt;
            t.y += (LANE_Y[t.lane] + 0.25f - t.y) * std::min(1.0f, dt * 3.0f);   // gentle arc down to chest height
            t.spin += dt * 9.0f;

            // A tomato only ever hits enemies: it flies straight through players.
            if (EnemySystem::HitEnemyInRange(t.lane, t.x - 0.3f, t.x + 0.3f, PLAYER_DAMAGE))
            {
                t.splatT = 0.0f;
                Audio::Play(Audio::Sfx::TomatoSplat, 0.9f);
            }
        }
        tomatoes.erase(std::remove_if(tomatoes.begin(), tomatoes.end(), [](const Tomato& t)
            { return t.splatT > SPLAT_TIME || t.x > WORLD_WIDTH + 0.5f; }), tomatoes.end());
    }
}

namespace PlayerSystem
{
    void Init()
    {
        players.clear();
        tomatoes.clear();
        nextId = 1;
        hoveredLane = -1;
        for (int i = 0; i < NUM_LANES; ++i)
            laneSelected[i] = false;
    }

    void Shutdown() { Init(); }

    // ------------------------------------------------------------------
    // Lanes
    // ------------------------------------------------------------------
    bool UpdateLanes()
    {
        const glm::vec2 mouse = Input::GetMouseWorldPos();
        hoveredLane = -1;

        for (int lane = 0; lane < NUM_LANES; ++lane)
        {
            // The lane rectangle, split into two triangles (Lab 3 hit test).
            const float bottom = LANE_Y[lane] - LANE_HEIGHT * 0.5f;
            const float top = LANE_Y[lane] + LANE_HEIGHT * 0.5f;
            const glm::vec2 a(LANE_LEFT, bottom), b(LANE_RIGHT, bottom);
            const glm::vec2 c(LANE_RIGHT, top), d(LANE_LEFT, top);

            if (pointInTriangle(mouse, a, b, c) || pointInTriangle(mouse, a, c, d))
            {
                hoveredLane = lane;
                break;
            }
        }

        // A click toggles the lane; several lanes can stay selected.
        if (hoveredLane != -1 && Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            laneSelected[hoveredLane] = !laneSelected[hoveredLane];
            Audio::Play(Audio::Sfx::LaneSelect, 0.8f);
            return true;
        }
        return false;
    }

    int GetHoveredLane() { return hoveredLane; }
    bool IsLaneSelected(int lane) { return lane >= 0 && lane < NUM_LANES && laneSelected[lane]; }

    int GetSelectedLaneCount()
    {
        int n = 0;
        for (int i = 0; i < NUM_LANES; ++i) if (laneSelected[i]) ++n;
        return n;
    }

    float GetLaneHighlight(int lane)
    {
        bool sel = laneSelected[lane], hov = (lane == hoveredLane);
        if (sel && hov) return 0.32f;
        if (sel) return 0.24f;
        if (hov) return 0.12f;
        return 0.0f;
    }

    // ------------------------------------------------------------------
    // Placement
    // ------------------------------------------------------------------
    int PlaceOnSelectedLanes(PlayerType type)
    {
        const int cost = PLAYER_COST[Index(type)];
        if (GetSelectedLaneCount() == 0)
        {
            Hud::ShowToast("SELECT A LANE FIRST");
            Audio::Play(Audio::Sfx::Locked);
            return 0;
        }

        int placed = 0;
        for (int lane = 0; lane < NUM_LANES; ++lane)
        {
            if (!laneSelected[lane])
                continue;
            if (Economy::GetCoins() < cost)
            {
                Hud::ShowToast("NOT ENOUGH COINS");
                break;
            }
            if (type == PlayerType::Defense)
            {
                // Only one mechanic fits on the spawn spot of a lane.
                bool taken = false;
                for (const Player& p : players)
                    if (p.lane == lane && p.type == PlayerType::Defense) taken = true;
                if (taken)
                {
                    Hud::ShowToast("EL ME'ALLEM IS ALREADY HERE");
                    continue;
                }
            }
            Economy::Spend(cost);
            SpawnPlayer(type, lane);
            Hud::AddSpark(glm::vec2(LANE_START_X, LANE_Y[lane]), glm::vec3(1.0f, 0.9f, 0.5f));
            ++placed;
        }
        if (placed > 0)
            Audio::Play(Audio::Sfx::Place);
        else
            Audio::Play(Audio::Sfx::Locked);
        return placed;
    }

    // ------------------------------------------------------------------
    // Per frame
    // ------------------------------------------------------------------
    void HandleKeys()
    {
        const bool up = Input::IsKeyPressed(GLFW_KEY_W) || Input::IsKeyPressed(GLFW_KEY_UP);
        const bool down = Input::IsKeyPressed(GLFW_KEY_S) || Input::IsKeyPressed(GLFW_KEY_DOWN);
        if (!up && !down)
            return;

        bool any = false;
        for (Player& p : players)
        {
            // Defense players are not affected by jump or duck.
            if (!laneSelected[p.lane] || p.type == PlayerType::Defense)
                continue;
            if (up && p.jumpT <= 0.0f)        { p.jumpT = JUMP_TIME; p.duckT = 0.0f; any = true; }
            else if (down && p.duckT <= 0.0f) { p.duckT = DUCK_TIME; p.jumpT = 0.0f; any = true; }
        }
        if (any)
            Audio::Play(up ? Audio::Sfx::Jump : Audio::Sfx::Duck, 0.7f);
    }

    void ShareWithEnemies()
    {
        EnemySystem::ClearPlayers();
        for (const Player& p : players)
            EnemySystem::AddPlayer(p.id, p.lane, p.x, p.duckT > 0.0f, p.jumpT > 0.0f, p.type == PlayerType::Defense);
    }

    void Update(float deltaTime)
    {
        // 1) damage from the enemies (dodging was already checked)
        for (const EnemySystem::EnemyHit& hit : EnemySystem::GetHits())
        {
            if (Player* p = FindPlayer(hit.playerId))
            {
                p->health -= hit.damage;
                p->flash = 1.0f;
                Audio::Play(Audio::Sfx::PlayerHurt, 0.8f);
                Hud::AddSpark(glm::vec2(p->x, LANE_Y[p->lane] + 0.2f), glm::vec3(1.0f, 0.35f, 0.2f));
            }
        }

        // 2) move, animate, attack, pick up coins
        for (Player& p : players)
        {
            UpdatePlayer(p, deltaTime);
            Economy::CollectAt(p.lane, p.x);
        }
        UpdateTomatoes(deltaTime);

        // 3) defeated players are removed; players that reach the far end leave
        for (const Player& p : players)
            if (p.health <= 0)
                Audio::Play(Audio::Sfx::PlayerDown, 0.9f);
        players.erase(std::remove_if(players.begin(), players.end(), [](const Player& p)
            { return p.health <= 0 || p.x > LANE_END_X; }), players.end());
    }

    void RenderLane(int lane)
    {
        // Defense players at the back, so the others walk in front of them.
        for (int pass = 0; pass < 2; ++pass)
            for (const Player& p : players)
            {
                if (p.lane != lane || (pass == 0) != (p.type == PlayerType::Defense))
                    continue;

                const float y = LANE_Y[lane];
                const float jump = JumpHeight(p);

                // shadow shrinks while in the air
                float shadow = 0.34f * (1.0f - 0.4f * jump / JUMP_HEIGHT);
                Renderer2D::DrawEllipse(glm::vec2(p.x, y - FOOT_OFFSET), glm::vec2(shadow, 0.07f), glm::vec3(0.45f, 0.36f, 0.25f));

                // red shades get stronger as health drops, plus a hit flash
                float lost = 1.0f - static_cast<float>(p.health) / static_cast<float>(p.maxHealth);
                Renderer2D::SetDamage(std::min(0.85f, 0.55f * lost + 0.45f * p.flash));
                CharacterRenderer::DrawPlayer(p.x, y, p.type, p.anim, p.animTime, jump, DuckAmount(p),
                                              (p.moving && p.anim == PlayerAnim::Attack) ? p.walkTime : -1.0f);
                Renderer2D::SetDamage(0.0f);

                HealthBar::Draw(glm::vec2(p.x, y - FOOT_OFFSET - 0.17f), p.health, p.maxHealth, true);
            }
    }

    void RenderProjectiles()
    {
        for (const Tomato& t : tomatoes)
        {
            if (t.splatT < 0.0f)
                CharacterRenderer::DrawTomato(t.x, t.y, t.spin);
            else
                CharacterRenderer::DrawSplat(t.x, t.y, t.splatT / SPLAT_TIME);
        }
    }

    int GetPlayerCount() { return static_cast<int>(players.size()); }

    int GetPlayerCountInLane(int lane, bool includeDefense)
    {
        int n = 0;
        for (const Player& p : players)
            if (p.lane == lane && (includeDefense || p.type != PlayerType::Defense)) ++n;
        return n;
    }
}
