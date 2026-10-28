#include "Game.h"
#include "GameConfig.h"
#include "Renderer2D.h"
#include "Input.h"
#include "Text.h"
#include "Draw.h"
#include "Cutscene.h"
#include "Audio.h"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <random>
#include <string>
#include <vector>

using glm::vec2;
using glm::vec3;

namespace
{
    const float PI = 3.14159265358979f;

    // ------------------------------------------------------------------
    //  Entities
    // ------------------------------------------------------------------
    struct Player
    {
        PlayerType type = PlayerType::Melee;
        int   lane = 0;
        float x = PLAYER_SPAWN_X;
        int   hp = 5, maxHp = 5;
        float cooldown = 0.6f;
        float afterT = 0.0f;       // keeps the attack pose briefly after a swing/shot
        float walk = 0.0f;
        float moving = 0.0f;       // smoothed 0..1 for the walk animation
        float jumpT = 0.0f;        // seconds of jump left
        float duckT = 0.0f;        // seconds of duck left
        float flash = 0.0f;        // hit flash
        float age = 0.0f;
    };

    struct Enemy
    {
        EnemyType type = EnemyType::EasyRanged;
        int   lane = 0;
        float x = ENEMY_SPAWN_X;
        int   hp = 5, maxHp = 5;
        float cooldown = 1.0f;
        float afterT = 0.0f;
        float walk = 0.0f;
        float moving = 0.0f;
        float flash = 0.0f;
        float age = 0.0f;
    };

    enum class ShotKind { Arrow, SpiritBolt, Stinger };

    struct Projectile
    {
        ShotKind kind = ShotKind::Arrow;
        int   lane = 0;
        float x = 0.0f;
        float speed = 0.0f;
        float dir = 1.0f;          // +1 right (player shot), -1 left (enemy shot)
        int   damage = 1;
        bool  high = false;        // enemy shots: true = high attack (duck), false = low attack (jump)
        float age = 0.0f;
        bool  dead = false;
    };

    struct Coin
    {
        int   lane = 0;
        float x = 0.0f;
        int   value = 10;
        float age = 0.0f;
        bool  taken = false;
    };

    struct Spark
    {
        vec2  pos{0.0f};
        float t = 0.0f;
        float life = 0.35f;
        vec3  color{1.0f};
    };

    struct FloatText
    {
        vec2        pos{0.0f};
        float       t = 0.0f;
        std::string text;
        vec3        color{1.0f, 0.9f, 0.3f};
    };

    // ------------------------------------------------------------------
    //  Game state
    // ------------------------------------------------------------------
    Game::State state = Game::State::MainMenu;
    bool wantsExit = false;
    std::mt19937 rng;

    std::vector<Player>     players;
    std::vector<Enemy>      enemies;
    std::vector<Projectile> shots;
    std::vector<Coin>       coinsOnField;
    std::vector<Spark>      sparks;
    std::vector<FloatText>  floaters;

    int   coins = START_COINS;
    bool  laneSelected[NUM_LANES] = {};
    float laneCooldown[NUM_LANES] = {};
    int   hoverLane = -1;
    int   hoverCard = -1;
    float cardPress[3] = {};

    enum class WaveState { Intermission, Spawning, Clearing };
    WaveState waveState = WaveState::Intermission;
    int   waveIndex = 0;
    float waveTimer = 0.0f;
    float spawnTimer = 0.0f;
    std::vector<EnemyType> spawnQueue;

    float worldTime = 0.0f;   // advances only while the game is actually running
    float envTime = 0.0f;     // always advances (flames, menu animation)
    float endTimer = 0.0f;    // delay before the win/lose menu appears
    float uiTime = 0.0f;
    std::string toast;
    float toastT = 0.0f;

    // ------------------------------------------------------------------
    //  Helpers
    // ------------------------------------------------------------------
    int Rand(int lo, int hi) { return std::uniform_int_distribution<int>(lo, hi)(rng); }
    float RandF(float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(rng); }

    float Clamp01(float v) { return std::min(1.0f, std::max(0.0f, v)); }

    void ShowToast(const std::string& s) { toast = s; toastT = 1.6f; }

    void AddSpark(vec2 pos, vec3 color, float life = 0.35f)
    {
        Spark s; s.pos = pos; s.color = color; s.life = life;
        sparks.push_back(s);
    }

    float LaneBottomOf(int lane) { return LANE_Y[lane] - FOOT_OFFSET; }

    int PlayerIndex(PlayerType t) { return static_cast<int>(t); }
    int EnemyIndex(EnemyType t) { return static_cast<int>(t); }

    bool CardUnlocked(PlayerType t) { return coins >= PLAYER_COST[PlayerIndex(t)]; }

    // attack animation progress for the models: -1 idle, otherwise 0..1
    float AttackAnim(float cooldown, float afterT, bool canAttack)
    {
        if (!canAttack) return -1.0f;
        if (afterT > 0.0f) return 1.0f;
        if (cooldown < ATTACK_WINDUP) return 1.0f - cooldown / ATTACK_WINDUP;
        return -1.0f;
    }

    float JumpProgress(const Player& p) { return p.jumpT > 0.0f ? 1.0f - p.jumpT / JUMP_TIME : 0.0f; }
    float JumpHeight(const Player& p) { return p.jumpT > 0.0f ? 0.55f * std::sin(PI * JumpProgress(p)) : 0.0f; }
    float DuckAmount(const Player& p)
    {
        if (p.duckT <= 0.0f) return 0.0f;
        float u = 1.0f - p.duckT / DUCK_TIME;
        return Clamp01(std::min(u, 1.0f - u) * 7.0f);
    }

    // ------------------------------------------------------------------
    //  World set-up
    // ------------------------------------------------------------------
    void ResetWorld()
    {
        players.clear(); enemies.clear(); shots.clear(); coinsOnField.clear();
        sparks.clear(); floaters.clear();
        coins = START_COINS;
        for (int i = 0; i < NUM_LANES; ++i) { laneSelected[i] = false; laneCooldown[i] = 0.0f; }
        for (int i = 0; i < 3; ++i) cardPress[i] = 0.0f;
        waveIndex = 0;
        waveState = WaveState::Intermission;
        waveTimer = 1.8f;
        spawnQueue.clear();
        spawnTimer = 0.0f;
        worldTime = 0.0f;
        endTimer = 0.0f;
        toastT = 0.0f;
    }

    void BuildWaveQueue(int wave)
    {
        spawnQueue.clear();
        int n = WAVE_ENEMY_COUNT[wave];
        if (wave == 0) {
            for (int i = 0; i < n; ++i) spawnQueue.push_back(EnemyType::EasyRanged);       // Wave 1: easy only
        } else if (wave == 1) {
            for (int i = 0; i < n; ++i)                                                     // Wave 2: easy + medium
                spawnQueue.push_back(i % 2 == 0 ? EnemyType::EasyRanged : EnemyType::MediumRanged);
        } else {
            for (int i = 0; i < n; ++i) {                                                   // Wave 3: all three
                int r = i % 3;
                spawnQueue.push_back(r == 0 ? EnemyType::EasyRanged : (r == 1 ? EnemyType::MediumRanged : EnemyType::HardMelee));
            }
        }
        std::shuffle(spawnQueue.begin(), spawnQueue.end(), rng);
    }

    void SpawnEnemy(EnemyType t)
    {
        Enemy e;
        e.type = t;
        e.lane = Rand(0, NUM_LANES - 1);          // enemies appear in a random lane
        e.x = ENEMY_SPAWN_X;
        e.maxHp = e.hp = ENEMY_MAX_HP[EnemyIndex(t)];
        e.cooldown = RandF(0.9f, 1.4f);
        enemies.push_back(e);
    }

    void SpawnPlayer(PlayerType t, int lane)
    {
        Player p;
        p.type = t;
        p.lane = lane;
        p.x = PLAYER_SPAWN_X;                     // players appear at the leftmost spot of the lane
        p.maxHp = p.hp = PLAYER_MAX_HP[PlayerIndex(t)];
        p.cooldown = 0.7f;
        players.push_back(p);
        AddSpark(vec2(p.x, LANE_Y[lane]), vec3(1.0f, 0.9f, 0.5f), 0.5f);
    }

    // Place `type` on every selected lane that can still afford it.
    void PlaceOnSelectedLanes(PlayerType type)
    {
        int cost = PLAYER_COST[PlayerIndex(type)];
        if (coins < cost) return;                 // locked card: ignored
        bool any = false;
        for (int l = 0; l < NUM_LANES; ++l) if (laneSelected[l]) any = true;
        if (!any) { ShowToast("SELECT A LANE FIRST"); Audio::Play(Audio::Sfx::Locked); return; }

        int placed = 0;
        for (int l = 0; l < NUM_LANES; ++l) {
            if (!laneSelected[l]) continue;
            if (coins < cost) break;
            if (laneCooldown[l] > 0.0f) continue;      // tiny delay stops accidental double-stacking
            if (type == PlayerType::Defense) {
                bool taken = false;
                for (const Player& p : players)
                    if (p.lane == l && p.type == PlayerType::Defense && p.x < PLAYER_SPAWN_X + 0.6f) taken = true;
                if (taken) { ShowToast("A GUARDIAN ALREADY STANDS HERE"); Audio::Play(Audio::Sfx::Locked); continue; }
            }
            coins -= cost;
            SpawnPlayer(type, l);
            laneCooldown[l] = 0.45f;
            ++placed;
        }
        if (placed > 0) Audio::Play(Audio::Sfx::Place);
    }

    // ------------------------------------------------------------------
    //  Combat
    // ------------------------------------------------------------------
    void DamagePlayer(Player& p, int dmg)
    {
        p.hp -= dmg;
        p.flash = 1.0f;
        Audio::Play(Audio::Sfx::Hurt, 0.8f);
        AddSpark(vec2(p.x, LANE_Y[p.lane] + 0.1f), vec3(1.0f, 0.35f, 0.2f));
    }

    void DamageEnemy(Enemy& e, int dmg)
    {
        e.hp -= dmg;
        e.flash = 1.0f;
        Audio::Play(Audio::Sfx::Hit, 0.8f);
        AddSpark(vec2(e.x, LANE_Y[e.lane] + 0.1f), vec3(0.95f, 0.85f, 0.3f));
    }

    void DropCoin(const Enemy& e)
    {
        Coin c;
        c.lane = e.lane;
        c.x = std::min(15.4f, std::max(HOME_BASE_RIGHT + 0.5f, e.x));
        c.value = ENEMY_COIN[EnemyIndex(e.type)];
        coinsOnField.push_back(c);
    }

    void CollectCoin(Coin& c)
    {
        if (c.taken) return;
        c.taken = true;
        coins += c.value;
        Audio::Play(Audio::Sfx::Coin);
        AddSpark(vec2(c.x, LANE_Y[c.lane] - 0.2f), vec3(1.0f, 0.85f, 0.2f), 0.4f);
        FloatText f; f.pos = vec2(c.x, LANE_Y[c.lane] + 0.1f); f.text = "+" + std::to_string(c.value);
        floaters.push_back(f);
    }

    // true if an enemy and a player in the same lane are touching
    bool EnemyTouchesPlayer(const Enemy& e)
    {
        for (const Player& p : players) {
            if (p.lane != e.lane) continue;
            float dx = e.x - p.x;
            if (dx > -0.2f && dx < CONTACT_DIST) return true;
        }
        return false;
    }

    bool PlayerTouchesEnemy(const Player& p)
    {
        for (const Enemy& e : enemies) {
            if (e.lane != p.lane) continue;
            float dx = e.x - p.x;
            if (dx > -0.2f && dx < CONTACT_DIST) return true;
        }
        return false;
    }

    void EnemyAttack(Enemy& e)
    {
        int idx = EnemyIndex(e.type);
        if (e.type == EnemyType::HardMelee) {
            // closest player in front of the brute takes the hit
            Player* best = nullptr;
            for (Player& p : players) {
                if (p.lane != e.lane || p.hp <= 0) continue;
                float dx = e.x - p.x;
                if (dx > -0.2f && dx < MELEE_REACH + 0.1f && (!best || p.x > best->x)) best = &p;
            }
            if (best) { Audio::Play(Audio::Sfx::Sword, 0.7f); DamagePlayer(*best, ENEMY_DAMAGE[idx]); }
        } else {
            Projectile s;
            s.lane = e.lane;
            s.x = e.x - 0.55f;
            s.dir = -1.0f;
            s.damage = ENEMY_DAMAGE[idx];
            if (e.type == EnemyType::EasyRanged) { s.kind = ShotKind::SpiritBolt; s.high = true;  s.speed = 3.6f; }
            else                                 { s.kind = ShotKind::Stinger;    s.high = false; s.speed = 3.9f; }
            shots.push_back(s);
            Audio::Play(e.type == EnemyType::EasyRanged ? Audio::Sfx::SpiritBolt : Audio::Sfx::Stinger, 0.7f);
        }
    }

    void PlayerAttack(Player& p)
    {
        if (p.type == PlayerType::Melee) {
            Enemy* best = nullptr;
            for (Enemy& e : enemies) {
                if (e.lane != p.lane || e.hp <= 0) continue;
                float dx = e.x - p.x;
                if (dx > -0.2f && dx < MELEE_REACH && (!best || e.x < best->x)) best = &e;
            }
            if (best) { Audio::Play(Audio::Sfx::Sword, 0.7f); DamageEnemy(*best, PLAYER_DAMAGE); }
        } else if (p.type == PlayerType::Ranged) {
            Projectile s;
            s.kind = ShotKind::Arrow;
            s.lane = p.lane;
            s.x = p.x + 0.5f;
            s.dir = 1.0f;
            s.speed = 5.4f;
            s.damage = PLAYER_DAMAGE;
            shots.push_back(s);
            Audio::Play(Audio::Sfx::Arrow, 0.8f);
        }
    }

    bool Dodges(const Player& p, const Projectile& s)
    {
        if (p.type == PlayerType::Defense) return false;      // defenders are not affected by jump/duck
        if (s.high && p.duckT > 0.0f) return true;            // ducking avoids high attacks
        if (!s.high && p.jumpT > 0.0f) return true;           // jumping avoids low attacks
        return false;
    }

    void UpdateShots(float dt)
    {
        for (Projectile& s : shots) {
            s.age += dt;
            s.x += s.dir * s.speed * dt;
            if (s.x < -1.0f || s.x > WORLD_WIDTH + 1.0f) { s.dead = true; continue; }

            if (s.dir < 0.0f) {
                // enemy shot: the first non-dodging player it reaches takes the hit. A defender
                // standing in front shields everything behind it because the shot meets it first.
                Player* hit = nullptr;
                for (Player& p : players) {
                    if (p.lane != s.lane || p.hp <= 0) continue;
                    if (std::fabs(p.x - s.x) > 0.34f) continue;
                    if (Dodges(p, s)) continue;
                    if (!hit || p.x > hit->x) hit = &p;
                }
                if (hit) { DamagePlayer(*hit, s.damage); s.dead = true; }
            } else {
                // player arrow: damages the first enemy in its path, never other players
                Enemy* hit = nullptr;
                for (Enemy& e : enemies) {
                    if (e.lane != s.lane || e.hp <= 0) continue;
                    if (std::fabs(e.x - s.x) > 0.45f) continue;
                    if (!hit || e.x < hit->x) hit = &e;
                }
                if (hit) { DamageEnemy(*hit, s.damage); s.dead = true; }
            }
        }
        shots.erase(std::remove_if(shots.begin(), shots.end(), [](const Projectile& s) { return s.dead; }), shots.end());
    }

    void UpdatePlayers(float dt)
    {
        for (Player& p : players) {
            p.age += dt;
            p.flash = std::max(0.0f, p.flash - dt * 3.0f);
            p.jumpT = std::max(0.0f, p.jumpT - dt);
            p.duckT = std::max(0.0f, p.duckT - dt);
            p.afterT = std::max(0.0f, p.afterT - dt);

            bool canMove = p.type != PlayerType::Defense && !PlayerTouchesEnemy(p);
            if (canMove) {
                float v = PLAYER_SPEED[PlayerIndex(p.type)];
                p.x += v * dt;
                p.walk += dt * v * 9.0f;
            }
            p.moving += ((canMove ? 1.0f : 0.0f) - p.moving) * std::min(1.0f, dt * 10.0f);

            if (p.type != PlayerType::Defense) {
                p.cooldown -= dt;
                if (p.cooldown <= 0.0f) {
                    PlayerAttack(p);
                    p.cooldown = PLAYER_COOLDOWN[PlayerIndex(p.type)];
                    p.afterT = 0.15f;
                }
            }
        }
        for (const Player& p : players) if (p.hp <= 0) Audio::Play(Audio::Sfx::PlayerDie, 0.8f);
        // defeated players leave the scene; players that reach the far end leave too
        players.erase(std::remove_if(players.begin(), players.end(),
            [](const Player& p) { return p.hp <= 0 || p.x > LANE_RIGHT_END + 0.4f; }), players.end());
    }

    bool UpdateEnemies(float dt)   // returns true if an enemy reached the home base
    {
        bool reached = false;
        for (Enemy& e : enemies) {
            e.age += dt;
            e.flash = std::max(0.0f, e.flash - dt * 3.0f);
            e.afterT = std::max(0.0f, e.afterT - dt);

            bool canMove = !EnemyTouchesPlayer(e);
            if (canMove) {
                float v = ENEMY_SPEED[EnemyIndex(e.type)];
                e.x -= v * dt;
                e.walk += dt * v * (e.type == EnemyType::HardMelee ? 6.0f : 8.0f);
            }
            e.moving += ((canMove ? 1.0f : 0.0f) - e.moving) * std::min(1.0f, dt * 10.0f);

            e.cooldown -= dt;
            if (e.cooldown <= 0.0f) {
                EnemyAttack(e);
                e.cooldown = ENEMY_COOLDOWN[EnemyIndex(e.type)];
                e.afterT = 0.15f;
            }
            if (e.x <= BASE_REACHED_X) reached = true;
        }
        for (const Enemy& e : enemies)
            if (e.hp <= 0) { DropCoin(e); Audio::Play(Audio::Sfx::EnemyDie, 0.8f); }
        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](const Enemy& e) { return e.hp <= 0; }), enemies.end());
        return reached;
    }

    void UpdateCoins(float dt)
    {
        for (Coin& c : coinsOnField) {
            c.age += dt;
            if (c.taken) continue;
            if (c.age < 0.35f) continue;   // still dropping
            for (const Player& p : players)
                if (p.lane == c.lane && p.hp > 0 && std::fabs(p.x - c.x) < 0.6f) { CollectCoin(c); break; }
        }
        coinsOnField.erase(std::remove_if(coinsOnField.begin(), coinsOnField.end(), [](const Coin& c) { return c.taken; }), coinsOnField.end());
    }

    void UpdateEffects(float dt)
    {
        for (Spark& s : sparks) s.t += dt;
        sparks.erase(std::remove_if(sparks.begin(), sparks.end(), [](const Spark& s) { return s.t >= s.life; }), sparks.end());
        for (FloatText& f : floaters) f.t += dt;
        floaters.erase(std::remove_if(floaters.begin(), floaters.end(), [](const FloatText& f) { return f.t > 1.0f; }), floaters.end());
        toastT = std::max(0.0f, toastT - dt);
        for (int i = 0; i < NUM_LANES; ++i) laneCooldown[i] = std::max(0.0f, laneCooldown[i] - dt);
        for (int i = 0; i < 3; ++i) cardPress[i] = std::max(0.0f, cardPress[i] - dt * 4.0f);
    }

    void UpdateWaves(float dt)
    {
        switch (waveState) {
        case WaveState::Intermission:
            waveTimer -= dt;
            if (waveTimer <= 0.0f) {
                BuildWaveQueue(waveIndex);
                Audio::Play(Audio::Sfx::Horn);
                waveState = WaveState::Spawning;   // a wave begins as soon as its first enemy appears
                spawnTimer = 0.0f;
            }
            break;
        case WaveState::Spawning:
            spawnTimer -= dt;
            if (spawnTimer <= 0.0f && !spawnQueue.empty()) {
                SpawnEnemy(spawnQueue.back());
                spawnQueue.pop_back();
                static const float lo[NUM_WAVES] = { 4.5f, 3.2f, 2.4f };
                static const float hi[NUM_WAVES] = { 6.5f, 5.0f, 3.8f };
                spawnTimer = RandF(lo[waveIndex], hi[waveIndex]);
                if (spawnQueue.empty()) waveState = WaveState::Clearing;   // wave ends when its last enemy appears
            }
            break;
        case WaveState::Clearing:
            if (enemies.empty()) {
                if (waveIndex >= NUM_WAVES - 1) {
                    state = Game::State::Won;
                    endTimer = 0.0f;
                } else {
                    ++waveIndex;
                    waveState = WaveState::Intermission;
                    waveTimer = 3.0f;
                }
            }
            break;
        }
    }

    // ------------------------------------------------------------------
    //  UI geometry
    // ------------------------------------------------------------------
    vec2 CardCenter(int i) { return vec2(1.35f + 2.0f * static_cast<float>(i), 7.95f); }
    const vec2 CARD_SIZE(1.85f, 1.70f);

    bool InBox(vec2 p, vec2 c, vec2 size)
    {
        return std::fabs(p.x - c.x) <= size.x * 0.5f && std::fabs(p.y - c.y) <= size.y * 0.5f;
    }

    int LaneAt(vec2 m)
    {
        if (m.x < HOME_BASE_RIGHT || m.x > WORLD_WIDTH) return -1;
        for (int i = 0; i < NUM_LANES; ++i)
            if (std::fabs(m.y - LANE_Y[i]) <= LANE_HEIGHT * 0.5f) return i;
        return -1;
    }

    int CardAt(vec2 m)
    {
        for (int i = 0; i < 3; ++i)
            if (InBox(m, CardCenter(i), CARD_SIZE)) return i;
        return -1;
    }

    // Menu buttons: the same layout is used for drawing and for hit testing.
    const vec2 BUTTON_SIZE(5.4f, 0.95f);
    vec2 ButtonCenter(int i, float topY) { return vec2(WORLD_WIDTH * 0.5f, topY - 1.25f * static_cast<float>(i)); }

    int ButtonAt(vec2 m, int count, float topY)
    {
        for (int i = 0; i < count; ++i)
            if (InBox(m, ButtonCenter(i, topY), BUTTON_SIZE)) return i;
        return -1;
    }

    void DrawButtons(const std::vector<std::string>& labels, float topY)
    {
        int hover = ButtonAt(Input::GetMouseWorldPos(), static_cast<int>(labels.size()), topY);
        for (size_t i = 0; i < labels.size(); ++i) {
            vec2 c = ButtonCenter(static_cast<int>(i), topY);
            bool h = hover == static_cast<int>(i);
            Draw::Plaque(c, BUTTON_SIZE, h ? 0.30f : 0.0f);
            Text::DrawShadow(labels[i], c + vec2(0.0f, -0.245f), 0.07f, h ? vec3(1.0f, 0.95f, 0.6f) : vec3(1.0f, 0.97f, 0.88f), Text::Align::Center);
        }
    }

    // ------------------------------------------------------------------
    //  Input handling per state
    // ------------------------------------------------------------------
    void StartGame()
    {
        ResetWorld();
        state = Game::State::Cutscene;
        Cutscene::Start();
    }

    void BeginPlaying()
    {
        ResetWorld();
        state = Game::State::Playing;
    }

    void ToMainMenu()
    {
        ResetWorld();
        state = Game::State::MainMenu;
    }

    void HandlePlayingInput()
    {
        vec2 m = Input::GetMouseWorldPos();
        hoverCard = CardAt(m);
        hoverLane = hoverCard >= 0 ? -1 : LaneAt(m);

        if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            bool used = false;

            // 1) collectibles can also be picked up by clicking them
            for (Coin& c : coinsOnField) {
                if (c.taken || c.age < 0.35f) continue;
                if (std::fabs(m.x - c.x) < 0.4f && std::fabs(m.y - (LANE_Y[c.lane] - 0.2f)) < 0.45f) {
                    CollectCoin(c); used = true; break;
                }
            }
            // 2) player cards: only unlocked ones respond
            if (!used && hoverCard >= 0) {
                PlayerType t = static_cast<PlayerType>(hoverCard);
                if (CardUnlocked(t)) { cardPress[hoverCard] = 1.0f; PlaceOnSelectedLanes(t); }
                used = true;
            }
            // 3) lanes: a click toggles that lane between selected and not selected
            if (!used && hoverLane >= 0) { laneSelected[hoverLane] = !laneSelected[hoverLane]; Audio::Play(Audio::Sfx::Click, 0.8f); }
        }

        // keyboard shortcuts for the cards
        if (Input::IsKeyPressed(GLFW_KEY_1) && CardUnlocked(PlayerType::Melee))   { cardPress[0] = 1.0f; PlaceOnSelectedLanes(PlayerType::Melee); }
        if (Input::IsKeyPressed(GLFW_KEY_2) && CardUnlocked(PlayerType::Ranged))  { cardPress[1] = 1.0f; PlaceOnSelectedLanes(PlayerType::Ranged); }
        if (Input::IsKeyPressed(GLFW_KEY_3) && CardUnlocked(PlayerType::Defense)) { cardPress[2] = 1.0f; PlaceOnSelectedLanes(PlayerType::Defense); }

        // jump / duck every player standing in a selected lane
        bool up = Input::IsKeyPressed(GLFW_KEY_W) || Input::IsKeyPressed(GLFW_KEY_UP);
        bool down = Input::IsKeyPressed(GLFW_KEY_S) || Input::IsKeyPressed(GLFW_KEY_DOWN);
        if (up || down) {
            bool any = false;
            for (const Player& p : players) if (laneSelected[p.lane] && p.type != PlayerType::Defense) any = true;
            if (any) Audio::Play(up ? Audio::Sfx::Jump : Audio::Sfx::Duck);
            for (Player& p : players) {
                if (!laneSelected[p.lane] || p.type == PlayerType::Defense) continue;
                if (up)   { p.jumpT = JUMP_TIME; p.duckT = 0.0f; }
                else      { p.duckT = DUCK_TIME; p.jumpT = 0.0f; }
            }
        }
    }
}

// ===========================================================================
//  Public API
// ===========================================================================

void Game::Init(unsigned seed)
{
    std::random_device rd;
    rng.seed(seed == 0 ? rd() : seed);
    ResetWorld();
    state = State::MainMenu;
    wantsExit = false;
}

bool Game::WantsExit() { return wantsExit; }

#ifdef GAME_TEST_HOOKS
void Game::DebugAddCoins(int n) { coins += n; }
#endif

Game::Snapshot Game::GetSnapshot()
{
    Snapshot s;
    s.state = state;
    s.coins = coins;
    s.wave = waveIndex + 1;
    s.players = static_cast<int>(players.size());
    s.enemies = static_cast<int>(enemies.size());
    s.selectedLanes = 0;
    for (int i = 0; i < NUM_LANES; ++i) if (laneSelected[i]) ++s.selectedLanes;
    s.enemiesToSpawn = static_cast<int>(spawnQueue.size());
    return s;
}

using State = Game::State;

static void UpdateCore(float dt)
{
    dt = std::min(dt, 0.05f);          // a long frame must never let things tunnel through each other
    envTime += dt;
    uiTime += dt;
    vec2 m = Input::GetMouseWorldPos();
    bool click = Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);

    switch (state) {
    case State::MainMenu: {
        int b = click ? ButtonAt(m, 2, 3.9f) : -1;
        if (b == 0) StartGame();
        else if (b == 1) wantsExit = true;
        break;
    }
    case State::Cutscene:
        Cutscene::Update(dt);
        if (Cutscene::IsFinished() || Input::IsKeyPressed(GLFW_KEY_SPACE) || Input::IsKeyPressed(GLFW_KEY_ENTER))
            BeginPlaying();
        break;

    case State::Playing: {
        if (Input::IsKeyPressed(GLFW_KEY_ESCAPE)) { state = State::Paused; break; }
        HandlePlayingInput();
        worldTime += dt;
        UpdateWaves(dt);
        UpdatePlayers(dt);
        bool reached = UpdateEnemies(dt);
        UpdateShots(dt);
        UpdateCoins(dt);
        UpdateEffects(dt);
        if (reached) { state = State::Lost; endTimer = 0.0f; }   // everything stops from here on
        break;
    }
    case State::Paused: {
        if (Input::IsKeyPressed(GLFW_KEY_ESCAPE)) { state = State::Playing; break; }
        int b = click ? ButtonAt(m, 3, 4.0f) : -1;
        if (b == 0) state = State::Playing;
        else if (b == 1) BeginPlaying();
        else if (b == 2) ToMainMenu();
        break;
    }
    case State::Won:
    case State::Lost: {
        endTimer += dt;
        if (endTimer > 0.9f && click) {
            int b = ButtonAt(m, 2, 3.4f);
            if (b == 0) BeginPlaying();
            else if (b == 1) ToMainMenu();
        }
        break;
    }
    }
}

// Sound layer: music per state, jingles on transitions, button clicks, cutscene stings, M to mute.
void Game::Update(float dt)
{
    static State prev = State::MainMenu;
    static float prevCut = 0.0f;
    static bool audioStarted = false;
    if (!audioStarted) { audioStarted = true; Audio::Init(); }

    vec2 m = Input::GetMouseWorldPos();
    if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
        int b = -1;
        if (state == State::MainMenu) b = ButtonAt(m, 2, 3.9f);
        else if (state == State::Paused) b = ButtonAt(m, 3, 4.0f);
        else if ((state == State::Won || state == State::Lost) && endTimer > 0.9f) b = ButtonAt(m, 2, 3.4f);
        else if (state == State::Playing) { int c = CardAt(m); if (c >= 0 && CardUnlocked(static_cast<PlayerType>(c))) b = 0; }
        if (b >= 0) Audio::Play(Audio::Sfx::Click);
    }
    if (Input::IsKeyPressed(GLFW_KEY_M)) Audio::ToggleMute();

    UpdateCore(dt);

    if (state == State::Cutscene) {
        float t = Cutscene::Time();
        auto crossed = [&](float at) { return prevCut < at && t >= at; };
        if (crossed(4.2f)) Audio::Play(Audio::Sfx::Horn, 0.8f);
        if (crossed(7.6f)) Audio::Play(Audio::Sfx::Roar);
        if (crossed(11.4f) || crossed(13.0f)) Audio::Play(Audio::Sfx::Arrow);
        if (crossed(13.5f)) Audio::Play(Audio::Sfx::SpiritBolt);
        if (crossed(14.1f)) Audio::Play(Audio::Sfx::Hurt);
        if (crossed(11.9f) || crossed(13.5f)) Audio::Play(Audio::Sfx::Hit, 0.7f);
        prevCut = t;
    } else prevCut = 0.0f;

    if (state != prev) {
        if (state == State::Won) Audio::Play(Audio::Sfx::Win);
        if (state == State::Lost) Audio::Play(Audio::Sfx::Lose);
        prev = state;
    }
    switch (state) {
    case State::MainMenu: case State::Cutscene: Audio::SetMusic(Audio::Music::Theme); break;
    case State::Playing: case State::Paused:    Audio::SetMusic(Audio::Music::Battle); break;
    default:                                    Audio::SetMusic(Audio::Music::None); break;
    }
    Audio::SetMusicDucked(state == State::Paused);
}

// ===========================================================================
//  Rendering
// ===========================================================================

namespace
{
    void DrawScene(bool interactive)
    {
        Draw::Sky(envTime);
        Draw::Foreground(envTime);
        for (int l = 0; l < NUM_LANES; ++l) {
            float hl = 0.0f;
            bool sel = interactive && laneSelected[l];
            if (interactive) {
                if (sel) hl = 0.20f;
                if (hoverLane == l) hl = sel ? 0.30f : 0.12f;
            }
            Draw::LaneFloor(l, hl, sel);
        }
        Draw::HomeBase(envTime);
    }

    void DrawPlayer(const Player& p)
    {
        Draw::Pose pose;
        float jump = JumpHeight(p);
        pose.foot = vec2(p.x, LaneBottomOf(p.lane) + jump);
        pose.facing = 1.0f;
        pose.time = worldTime + static_cast<float>(p.lane);
        pose.walk = p.walk;
        pose.moving = p.moving;
        pose.attack = AttackAnim(p.cooldown, p.afterT, p.type != PlayerType::Defense);
        pose.air = p.jumpT > 0.0f ? std::sin(PI * JumpProgress(p)) : 0.0f;
        pose.duck = DuckAmount(p);
        pose.fx.red = 1.0f - static_cast<float>(p.hp) / static_cast<float>(p.maxHp);   // redder as health drops
        pose.fx.flash = p.flash * 0.35f;
        switch (p.type) {
        case PlayerType::Melee:   Draw::Melee(pose); break;
        case PlayerType::Ranged:  Draw::Archer(pose); break;
        case PlayerType::Defense: Draw::Guardian(pose); break;
        }
    }

    void DrawEnemy(const Enemy& e)
    {
        Draw::Pose pose;
        pose.foot = vec2(e.x, LaneBottomOf(e.lane));
        if (e.type == EnemyType::EasyRanged) pose.foot.y += Draw::VULTURE_HOVER;
        pose.facing = -1.0f;
        pose.time = worldTime + static_cast<float>(e.lane) * 0.7f;
        pose.walk = e.walk;
        pose.moving = e.moving;
        pose.attack = AttackAnim(e.cooldown, e.afterT, true);
        pose.fx.red = 1.0f - static_cast<float>(e.hp) / static_cast<float>(e.maxHp);
        pose.fx.flash = e.flash * 0.35f;
        switch (e.type) {
        case EnemyType::EasyRanged:   Draw::Vulture(pose); break;
        case EnemyType::MediumRanged: Draw::Scorpion(pose); break;
        case EnemyType::HardMelee:    Draw::Mummy(pose); break;
        }
    }

    void DrawShot(const Projectile& s)
    {
        float y = LANE_Y[s.lane];
        switch (s.kind) {
        case ShotKind::Arrow:      Draw::Arrow(vec2(s.x, y + 0.42f), s.age); break;
        case ShotKind::SpiritBolt: Draw::SpiritBolt(vec2(s.x, y + 0.50f), s.age); break;
        case ShotKind::Stinger:    Draw::StingerBolt(vec2(s.x, y - 0.40f), s.age); break;
        }
    }

    void DrawCardsAndHud()
    {
        // ---- player cards ----
        for (int i = 0; i < 3; ++i) {
            PlayerType t = static_cast<PlayerType>(i);
            bool unlocked = CardUnlocked(t);
            vec2 c = CardCenter(i);
            Draw::Fx fx;
            fx.gray = unlocked ? 0.0f : 1.0f;          // locked cards are grayscale
            bool hov = unlocked && hoverCard == i;     // locked cards never react to the mouse
            float press = cardPress[i];
            vec2 cc = c + vec2(0.0f, hov ? 0.05f : 0.0f) + vec2(0.0f, -0.08f * press);
            Draw::Plaque(cc, CARD_SIZE, hov ? 0.28f : 0.0f, fx);
            Draw::Fx pf = fx;
            pf.flash = hov ? 0.12f : 0.0f;
            Draw::CardPortrait(t, cc + vec2(0.0f, 0.12f), envTime, pf);

            // cost row: coin icon + price
            Draw::Fx cf; cf.gray = fx.gray;
            Draw::Coin(cc + vec2(-0.34f, -0.67f), 0.0f, 0.5f, cf);
            vec3 txt = unlocked ? vec3(1.0f, 0.95f, 0.7f) : vec3(0.75f);
            Text::DrawShadow(std::to_string(PLAYER_COST[i]), cc + vec2(-0.10f, -0.775f), 0.045f, txt);
            Text::DrawShadow(std::to_string(i + 1), cc + vec2(-0.80f, 0.62f), 0.04f, txt);
        }

        // ---- wave panel ----
        Draw::Plaque(vec2(9.0f, 8.0f), vec2(3.4f, 1.1f));
        int shownWave = std::min(waveIndex + 1, NUM_WAVES);
        Text::DrawShadow("WAVE " + std::to_string(shownWave) + " / " + std::to_string(NUM_WAVES),
                         vec2(9.0f, 8.15f), 0.05f, vec3(1.0f, 0.95f, 0.8f), Text::Align::Center);
        int left = static_cast<int>(enemies.size() + spawnQueue.size());
        Text::DrawShadow("ENEMIES LEFT: " + std::to_string(left), vec2(9.0f, 7.74f), 0.034f, vec3(1.0f, 0.85f, 0.6f), Text::Align::Center);

        // ---- coin counter (top right) ----
        Draw::Plaque(vec2(13.7f, 8.0f), vec2(3.9f, 1.2f));
        Draw::Coin(vec2(12.45f, 8.0f), envTime, 1.35f);
        Text::DrawShadow(std::to_string(coins), vec2(15.25f, 7.72f), 0.075f, vec3(1.0f, 0.92f, 0.45f), Text::Align::Right);
        Text::DrawShadow("COINS", vec2(13.0f, 8.4f), 0.032f, vec3(1.0f, 0.85f, 0.6f), Text::Align::Left);

        // ---- control hints in the bottom strip ----
        Text::DrawShadow("CLICK A LANE TO SELECT   CARD OR 1-3: PLACE   W/S: JUMP / DUCK   ESC: PAUSE",
                         vec2(WORLD_WIDTH * 0.5f, 0.56f), 0.03f, vec3(1.0f, 0.95f, 0.8f), Text::Align::Center);
    }

    void DrawOverlayDim(float a)
    {
        Renderer2D::DrawQuadRot(vec2(WORLD_WIDTH * 0.5f, WORLD_HEIGHT * 0.5f), vec2(WORLD_WIDTH, WORLD_HEIGHT), 0.0f, vec3(0.03f, 0.02f, 0.05f), a);
    }
}

void Game::Render()
{
    if (state == State::Cutscene) { Cutscene::Render(); return; }

    bool inGame = state == State::Playing || state == State::Paused || state == State::Won || state == State::Lost;
    DrawScene(inGame);

    if (state == State::MainMenu) {
        // idle guardians and a curious mummy decorate the menu backdrop
        Draw::Pose a; a.foot = vec2(3.2f, LaneBottomOf(1)); a.time = envTime; Draw::Melee(a);
        Draw::Pose b; b.foot = vec2(2.3f, LaneBottomOf(0)); b.time = envTime; Draw::Archer(b);
        Draw::Pose c; c.foot = vec2(2.6f, LaneBottomOf(2)); c.time = envTime; Draw::Guardian(c);
        Draw::Pose d; d.foot = vec2(13.4f, LaneBottomOf(1)); d.facing = -1.0f; d.time = envTime; d.walk = envTime * 3.0f; d.moving = 0.0f; Draw::Mummy(d);
        Draw::Pose e; e.foot = vec2(12.4f, LaneBottomOf(2) + Draw::VULTURE_HOVER); e.facing = -1.0f; e.time = envTime; Draw::Vulture(e);
        Draw::Pose f; f.foot = vec2(14.2f, LaneBottomOf(0)); f.facing = -1.0f; f.time = envTime; Draw::Scorpion(f);
        DrawOverlayDim(0.30f);

        Draw::Plaque(vec2(WORLD_WIDTH * 0.5f, 7.55f), vec2(11.6f, 2.5f));
        Text::DrawShadow("GUARDIANS OF", vec2(WORLD_WIDTH * 0.5f, 7.62f), 0.125f, vec3(1.0f, 0.88f, 0.35f), Text::Align::Center);
        Text::DrawShadow("THE SUN DISC", vec2(WORLD_WIDTH * 0.5f, 6.62f), 0.125f, vec3(1.0f, 0.88f, 0.35f), Text::Align::Center);
        Text::DrawShadow("AN ANCIENT EGYPT TOWER DEFENSE", vec2(WORLD_WIDTH * 0.5f, 5.55f), 0.055f, vec3(1.0f, 0.95f, 0.8f), Text::Align::Center);
        DrawButtons({ "START GAME", "EXIT" }, 3.9f);
        Text::DrawShadow("SELECT LANES, PLACE GUARDIANS AND STOP THE CURSED ARMY!",
                         vec2(WORLD_WIDTH * 0.5f, 1.25f), 0.04f, vec3(1.0f, 0.95f, 0.8f), Text::Align::Center);
        return;
    }

    // ---- gameplay layers: far lanes first so nearer lanes overlap them ----
    for (int l = NUM_LANES - 1; l >= 0; --l) {
        for (const Coin& c : coinsOnField) {
            if (c.lane != l) continue;
            float drop = c.age < 0.35f ? (1.0f - c.age / 0.35f) * 0.8f : 0.0f;
            Draw::Coin(vec2(c.x, LANE_Y[l] - 0.2f + drop), c.age + static_cast<float>(l), 1.0f);
        }
        for (const Player& p : players) if (p.lane == l) DrawPlayer(p);
        for (const Enemy& e : enemies) if (e.lane == l) DrawEnemy(e);
        for (const Projectile& s : shots) if (s.lane == l) DrawShot(s);
        // health is always visible: a bar with one notch per life under every unit
        for (const Player& p : players) if (p.lane == l) Draw::HealthBar(vec2(p.x, LANE_Y[l] - 0.68f), 0.80f, p.hp, p.maxHp, true);
        for (const Enemy& e : enemies) if (e.lane == l) Draw::HealthBar(vec2(e.x, LANE_Y[l] - 0.68f), 0.80f, e.hp, e.maxHp, false);
    }
    for (const Spark& s : sparks) Draw::HitSpark(s.pos, s.t / s.life, s.color);
    for (const FloatText& f : floaters)
        Text::DrawShadow(f.text, f.pos + vec2(0.0f, 0.45f * f.t), 0.055f, f.color, Text::Align::Center, 1.0f - f.t);

    DrawCardsAndHud();

    // wave banner between waves
    if (state == State::Playing && waveState == WaveState::Intermission) {
        float a = Clamp01(std::min(waveTimer, 1.2f));
        Text::DrawShadow("WAVE " + std::to_string(waveIndex + 1), vec2(WORLD_WIDTH * 0.5f + 0.6f, 3.55f), 0.16f, vec3(1.0f, 0.88f, 0.35f), Text::Align::Center, a);
    }
    if (toastT > 0.0f) {
        float a = Clamp01(toastT * 2.0f);
        Draw::Plaque(vec2(WORLD_WIDTH * 0.5f + 0.6f, 3.8f), vec2(Text::Width(toast, 0.06f) + 0.9f, 0.8f), 0.0f, [a] { Draw::Fx f; f.alpha = a; return f; }());
        Text::DrawShadow(toast, vec2(WORLD_WIDTH * 0.5f + 0.6f, 3.62f), 0.06f, vec3(1.0f, 0.95f, 0.8f), Text::Align::Center, a);
    }

    if (state == State::Paused) {
        DrawOverlayDim(0.55f);
        Draw::Plaque(vec2(WORLD_WIDTH * 0.5f, 6.1f), vec2(7.0f, 1.5f));
        Text::DrawShadow("PAUSED", vec2(WORLD_WIDTH * 0.5f, 5.7f), 0.12f, vec3(1.0f, 0.88f, 0.35f), Text::Align::Center);
        DrawButtons({ "CONTINUE", "RESTART", "MAIN MENU" }, 4.0f);
    } else if (state == State::Won || state == State::Lost) {
        float a = Clamp01(endTimer / 0.8f);
        DrawOverlayDim(0.55f * a);
        if (endTimer > 0.9f) {
            bool won = state == State::Won;
            Draw::Plaque(vec2(WORLD_WIDTH * 0.5f, 6.0f), vec2(12.0f, 2.7f));
            Text::DrawShadow(won ? "VICTORY!" : "DEFEAT", vec2(WORLD_WIDTH * 0.5f, 6.0f), 0.115f,
                             won ? vec3(1.0f, 0.88f, 0.35f) : vec3(1.0f, 0.35f, 0.3f), Text::Align::Center);
            Text::DrawShadow(won ? "THE SUN DISC IS SAFE. DAWN WILL RISE OVER EGYPT"
                                 : "THE CURSED ARMY HAS REACHED THE SUN TEMPLE",
                             vec2(WORLD_WIDTH * 0.5f, 5.25f), 0.036f, vec3(1.0f, 0.95f, 0.8f), Text::Align::Center);
            DrawButtons({ "RESTART", "MAIN MENU" }, 3.4f);
        }
    }
}
