#include "EnemySystem.h"
#include "GameConfig.h"
#include "CharacterRenderer.h"
#include "Renderer2D.h"
#include "HealthBar.h"
#include "Audio.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>

namespace EnemySystem
{
    using CharacterRenderer::EnemyAnim;

    // ------------------------------------------------------------------
    // Attack settings (world units and seconds)
    // ------------------------------------------------------------------

    // The knife guy runs until he is this close to a player.
    const float MELEE_ATTACK_RANGE = CONTACT_DIST;

    // Waiting time between two attacks.
    const float RANGED_COOLDOWN = 3.4f;   // slipper / slingshot
    const float MELEE_COOLDOWN  = 1.3f;   // knife

    const float PROJECTILE_SPEED = 4.5f;
    const float HIT_DISTANCE     = 0.30f;   // how close a projectile must get to a player

    // Flight height of projectiles, measured from the lane center.
    const float HIGH_FLIGHT_Y =  0.62f;     // head height: duck to dodge
    const float LOW_FLIGHT_Y  = -0.42f;     // foot height: jump to dodge

    // Information stored for every enemy.
    struct Enemy
    {
        float x;
        int lane;
        float speed;
        EnemyType type;
        int health;
        int maxHealth;
        int damage;

        EnemyAnim anim;      // Run, Attack or Idle
        float animTime;      // seconds since the current animation started
        float cooldown;      // time left before the next attack
        bool hasHit;         // true once the current attack has fired / landed
        float flash;         // short flash after being hit (1 -> 0)
    };

    // A slipper or pebble flying toward the koshk.
    struct Projectile
    {
        float x;
        float y;
        float flightY;       // height the projectile settles at
        int lane;
        int damage;
        AttackHeight height;
        float spin;          // slipper rotation (radians)
    };

    // A player in a lane (filled by the player system every frame).
    struct PlayerSpot
    {
        int id;
        int lane;
        float x;
        bool ducking;
        bool jumping;
        bool defense;
    };

    std::vector<Enemy> enemies;
    std::vector<Projectile> projectiles;
    std::vector<PlayerSpot> players;
    std::vector<EnemyHit> hits;
    std::vector<Defeated> defeated;
    bool reachedBase = false;

    int GetEnemyCount() { return static_cast<int>(enemies.size()); }
    bool ReachedBase() { return reachedBase; }

    int GetEnemyCountInLane(int lane)
    {
        int n = 0;
        for (const Enemy& e : enemies) if (e.lane == lane) ++n;
        return n;
    }
    void ClearPlayers() { players.clear(); }

    void AddPlayer(int id, int lane, float x, bool ducking, bool jumping, bool isDefense)
    {
        players.push_back({ id, lane, x, ducking, jumping, isDefense });
    }

    const std::vector<EnemyHit>& GetHits() { return hits; }
    const std::vector<Defeated>& GetDefeated() { return defeated; }

    int TypeIndex(EnemyType type) { return static_cast<int>(type); }

    // Sets enemy properties according to its type (numbers in GameConfig.h).
    void ConfigureEnemy(Enemy& enemy, EnemyType type)
    {
        const int i = TypeIndex(type);
        enemy.type = type;
        enemy.health = ENEMY_MAX_HP[i];
        enemy.maxHealth = ENEMY_MAX_HP[i];
        enemy.damage = ENEMY_DAMAGE[i];
        enemy.speed = ENEMY_SPEED[i];
    }

    void Init()
    {
        enemies.clear();
        projectiles.clear();
        players.clear();
        hits.clear();
        defeated.clear();
        reachedBase = false;

        std::srand(static_cast<unsigned int>(std::time(nullptr)));
    }

    // Creates a new enemy at the right edge, in a random lane.
    void SpawnEnemy(EnemyType type)
    {
        Enemy enemy{};
        enemy.x = ENEMY_SPAWN_X;
        enemy.lane = std::rand() % NUM_LANES;
        ConfigureEnemy(enemy, type);

        enemy.anim = EnemyAnim::Run;
        enemy.animTime = 0.0f;
        // Ranged enemies start attacking almost as soon as they appear.
        enemy.cooldown = (type == EnemyType::HardMelee) ? 0.0f : 1.2f;
        enemy.hasHit = false;
        enemy.flash = 0.0f;

        enemies.push_back(enemy);
    }

    // Finds the closest player in front of the enemy (to its left) in the
    // same lane. Returns its index, or -1 if there is none.
    int FindTarget(const Enemy& enemy)
    {
        int best = -1;
        for (size_t i = 0; i < players.size(); ++i)
        {
            const PlayerSpot& p = players[i];
            if (p.lane != enemy.lane || p.x > enemy.x + 0.2f)
                continue;
            if (best < 0 || p.x > players[best].x)
                best = static_cast<int>(i);
        }
        return best;
    }

    bool EnemyInContact(int lane, float playerX)
    {
        for (const Enemy& e : enemies)
            if (e.lane == lane && e.x - playerX >= -0.2f && e.x - playerX <= CONTACT_DIST)
                return true;
        return false;
    }

    bool HitEnemyInRange(int lane, float fromX, float toX, int damage, glm::vec2* hitPoint)
    {
        Enemy* best = nullptr;
        for (Enemy& e : enemies)
        {
            if (e.lane != lane || e.health <= 0 || e.x < fromX || e.x > toX)
                continue;
            if (!best || e.x < best->x)
                best = &e;
        }
        if (!best)
            return false;

        best->health -= damage;
        best->flash = 1.0f;
        if (hitPoint)
            *hitPoint = glm::vec2(best->x, LANE_Y[lane] + 0.15f);
        return true;
    }

    void StartAttack(Enemy& enemy)
    {
        enemy.anim = EnemyAnim::Attack;
        enemy.animTime = 0.0f;
        enemy.hasHit = false;
    }

    // Called at the moment of the attack animation where the weapon is used.
    void PerformAttack(const Enemy& enemy, int target)
    {
        const float laneY = LANE_Y[enemy.lane];

        // Knife: hits right away, if the player is still close enough.
        if (enemy.type == EnemyType::HardMelee)
        {
            Audio::Play(Audio::Sfx::Knife, 0.8f);
            if (target >= 0 && enemy.x - players[target].x <= MELEE_ATTACK_RANGE + 0.15f)
                hits.push_back({ players[target].id, enemy.damage, AttackHeight::Melee });
            return;
        }

        // Slipper or pebble: starts at the hand and flies left, even if
        // nobody is in the lane yet.
        glm::vec2 start = CharacterRenderer::GetAttackPoint(enemy.x, laneY, enemy.type);

        Projectile p;
        p.x = start.x;
        p.y = start.y;
        p.lane = enemy.lane;
        p.damage = enemy.damage;
        p.spin = 0.0f;

        if (enemy.type == EnemyType::EasyRanged)
        {
            p.height = AttackHeight::High;
            p.flightY = laneY + HIGH_FLIGHT_Y;
            Audio::Play(Audio::Sfx::SlipperThrow, 0.7f);
        }
        else
        {
            p.height = AttackHeight::Low;
            p.flightY = laneY + LOW_FLIGHT_Y;
            Audio::Play(Audio::Sfx::Slingshot, 0.7f);
        }

        projectiles.push_back(p);
    }

    // Decides what one enemy does this frame: run, attack or wait.
    void UpdateEnemy(Enemy& enemy, float deltaTime)
    {
        const int target = FindTarget(enemy);
        const bool touching = target >= 0 && (enemy.x - players[target].x) <= CONTACT_DIST;
        const bool isMelee = enemy.type == EnemyType::HardMelee;

        enemy.cooldown -= deltaTime;
        enemy.flash = std::max(0.0f, enemy.flash - deltaTime * 4.0f);

        switch (enemy.anim)
        {
        case EnemyAnim::Run:
            if (touching)
            {
                // Met a player: stop. Attack as soon as the weapon is ready.
                enemy.anim = EnemyAnim::Idle;
                enemy.animTime = 0.0f;
            }
            else if (!isMelee && enemy.cooldown <= 0.0f)
            {
                // Ranged enemies stop for a moment to throw, then walk on.
                StartAttack(enemy);
            }
            else
            {
                enemy.x -= enemy.speed * deltaTime;   // keep walking left
            }
            break;

        case EnemyAnim::Attack:
            if (!enemy.hasHit && enemy.animTime >= CharacterRenderer::ATTACK_HIT_TIME)
            {
                enemy.hasHit = true;
                PerformAttack(enemy, target);
            }
            if (enemy.animTime >= CharacterRenderer::ATTACK_DURATION)
            {
                enemy.cooldown = isMelee ? MELEE_COOLDOWN : RANGED_COOLDOWN;
                enemy.anim = touching ? EnemyAnim::Idle : EnemyAnim::Run;
                enemy.animTime = 0.0f;
            }
            break;

        case EnemyAnim::Idle:
            if (!touching)
            {
                enemy.anim = EnemyAnim::Run;   // the player is gone: walk again
                enemy.animTime = 0.0f;
            }
            else if (enemy.cooldown <= 0.0f)
            {
                StartAttack(enemy);
            }
            break;
        }

        // Faster enemies move their legs faster while running.
        if (enemy.anim == EnemyAnim::Run)
            enemy.animTime += deltaTime * enemy.speed / 1.2f;
        else
            enemy.animTime += deltaTime;
    }

    // Moves projectiles and checks if they reached a player.
    void UpdateProjectiles(float deltaTime)
    {
        for (auto it = projectiles.begin(); it != projectiles.end();)
        {
            Projectile& p = *it;

            p.x -= PROJECTILE_SPEED * deltaTime;
            p.y += (p.flightY - p.y) * std::min(1.0f, deltaTime * 8.0f);   // settle at flight height
            p.spin += deltaTime * 14.0f;

            // It smacks into the koshk wall and falls.
            bool remove = (p.x < HOME_BASE_RIGHT);

            // The first player it reaches (the one furthest right) takes the
            // hit, unless he dodges. A Defense player cannot dodge, so
            // players standing behind him are shielded.
            int best = -1;
            for (size_t i = 0; i < players.size(); ++i)
            {
                const PlayerSpot& pl = players[i];
                if (pl.lane != p.lane || std::fabs(p.x - pl.x) > HIT_DISTANCE)
                    continue;
                if (!pl.defense)
                {
                    if (p.height == AttackHeight::High && pl.ducking) continue;
                    if (p.height == AttackHeight::Low && pl.jumping) continue;
                }
                if (best < 0 || pl.x > players[best].x)
                    best = static_cast<int>(i);
            }
            if (best >= 0)
            {
                hits.push_back({ players[best].id, p.damage, p.height });
                Audio::Play(p.height == AttackHeight::High ? Audio::Sfx::SlipperHit : Audio::Sfx::PebbleHit, 0.8f);
                remove = true;
            }

            if (remove)
                it = projectiles.erase(it);
            else
                ++it;
        }
    }

    void Update(float deltaTime)
    {
        hits.clear();
        defeated.clear();

        for (Enemy& enemy : enemies)
        {
            UpdateEnemy(enemy, deltaTime);
            if (enemy.x <= BASE_REACHED_X)
                reachedBase = true;
        }

        UpdateProjectiles(deltaTime);

        // Defeated enemies leave the scene and drop coins.
        for (auto it = enemies.begin(); it != enemies.end();)
        {
            if (it->health <= 0)
            {
                defeated.push_back({ it->lane, it->x, it->type });
                Audio::Play(Audio::Sfx::EnemyDown, 0.9f);
                it = enemies.erase(it);
            }
            else
                ++it;
        }
    }

    void RenderLane(int lane)
    {
        for (const Enemy& enemy : enemies)
        {
            if (enemy.lane != lane)
                continue;

            const float y = LANE_Y[enemy.lane];

            // Shadow on the ground.
            Renderer2D::DrawEllipse(glm::vec2(enemy.x, y - FOOT_OFFSET), glm::vec2(0.36f, 0.07f), glm::vec3(0.45f, 0.36f, 0.25f));

            // Red tint grows as health drops, plus a short flash on each hit.
            float lost = 1.0f - static_cast<float>(enemy.health) / static_cast<float>(enemy.maxHealth);
            Renderer2D::SetDamage(std::min(0.85f, 0.55f * lost + 0.45f * enemy.flash));
            CharacterRenderer::Draw(enemy.x, y, enemy.type, enemy.anim, enemy.animTime);
            Renderer2D::SetDamage(0.0f);

            HealthBar::Draw(glm::vec2(enemy.x, y - FOOT_OFFSET - 0.17f), enemy.health, enemy.maxHealth, false);
        }
    }

    void RenderProjectiles()
    {
        for (const auto& p : projectiles)
        {
            if (p.height == AttackHeight::High)
                CharacterRenderer::DrawSlipper(p.x, p.y, p.spin);
            else
                CharacterRenderer::DrawPebble(p.x, p.y);
        }
    }

    void Shutdown()
    {
        enemies.clear();
        projectiles.clear();
        hits.clear();
        defeated.clear();
        reachedBase = false;
    }
}
