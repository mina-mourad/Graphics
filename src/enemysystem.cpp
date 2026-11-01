
#include "EnemySystem.h"
#include "GameConfig.h"
#include "CharacterRenderer.h"
#include "Renderer2D.h"
#include "Input.h"

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

    // Ranged enemies stop as soon as a player stands anywhere in front of
    // them in their lane. Lower this (e.g. 6.0f) to make them walk closer first.
    const float RANGED_ATTACK_RANGE = WORLD_WIDTH;

    // The knife guy runs until he is this close to a player.
    const float MELEE_ATTACK_RANGE = 0.9f;

    // Waiting time between two attacks (the Idle animation plays meanwhile).
    const float ATTACK_COOLDOWN = 1.2f;

    const float PROJECTILE_SPEED = 6.0f;
    const float HIT_DISTANCE     = 0.35f;   // how close a projectile must get to a player

    // Flight height of projectiles, measured from the lane center.
    const float HIGH_FLIGHT_Y =  0.75f;     // head height: duck to dodge
    const float LOW_FLIGHT_Y  = -0.42f;     // foot height: jump to dodge

    // Information stored for every enemy.
    struct Enemy
    {
        float x;
        int lane;
        float speed;
        EnemyType type;
        int health;
        int damage;

        EnemyAnim anim;      // Run, Attack or Idle
        float animTime;      // seconds since the current animation started
        float cooldown;      // time left before the next attack
        bool hasHit;         // true once the current attack has fired / landed
    };

    // A slipper or pebble flying toward the player.
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

    // A player position in a lane (filled by the player system).
    struct PlayerSpot
    {
        int lane;
        float x;
    };

    // Stores all active enemies.
    std::vector<Enemy> enemies;
    std::vector<Projectile> projectiles;
    std::vector<PlayerSpot> players;
    std::vector<EnemyHit> hits;

    // ------------------------------------------------------------------
    // TEMPORARY TEST HELPER
    // Press T in-game to place a training dummy in every lane, so the
    // attacks can be tested before the real player system exists.
    // Delete this block (and its uses below) once players can be placed.
    // ------------------------------------------------------------------
    const float TEST_DUMMY_X = 2.5f;
    bool testDummiesOn = false;
    bool previousT = false;
    float dummyFlash[NUM_LANES] = {};

    void UpdateTestDummies(float deltaTime)
    {
        bool tDown = Input::IsKeyDown(GLFW_KEY_T);
        if (tDown && !previousT)
            testDummiesOn = !testDummiesOn;
        previousT = tDown;

        for (int lane = 0; lane < NUM_LANES; ++lane)
            dummyFlash[lane] = std::max(0.0f, dummyFlash[lane] - deltaTime);
    }

    void RenderTestDummies()
    {
        if (!testDummiesOn)
            return;

        for (int lane = 0; lane < NUM_LANES; ++lane)
        {
            float y = LANE_Y[lane];
            float flash = dummyFlash[lane] > 0.0f ? 1.0f : 0.0f;

            Renderer2D::DrawQuad(glm::vec2(TEST_DUMMY_X, y - 0.15f), glm::vec2(0.08f, 0.9f), glm::vec3(0.40f, 0.26f, 0.14f));
            Renderer2D::DrawQuad(glm::vec2(TEST_DUMMY_X, y + 0.05f), glm::vec2(0.45f, 0.65f), glm::vec3(0.80f, 0.72f, 0.52f), flash);
            Renderer2D::DrawCircle(glm::vec2(TEST_DUMMY_X, y + 0.55f), 0.2f, glm::vec3(0.80f, 0.72f, 0.52f));
        }
    }
    // ------------------------------------------------------------------

    // Returns the number of active enemies.
    int GetEnemyCount()
    {
        return static_cast<int>(enemies.size());
    }

    void ClearPlayers()
    {
        players.clear();
    }

    void AddPlayer(int lane, float x)
    {
        players.push_back({ lane, x });
    }

    const std::vector<EnemyHit>& GetHits()
    {
        return hits;
    }

    // Sets enemy properties according to its type.
    void ConfigureEnemy(Enemy& enemy, EnemyType type)
    {
        enemy.type = type;

        switch (type)
        {
        case EnemyType::EasyRanged:
            enemy.health = 5;
            enemy.damage = 1;
            enemy.speed = 2.0f;
            break;

        case EnemyType::MediumRanged:
            enemy.health = 5;
            enemy.damage = 1;
            enemy.speed = 2.0f;
            break;

        case EnemyType::HardMelee:
            enemy.health = 7;
            enemy.damage = 2;
            enemy.speed = 3.0f;
            break;
        }
    }

    // Initializes the enemy system.
    void Init()
    {
        enemies.clear();
        projectiles.clear();
        players.clear();
        hits.clear();

        // Initialize random number generation.
        std::srand(
            static_cast<unsigned int>(std::time(nullptr))
        );
    }

    // Creates a new enemy.
    void SpawnEnemy(EnemyType type)
    {
        Enemy enemy{};

        // Spawn enemy on the right side.
        enemy.x = ENEMY_SPAWN_X;

        // Select one of the three lanes randomly.
        enemy.lane = std::rand() % NUM_LANES;

        // Set health, damage, speed, and type.
        ConfigureEnemy(enemy, type);

        // Every enemy starts running.
        enemy.anim = EnemyAnim::Run;
        enemy.animTime = 0.0f;
        enemy.cooldown = 0.0f;
        enemy.hasHit = false;

        // Add the enemy to the active enemies list.
        enemies.push_back(enemy);
    }

    // All players the enemies can see this frame (real players + test dummies).
    std::vector<PlayerSpot> GetTargets()
    {
        std::vector<PlayerSpot> targets = players;

        if (testDummiesOn)
        {
            for (int lane = 0; lane < NUM_LANES; ++lane)
                targets.push_back({ lane, TEST_DUMMY_X });
        }

        return targets;
    }

    // Finds the closest player in front of the enemy (to its left) in the
    // same lane. Returns false if there is none.
    bool FindTarget(const Enemy& enemy, const std::vector<PlayerSpot>& targets, float& targetX)
    {
        bool found = false;

        for (const PlayerSpot& player : targets)
        {
            if (player.lane != enemy.lane || player.x >= enemy.x)
                continue;

            if (!found || player.x > targetX)
            {
                targetX = player.x;
                found = true;
            }
        }

        return found;
    }

    // Records a hit (and flashes the test dummy if that is what was hit).
    void AddHit(int lane, float playerX, int damage, AttackHeight height)
    {
        hits.push_back({ lane, playerX, damage, height });

        if (testDummiesOn && playerX == TEST_DUMMY_X)
            dummyFlash[lane] = 0.2f;
    }

    void StartAttack(Enemy& enemy)
    {
        enemy.anim = EnemyAnim::Attack;
        enemy.animTime = 0.0f;
        enemy.hasHit = false;
    }

    // Called at the moment of the attack animation where the weapon is used.
    void PerformAttack(const Enemy& enemy, bool hasTarget, float targetX)
    {
        const float laneY = LANE_Y[enemy.lane];

        // Knife: hits right away, if the player is still close enough.
        if (enemy.type == EnemyType::HardMelee)
        {
            if (hasTarget && enemy.x - targetX <= MELEE_ATTACK_RANGE + 0.1f)
                AddHit(enemy.lane, targetX, enemy.damage, AttackHeight::Melee);
            return;
        }

        // Slipper or pebble: starts at the hand and flies left.
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
        }
        else
        {
            p.height = AttackHeight::Low;
            p.flightY = laneY + LOW_FLIGHT_Y;
        }

        projectiles.push_back(p);
    }

    // Decides what one enemy does this frame: run, attack or wait.
    void UpdateEnemy(Enemy& enemy, const std::vector<PlayerSpot>& targets, float deltaTime)
    {
        float targetX = 0.0f;
        bool hasTarget = FindTarget(enemy, targets, targetX);

        float range = (enemy.type == EnemyType::HardMelee)
            ? MELEE_ATTACK_RANGE
            : RANGED_ATTACK_RANGE;

        bool inRange = hasTarget && (enemy.x - targetX) <= range;

        switch (enemy.anim)
        {
        case EnemyAnim::Run:
            if (inRange)
                StartAttack(enemy);
            else
                enemy.x -= enemy.speed * deltaTime;  // keep walking left
            break;

        case EnemyAnim::Attack:
            // Use the weapon at the right moment of the animation.
            if (!enemy.hasHit && enemy.animTime >= CharacterRenderer::ATTACK_HIT_TIME)
            {
                enemy.hasHit = true;
                PerformAttack(enemy, hasTarget, targetX);
            }

            // Attack finished: wait a little before the next one.
            if (enemy.animTime >= CharacterRenderer::ATTACK_DURATION)
            {
                enemy.anim = EnemyAnim::Idle;
                enemy.animTime = 0.0f;
                enemy.cooldown = ATTACK_COOLDOWN;
            }
            break;

        case EnemyAnim::Idle:
            enemy.cooldown -= deltaTime;

            if (!inRange)
            {
                // The player is gone: start walking again.
                enemy.anim = EnemyAnim::Run;
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
            enemy.animTime += deltaTime * enemy.speed / 2.0f;
        else
            enemy.animTime += deltaTime;
    }

    // Moves projectiles and checks if they reached a player.
    void UpdateProjectiles(const std::vector<PlayerSpot>& targets, float deltaTime)
    {
        for (auto it = projectiles.begin(); it != projectiles.end();)
        {
            Projectile& p = *it;

            p.x -= PROJECTILE_SPEED * deltaTime;
            p.y += (p.flightY - p.y) * std::min(1.0f, deltaTime * 8.0f);  // settle at flight height
            p.spin += deltaTime * 14.0f;

            bool remove = (p.x < -0.5f);  // left the screen

            for (const PlayerSpot& player : targets)
            {
                if (player.lane == p.lane && std::fabs(p.x - player.x) <= HIT_DISTANCE)
                {
                    AddHit(p.lane, player.x, p.damage, p.height);
                    remove = true;
                    break;
                }
            }

            if (remove)
                it = projectiles.erase(it);
            else
                ++it;
        }
    }

    // Updates enemy movement and attacks.
    void Update(float deltaTime)
    {
        hits.clear();

        UpdateTestDummies(deltaTime);

        const std::vector<PlayerSpot> targets = GetTargets();

        for (auto& enemy : enemies)
        {
            UpdateEnemy(enemy, targets, deltaTime);
        }

        UpdateProjectiles(targets, deltaTime);

        // Remove enemies that reach the base.
        for (auto it = enemies.begin(); it != enemies.end();)
        {
            if (it->x <= 0.5f)
            {
                it = enemies.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    // Draws all active enemies and their projectiles.
    void Render()
    {
        RenderTestDummies();

        for (const auto& enemy : enemies)
        {
            CharacterRenderer::Draw(
                enemy.x,
                LANE_Y[enemy.lane],
                enemy.type,
                enemy.anim,
                enemy.animTime
            );
        }

        for (const auto& p : projectiles)
        {
            if (p.height == AttackHeight::High)
                CharacterRenderer::DrawSlipper(p.x, p.y, p.spin);
            else
                CharacterRenderer::DrawPebble(p.x, p.y);
        }
    }

    // Clears enemy data.
    void Shutdown()
    {
        enemies.clear();
        projectiles.clear();
        hits.clear();
    }
}
