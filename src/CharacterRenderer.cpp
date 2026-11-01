
#include "CharacterRenderer.h"
#include "Renderer2D.h"

#include <glm/glm.hpp>
#include <cmath>
#include <vector>

// How the enemy drawings work
// ---------------------------
// 1. Every enemy is built from simple shapes (circles, polygons and thick
//    lines) drawn with Renderer2D -- no image files are needed.
//
// 2. Each character is DESIGNED FACING RIGHT (+x = forward), like a normal
//    side-view sketch. ToWorld() mirrors it when it goes on screen, so the
//    enemies face LEFT toward the player.
//
// 3. Animation uses POSES. A pose stores where the knees, feet, elbows and
//    hands are. An animation is a short list of poses (keyframes), each with
//    a time. To draw a frame we find the two keyframes around the current
//    time and blend between them, so the movement is smooth.
//
// Design units: the feet stand at y = -1, the head is around y = +0.8.

namespace CharacterRenderer
{
    // ===========================================================================
    // Size and placement
    // ===========================================================================

    const float CHARACTER_SCALE = 0.8f;   // design units -> world units
    const float KID_SCALE       = 0.82f;  // the kid is smaller than the adults
    const float GROUND_OFFSET   = -0.55f; // feet stand this far below the lane center

    const float OUTLINE_WIDTH = 0.035f;   // thickness of the dark cartoon outline

    // ===========================================================================
    // Colors
    // ===========================================================================

    const glm::vec3 OUTLINE    (0.08f, 0.05f, 0.04f);
    const glm::vec3 WHITE      (0.97f, 0.95f, 0.90f);
    const glm::vec3 MOUTH      (0.32f, 0.06f, 0.06f);
    const glm::vec3 TONGUE     (0.82f, 0.34f, 0.32f);
    const glm::vec3 HAIR       (0.10f, 0.07f, 0.05f);
    const glm::vec3 HAIR_SHINE (0.30f, 0.21f, 0.14f);
    const glm::vec3 KID_HAIR   (0.24f, 0.14f, 0.07f);
    const glm::vec3 GOLD       (0.97f, 0.75f, 0.20f);
    const glm::vec3 GOLD_DARK  (0.72f, 0.48f, 0.08f);
    const glm::vec3 METAL      (0.82f, 0.86f, 0.90f);
    const glm::vec3 METAL_DARK (0.50f, 0.55f, 0.62f);
    const glm::vec3 WOOD       (0.58f, 0.36f, 0.17f);
    const glm::vec3 DIRT       (0.52f, 0.40f, 0.26f);
    const glm::vec3 PEBBLE     (0.62f, 0.60f, 0.56f);

    // Small details drawn on each character's trousers.
    enum class LegDetail
    {
        RolledCuff,  // jeans rolled up at the ankle
        TornEdge,    // short trousers with a ripped edge
        Patch        // a sewn-on patch on the thigh
    };

    // Everything that makes one character look different from another.
    struct Look
    {
        glm::vec3 skin;
        glm::vec3 shirt;
        glm::vec3 shirtShade;
        glm::vec3 trousers;
        glm::vec3 slipper;
        float trouserLength;  // 1 = down to the ankle, smaller = shorter
        LegDetail legDetail;
        float limbSize;       // thickness multiplier (the kid is skinnier)
    };

    // Easy: dirty white tank top, blue jeans, blue plastic slippers.
    const Look EASY_LOOK = {
        glm::vec3(0.80f, 0.52f, 0.33f),
        glm::vec3(0.93f, 0.91f, 0.84f),
        glm::vec3(0.74f, 0.70f, 0.60f),
        glm::vec3(0.20f, 0.25f, 0.33f),
        glm::vec3(0.22f, 0.48f, 0.78f),
        1.0f, LegDetail::RolledCuff, 1.0f
    };

    // Medium: torn red T-shirt, short olive trousers, grey flip-flops.
    const Look KID_LOOK = {
        glm::vec3(0.84f, 0.57f, 0.37f),
        glm::vec3(0.80f, 0.16f, 0.13f),
        glm::vec3(0.56f, 0.09f, 0.08f),
        glm::vec3(0.46f, 0.44f, 0.30f),
        glm::vec3(0.40f, 0.45f, 0.52f),
        0.55f, LegDetail::TornEdge, 0.85f
    };

    // Hard: faded blue tank top, brown patched trousers, black slippers.
    const Look HARD_LOOK = {
        glm::vec3(0.66f, 0.41f, 0.25f),
        glm::vec3(0.50f, 0.64f, 0.76f),
        glm::vec3(0.35f, 0.47f, 0.58f),
        glm::vec3(0.42f, 0.31f, 0.22f),
        glm::vec3(0.18f, 0.17f, 0.16f),
        1.0f, LegDetail::Patch, 1.0f
    };

    // ===========================================================================
    // Drawing helpers
    // ===========================================================================

    // Where the character currently being drawn is placed on screen.
    glm::vec2 g_origin(0.0f, 0.0f);
    float g_scale = 1.0f;

    // Converts a design point (facing right) into a world point (facing left).
    glm::vec2 ToWorld(glm::vec2 p)
    {
        return glm::vec2(g_origin.x - p.x * g_scale, g_origin.y + p.y * g_scale);
    }

    // A filled circle.
    void Dot(glm::vec2 center, float radius, glm::vec3 color)
    {
        Renderer2D::DrawCircle(ToWorld(center), radius * g_scale, color);
    }

    // A filled polygon. Points should go around the shape in order.
    void Shape(const std::vector<glm::vec2>& points, glm::vec3 color)
    {
        std::vector<glm::vec2> world;

        for (const glm::vec2& p : points)
            world.push_back(ToWorld(p));

        Renderer2D::DrawPolygon(world.data(), static_cast<int>(world.size()), color);
    }

    // Returns a copy of a polygon pushed outward by `amount` on every side.
    // Drawing this bigger copy in dark color behind the real shape gives
    // the shape a cartoon outline.
    std::vector<glm::vec2> Grow(const std::vector<glm::vec2>& points, float amount)
    {
        const size_t count = points.size();

        // The sign of the area tells us which way the points go around.
        float area = 0.0f;
        for (size_t i = 0; i < count; ++i)
        {
            const glm::vec2& a = points[i];
            const glm::vec2& b = points[(i + 1) % count];
            area += a.x * b.y - b.x * a.y;
        }
        const float side = (area >= 0.0f) ? 1.0f : -1.0f;

        std::vector<glm::vec2> result;

        for (size_t i = 0; i < count; ++i)
        {
            const glm::vec2& prev = points[(i + count - 1) % count];
            const glm::vec2& cur  = points[i];
            const glm::vec2& next = points[(i + 1) % count];

            glm::vec2 e1 = cur - prev;
            glm::vec2 e2 = next - cur;
            if (glm::length(e1) < 0.0001f || glm::length(e2) < 0.0001f)
            {
                result.push_back(cur);
                continue;
            }
            e1 = glm::normalize(e1);
            e2 = glm::normalize(e2);

            // Outward direction of the two edges that meet at this corner.
            glm::vec2 n1 = glm::vec2(e1.y, -e1.x) * side;
            glm::vec2 n2 = glm::vec2(e2.y, -e2.x) * side;
            glm::vec2 corner = n1 + n2;

            if (glm::length(corner) < 0.0001f)
                corner = n1;

            corner = glm::normalize(corner);

            // Sharp corners need to move further to keep the outline even,
            // but not too far (or spikes get huge outlines).
            float push = amount / std::fmax(glm::dot(corner, n1), 0.4f);
            result.push_back(cur + corner * push);
        }

        return result;
    }

    // Dark outline only (draw this first, then the fill).
    void ShapeOutline(const std::vector<glm::vec2>& points)
    {
        Shape(Grow(points, OUTLINE_WIDTH), OUTLINE);
    }

    // A filled polygon with a cartoon outline.
    void OutlinedShape(const std::vector<glm::vec2>& points, glm::vec3 color)
    {
        ShapeOutline(points);
        Shape(points, color);
    }

    // A filled circle with a cartoon outline.
    void OutlinedDot(glm::vec2 center, float radius, glm::vec3 color)
    {
        Dot(center, radius + OUTLINE_WIDTH, OUTLINE);
        Dot(center, radius, color);
    }

    // A thick line with rounded ends, `widthA` wide at a and `widthB` wide at b.
    void Taper(glm::vec2 a, glm::vec2 b, float widthA, float widthB, glm::vec3 color)
    {
        glm::vec2 d = b - a;
        float length = glm::length(d);

        if (length > 0.0001f)
        {
            glm::vec2 n = glm::vec2(-d.y, d.x) / length;
            Shape({ a + n * (widthA * 0.5f), b + n * (widthB * 0.5f),
                    b - n * (widthB * 0.5f), a - n * (widthA * 0.5f) }, color);
        }

        Dot(a, widthA * 0.5f, color);
        Dot(b, widthB * 0.5f, color);
    }

    // A thick line with rounded ends.
    void Line(glm::vec2 a, glm::vec2 b, float width, glm::vec3 color)
    {
        Taper(a, b, width, width, color);
    }

    // A thick line with a cartoon outline.
    void OutlinedLine(glm::vec2 a, glm::vec2 b, float width, glm::vec3 color)
    {
        Line(a, b, width + 2.0f * OUTLINE_WIDTH, OUTLINE);
        Line(a, b, width, color);
    }

    // A bent limb (shoulder-elbow-hand or hip-knee-ankle) with an outline.
    // The outline is drawn for both parts first, so there is no dark line
    // across the elbow / knee.
    void Limb(glm::vec2 a, glm::vec2 b, glm::vec2 c,
              float widthA, float widthC, glm::vec3 color)
    {
        const float widthB = (widthA + widthC) * 0.5f;
        const float o = 2.0f * OUTLINE_WIDTH;

        Taper(a, b, widthA + o, widthB + o, OUTLINE);
        Taper(b, c, widthB + o, widthC + o, OUTLINE);
        Taper(a, b, widthA, widthB, color);
        Taper(b, c, widthB, widthC, color);
    }

    // Unit vector from a to b (or "up" if the points are on top of each other).
    glm::vec2 Direction(glm::vec2 a, glm::vec2 b)
    {
        glm::vec2 d = b - a;
        if (glm::length(d) < 0.0001f)
            return glm::vec2(0.0f, 1.0f);
        return glm::normalize(d);
    }

    // ===========================================================================
    // Poses and animations
    // ===========================================================================

    struct Pose
    {
        float bob;                      // moves the upper body up (+) / down (-)
        float lean;                     // moves the shoulders forward (+) / back (-)
        glm::vec2 nearKnee, nearFoot;   // leg closer to the camera (design space)
        glm::vec2 farKnee,  farFoot;    // leg further away
        glm::vec2 nearElbow, nearHand;  // arm closer to the camera (from the shoulder)
        glm::vec2 farElbow,  farHand;   // arm further away
        float weapon;                   // 1 = weapon ready in hand, 0 = just used
    };

    struct Keyframe
    {
        float time;
        Pose pose;
    };

    // Mixes two poses: t = 0 gives a, t = 1 gives b.
    Pose Blend(const Pose& a, const Pose& b, float t)
    {
        Pose p;
        p.bob       = glm::mix(a.bob, b.bob, t);
        p.lean      = glm::mix(a.lean, b.lean, t);
        p.nearKnee  = glm::mix(a.nearKnee, b.nearKnee, t);
        p.nearFoot  = glm::mix(a.nearFoot, b.nearFoot, t);
        p.farKnee   = glm::mix(a.farKnee, b.farKnee, t);
        p.farFoot   = glm::mix(a.farFoot, b.farFoot, t);
        p.nearElbow = glm::mix(a.nearElbow, b.nearElbow, t);
        p.nearHand  = glm::mix(a.nearHand, b.nearHand, t);
        p.farElbow  = glm::mix(a.farElbow, b.farElbow, t);
        p.farHand   = glm::mix(a.farHand, b.farHand, t);
        p.weapon    = glm::mix(a.weapon, b.weapon, t);
        return p;
    }

    // Finds the pose at `time` in a list of keyframes.
    // loop   = repeat the animation (the last keyframe must equal the first).
    // smooth = ease in and out of every keyframe (nice for attacks).
    Pose Sample(const std::vector<Keyframe>& keys, float time, bool loop, bool smooth)
    {
        const float length = keys.back().time;

        if (loop)
            time = std::fmod(time, length);

        if (time <= keys.front().time)
            return keys.front().pose;

        for (size_t i = 0; i + 1 < keys.size(); ++i)
        {
            if (time <= keys[i + 1].time)
            {
                float t = (time - keys[i].time) / (keys[i + 1].time - keys[i].time);

                if (smooth)
                    t = t * t * (3.0f - 2.0f * t);

                return Blend(keys[i].pose, keys[i + 1].pose, t);
            }
        }

        return keys.back().pose;
    }

    // ---------------------------------------------------------------------------
    // Running (shared by all three enemies)
    //
    // One leg's 4-frame running cycle. The other leg plays the same cycle
    // half a stride later, and each arm swings opposite to its leg.
    //   frame 0: foot reaches forward      frame 2: foot pushes off behind
    //   frame 1: foot under the body       frame 3: knee comes up again
    // ---------------------------------------------------------------------------

    const glm::vec2 RUN_KNEE[4]  = { { 0.24f, -0.48f}, { 0.10f, -0.58f}, {-0.10f, -0.58f}, { 0.26f, -0.46f} };
    const glm::vec2 RUN_FOOT[4]  = { { 0.40f, -0.88f}, {-0.02f, -0.98f}, {-0.40f, -0.84f}, {-0.04f, -0.70f} };
    const glm::vec2 RUN_ELBOW[4] = { { 0.14f, -0.24f}, { 0.02f, -0.28f}, {-0.22f, -0.18f}, {-0.04f, -0.28f} };
    const glm::vec2 RUN_HAND[4]  = { { 0.34f, -0.08f}, { 0.18f, -0.46f}, {-0.18f, -0.44f}, { 0.12f, -0.48f} };
    const float     RUN_BOB[4]   = { 0.03f, -0.03f, 0.03f, -0.03f };

    const float RUN_CYCLE_TIME = 0.5f;  // seconds for one full stride

    std::vector<Keyframe> MakeRunAnimation(float weapon)
    {
        std::vector<Keyframe> keys;

        for (int frame = 0; frame <= 4; ++frame)
        {
            const int nearIdx = frame % 4;
            const int farIdx  = (frame + 2) % 4;  // half a stride later

            Pose p;
            p.bob       = RUN_BOB[nearIdx];
            p.lean      = 0.08f;
            p.nearKnee  = RUN_KNEE[nearIdx];
            p.nearFoot  = RUN_FOOT[nearIdx];
            p.farKnee   = RUN_KNEE[farIdx];
            p.farFoot   = RUN_FOOT[farIdx];
            p.nearElbow = RUN_ELBOW[farIdx];   // arms swing opposite to the legs
            p.nearHand  = RUN_HAND[farIdx];
            p.farElbow  = RUN_ELBOW[nearIdx];
            p.farHand   = RUN_HAND[nearIdx];
            p.weapon    = weapon;

            keys.push_back({ frame * RUN_CYCLE_TIME / 4.0f, p });
        }

        return keys;
    }

    // ---------------------------------------------------------------------------
    // Attack and idle poses
    // ---------------------------------------------------------------------------

    // A pose with both feet planted (used while attacking or waiting).
    Pose StancePose(float bob, float lean,
                    glm::vec2 nearElbow, glm::vec2 nearHand,
                    glm::vec2 farElbow, glm::vec2 farHand, float weapon)
    {
        Pose p;
        p.bob = bob;
        p.lean = lean;
        p.nearKnee = glm::vec2( 0.16f, -0.56f);
        p.nearFoot = glm::vec2( 0.30f, -0.98f);
        p.farKnee  = glm::vec2(-0.10f, -0.58f);
        p.farFoot  = glm::vec2(-0.26f, -0.98f);
        p.nearElbow = nearElbow;
        p.nearHand  = nearHand;
        p.farElbow  = farElbow;
        p.farHand   = farHand;
        p.weapon = weapon;
        return p;
    }

    // The kid kneels down to shoot low.
    Pose CrouchPose(float lean,
                    glm::vec2 nearElbow, glm::vec2 nearHand,
                    glm::vec2 farElbow, glm::vec2 farHand, float weapon)
    {
        Pose p;
        p.bob = -0.26f;
        p.lean = lean;
        p.nearKnee = glm::vec2( 0.30f, -0.62f);
        p.nearFoot = glm::vec2( 0.30f, -0.98f);
        p.farKnee  = glm::vec2(-0.08f, -0.90f);
        p.farFoot  = glm::vec2(-0.44f, -0.96f);
        p.nearElbow = nearElbow;
        p.nearHand  = nearHand;
        p.farElbow  = farElbow;
        p.farHand   = farHand;
        p.weapon = weapon;
        return p;
    }

    // EASY: winds up, throws a slipper over his head, follows through.
    std::vector<Keyframe> MakeEasyAttack()
    {
        Pose ready  = StancePose( 0.00f,  0.02f, { 0.04f, -0.27f}, { 0.16f, -0.45f}, { 0.12f, -0.24f}, { 0.30f, -0.14f}, 1.0f);
        Pose windup = StancePose( 0.01f, -0.08f, {-0.20f,  0.18f}, {-0.24f,  0.42f}, { 0.20f, -0.14f}, { 0.44f, -0.04f}, 1.0f);
        Pose thrown = StancePose( 0.00f,  0.12f, { 0.22f,  0.14f}, { 0.46f,  0.18f}, {-0.16f, -0.22f}, {-0.18f, -0.46f}, 1.0f);
        Pose follow = StancePose(-0.03f,  0.14f, { 0.22f, -0.14f}, { 0.36f, -0.34f}, {-0.16f, -0.22f}, {-0.18f, -0.46f}, 0.0f);

        Pose released = thrown;
        released.weapon = 0.0f;  // the slipper has left his hand

        Pose end = ready;
        end.weapon = 0.0f;

        return {
            { 0.00f, ready },
            { 0.35f, windup },
            { ATTACK_HIT_TIME, thrown },
            { ATTACK_HIT_TIME + 0.01f, released },
            { 0.70f, follow },
            { ATTACK_DURATION, end }
        };
    }

    // MEDIUM: kneels, pulls the slingshot band back, releases, recoils.
    std::vector<Keyframe> MakeKidAttack()
    {
        Pose ready  = CrouchPose(0.10f, { 0.22f, -0.16f}, { 0.46f, -0.24f}, { 0.14f, -0.24f}, { 0.40f, -0.24f}, 0.0f);
        Pose aim    = CrouchPose(0.06f, { 0.22f, -0.16f}, { 0.46f, -0.24f}, {-0.16f, -0.18f}, { 0.08f, -0.20f}, 1.0f);
        Pose snap   = CrouchPose(0.08f, { 0.22f, -0.16f}, { 0.46f, -0.24f}, { 0.10f, -0.24f}, { 0.34f, -0.22f}, 0.0f);
        Pose recoil = CrouchPose(0.02f, { 0.20f, -0.12f}, { 0.42f, -0.12f}, { 0.10f, -0.24f}, { 0.32f, -0.20f}, 0.0f);
        recoil.bob = -0.24f;

        return {
            { 0.00f, ready },
            { 0.35f, aim },
            { ATTACK_HIT_TIME, aim },
            { ATTACK_HIT_TIME + 0.02f, snap },
            { 0.65f, recoil },
            { ATTACK_DURATION, ready }
        };
    }

    // HARD: raises the knife high, slashes down in front of him, recovers.
    std::vector<Keyframe> MakeHardAttack()
    {
        Pose ready  = StancePose( 0.00f,  0.06f, { 0.14f, -0.22f}, { 0.32f, -0.10f}, { 0.08f, -0.26f}, { 0.24f, -0.16f}, 1.0f);
        Pose raise  = StancePose( 0.02f, -0.06f, { 0.06f,  0.24f}, { 0.02f,  0.50f}, { 0.20f, -0.12f}, { 0.42f, -0.02f}, 1.0f);
        Pose slash  = StancePose(-0.05f,  0.18f, { 0.26f,  0.04f}, { 0.50f, -0.10f}, {-0.14f, -0.22f}, {-0.12f, -0.46f}, 1.0f);
        Pose follow = StancePose(-0.05f,  0.16f, { 0.16f, -0.22f}, { 0.30f, -0.44f}, {-0.14f, -0.22f}, {-0.12f, -0.46f}, 1.0f);

        return {
            { 0.00f, ready },
            { 0.35f, raise },
            { ATTACK_HIT_TIME, slash },
            { 0.65f, follow },
            { ATTACK_DURATION, ready }
        };
    }

    // Idle = the first attack pose, gently breathing up and down.
    std::vector<Keyframe> MakeIdleAnimation(const std::vector<Keyframe>& attack, float weapon)
    {
        Pose low = attack.front().pose;
        low.weapon = weapon;

        Pose high = low;
        high.bob += 0.025f;

        return { { 0.0f, low }, { 0.6f, high }, { 1.2f, low } };
    }

    // All animations of one enemy type.
    struct AnimationSet
    {
        std::vector<Keyframe> run;
        std::vector<Keyframe> attack;
        std::vector<Keyframe> idle;
    };

    // Built once, the first time they are needed.
    const AnimationSet& GetAnimations(EnemyType type)
    {
        static AnimationSet easy;
        static AnimationSet kid;
        static AnimationSet hard;
        static bool built = false;

        if (!built)
        {
            easy.run    = MakeRunAnimation(1.0f);   // runs holding a slipper
            easy.attack = MakeEasyAttack();
            easy.idle   = MakeIdleAnimation(easy.attack, 1.0f);

            kid.run     = MakeRunAnimation(0.0f);   // slingshot not loaded while running
            kid.attack  = MakeKidAttack();
            kid.idle    = MakeIdleAnimation(kid.attack, 0.0f);

            hard.run    = MakeRunAnimation(1.0f);   // always holds the knife
            hard.attack = MakeHardAttack();
            hard.idle   = MakeIdleAnimation(hard.attack, 1.0f);

            built = true;
        }

        switch (type)
        {
        case EnemyType::EasyRanged:   return easy;
        case EnemyType::MediumRanged: return kid;
        default:                      return hard;
        }
    }

    // Picks the right pose for an enemy at this moment.
    Pose GetPose(EnemyType type, EnemyAnim anim, float animTime)
    {
        const AnimationSet& set = GetAnimations(type);

        switch (anim)
        {
        case EnemyAnim::Attack: return Sample(set.attack, animTime, false, true);
        case EnemyAnim::Idle:   return Sample(set.idle, animTime, true, true);
        default:                return Sample(set.run, animTime, true, false);
        }
    }

    // ===========================================================================
    // Skeleton: turns a pose into actual joint positions
    // ===========================================================================

    struct Skeleton
    {
        glm::vec2 hip, nearHip, farHip;
        glm::vec2 shoulder, nearShoulder, farShoulder;
        glm::vec2 head;
        glm::vec2 nearKnee, nearFoot, farKnee, farFoot;
        glm::vec2 nearElbow, nearHand, farElbow, farHand;
    };

    Skeleton BuildSkeleton(const Pose& p)
    {
        Skeleton s;

        s.hip      = glm::vec2(0.0f, -0.18f + p.bob);
        s.shoulder = glm::vec2(0.04f + p.lean, 0.42f + p.bob);
        s.head     = s.shoulder + glm::vec2(0.08f + p.lean * 0.4f, 0.34f);

        s.nearHip = s.hip + glm::vec2( 0.04f, 0.0f);
        s.farHip  = s.hip + glm::vec2(-0.04f, 0.0f);
        s.nearShoulder = s.shoulder + glm::vec2( 0.02f, -0.03f);
        s.farShoulder  = s.shoulder + glm::vec2(-0.06f,  0.00f);

        s.nearKnee = p.nearKnee;
        s.nearFoot = p.nearFoot;
        s.farKnee  = p.farKnee;
        s.farFoot  = p.farFoot;

        s.nearElbow = s.nearShoulder + p.nearElbow;
        s.nearHand  = s.nearShoulder + p.nearHand;
        s.farElbow  = s.farShoulder + p.farElbow;
        s.farHand   = s.farShoulder + p.farHand;

        return s;
    }

    // ===========================================================================
    // Body parts (shared by all characters)
    // ===========================================================================

    // Things on the far side of the body are drawn a little darker.
    const float FAR_SHADE = 0.78f;

    // A slipper / flip-flop under the foot, pointing where the toes point.
    void DrawFoot(glm::vec2 knee, glm::vec2 ankle, glm::vec3 skin, glm::vec3 slipper)
    {
        glm::vec2 down = Direction(knee, ankle);
        glm::vec2 forward(-down.y, down.x);  // toes point this way

        // sole
        glm::vec2 heel = ankle - forward * 0.06f + down * 0.06f;
        glm::vec2 toe  = ankle + forward * 0.20f + down * 0.06f;
        OutlinedLine(heel, toe, 0.05f, slipper * 0.8f);

        // foot
        OutlinedLine(ankle, ankle + forward * 0.15f + down * 0.02f, 0.08f, skin);

        // strap over the foot
        Line(ankle + forward * 0.08f - down * 0.03f,
             ankle + forward * 0.10f + down * 0.04f, 0.05f, slipper);
    }

    void DrawLeg(glm::vec2 hip, glm::vec2 knee, glm::vec2 ankle,
                 const Look& look, bool far)
    {
        const float shade = far ? FAR_SHADE : 1.0f;
        const float t = look.limbSize;

        const glm::vec3 skin  = look.skin * shade;
        const glm::vec3 cloth = look.trousers * shade;

        const glm::vec2 down = Direction(knee, ankle);
        const glm::vec2 side(-down.y, down.x);

        // Bare shin (only visible below short trousers).
        if (look.trouserLength < 1.0f)
            Limb(knee, glm::mix(knee, ankle, 0.5f), ankle, 0.11f * t, 0.09f * t, skin);

        DrawFoot(knee, ankle, skin, look.slipper * shade);

        // Trousers: wide at the hip, narrower at the bottom.
        const glm::vec2 cut = glm::mix(knee, ankle, look.trouserLength);
        Limb(hip, knee, cut, 0.22f * t, 0.16f * t, cloth);

        switch (look.legDetail)
        {
        case LegDetail::RolledCuff:
            // a flat band, a little lighter than the jeans
            OutlinedShape({ cut + side * 0.085f, cut - down * 0.07f + side * 0.09f,
                            cut - down * 0.07f - side * 0.09f, cut - side * 0.085f },
                          glm::mix(cloth, glm::vec3(0.75f), 0.2f));
            break;

        case LegDetail::TornEdge:
            // ripped strips hanging from the edge
            for (int i = -1; i <= 1; ++i)
            {
                glm::vec2 base = cut + side * (0.055f * i);
                OutlinedShape({ base - side * 0.035f,
                                base + down * (0.06f + 0.03f * (i & 1)),
                                base + side * 0.035f }, cloth);
            }
            break;

        case LegDetail::Patch:
        {
            if (far)
                break;  // only the near knee has a patch

            glm::vec2 c = glm::mix(hip, knee, 0.9f);
            OutlinedShape({ c + glm::vec2(-0.05f, -0.05f), c + glm::vec2(0.05f, -0.05f),
                            c + glm::vec2(0.05f, 0.05f), c + glm::vec2(-0.05f, 0.05f) },
                          glm::mix(cloth, glm::vec3(0.75f, 0.62f, 0.40f), 0.45f));
            break;
        }
        }
    }

    // Upper arm + forearm. `sleeve` = draw a T-shirt sleeve over the upper arm.
    void DrawArm(glm::vec2 shoulder, glm::vec2 elbow, glm::vec2 hand,
                 const Look& look, bool far, bool sleeve)
    {
        const float shade = far ? FAR_SHADE : 1.0f;
        const float t = look.limbSize;

        Limb(shoulder, elbow, hand, 0.14f * t, 0.11f * t, look.skin * shade);

        if (sleeve)
        {
            glm::vec2 end = glm::mix(shoulder, elbow, 0.6f);
            Taper(shoulder, end, 0.21f * t + 2.0f * OUTLINE_WIDTH, 0.19f * t + 2.0f * OUTLINE_WIDTH, OUTLINE);
            Taper(shoulder, end, 0.21f * t, 0.19f * t, look.shirt * shade);
        }
    }

    void DrawFist(glm::vec2 hand, const Look& look, bool far)
    {
        const float shade = far ? FAR_SHADE : 1.0f;
        OutlinedDot(hand, 0.07f * look.limbSize, look.skin * shade);
    }

    // The top of the trousers, visible under the shirt.
    void DrawPelvis(const Skeleton& s, const Look& look)
    {
        const glm::vec2 h = s.hip;
        OutlinedShape({ h + glm::vec2(-0.20f, 0.04f), h + glm::vec2(0.20f, 0.04f),
                        h + glm::vec2(0.18f, -0.14f), h + glm::vec2(-0.18f, -0.14f) },
                      look.trousers);
    }

    void DrawNeck(const Skeleton& s, const Look& look)
    {
        OutlinedLine(s.shoulder + glm::vec2(0.03f, 0.02f),
                     s.head + glm::vec2(-0.03f, -0.12f),
                     0.12f * look.limbSize, look.skin);
    }

    // Tank top over a bare chest (Easy and Hard).
    void DrawTankTop(const Skeleton& s, const Look& look)
    {
        const glm::vec2 h = s.hip;
        const glm::vec2 sh = s.shoulder;

        // bare chest and shoulders
        OutlinedShape({ h + glm::vec2(-0.19f, -0.02f), h + glm::vec2(0.19f, -0.02f),
                        sh + glm::vec2(0.21f, -0.12f), sh + glm::vec2(0.12f, 0.08f),
                        sh + glm::vec2(-0.10f, 0.08f), sh + glm::vec2(-0.21f, -0.08f) },
                      look.skin);

        // tank top body
        OutlinedShape({ h + glm::vec2(-0.21f, -0.05f), h + glm::vec2(0.21f, -0.05f),
                        sh + glm::vec2(0.23f, -0.14f), sh + glm::vec2(0.13f, -0.06f),
                        sh + glm::vec2(-0.04f, -0.04f), sh + glm::vec2(-0.22f, -0.10f) },
                      look.shirt);

        // shadow on the back
        Shape({ h + glm::vec2(-0.19f, -0.03f), h + glm::vec2(-0.09f, -0.03f),
                sh + glm::vec2(-0.10f, -0.08f), sh + glm::vec2(-0.20f, -0.10f) },
              look.shirtShade);

        // strap over the shoulder
        OutlinedLine(sh + glm::vec2(0.06f, -0.06f), sh + glm::vec2(-0.03f, 0.07f), 0.06f, look.shirt);
    }

    // T-shirt with a ripped hem and a hole (the kid).
    void DrawTShirt(const Skeleton& s, const Look& look)
    {
        const glm::vec2 h = s.hip;
        const glm::vec2 sh = s.shoulder;
        const glm::vec3 dirty = glm::mix(look.shirt, DIRT, 0.5f);

        // ripped strips hanging under the hem
        for (int i = 0; i < 4; ++i)
        {
            float x = -0.17f + 0.09f * i;
            float drop = (i % 2 == 0) ? 0.14f : 0.10f;
            OutlinedShape({ h + glm::vec2(x, -0.04f),
                            h + glm::vec2(x + 0.05f, -0.04f - drop),
                            h + glm::vec2(x + 0.09f, -0.04f) }, look.shirt);
        }

        // shirt body
        OutlinedShape({ h + glm::vec2(-0.19f, -0.08f), h + glm::vec2(0.19f, -0.08f),
                        sh + glm::vec2(0.22f, -0.12f), sh + glm::vec2(0.14f, 0.08f),
                        sh + glm::vec2(-0.12f, 0.09f), sh + glm::vec2(-0.22f, -0.08f) },
                      look.shirt);

        // shadow on the back
        Shape({ h + glm::vec2(-0.17f, -0.06f), h + glm::vec2(-0.07f, -0.06f),
                sh + glm::vec2(-0.10f, -0.08f), sh + glm::vec2(-0.20f, -0.08f) },
              look.shirtShade);

        // ragged hole showing skin
        OutlinedShape({ h + glm::vec2(0.05f, 0.19f), h + glm::vec2(0.08f, 0.25f),
                        h + glm::vec2(0.11f, 0.22f), h + glm::vec2(0.15f, 0.24f),
                        h + glm::vec2(0.14f, 0.16f), h + glm::vec2(0.09f, 0.13f) },
                      look.skin * 0.9f);

        // dirt
        Dot(h + glm::vec2(-0.07f, 0.12f), 0.045f, dirty);
        Dot(sh + glm::vec2(0.09f, -0.22f), 0.035f, dirty);
    }

    // ===========================================================================
    // Faces
    //
    // F(x, y) gives a point on the face: (x, y) are offsets from the head
    // center for a normal head, and k makes the face bigger (the kid).
    // ===========================================================================

    struct Face
    {
        glm::vec2 center;
        float k;

        glm::vec2 F(float x, float y) const { return center + glm::vec2(x, y) * k; }
    };

    // Round skull + chin, ear, and nose.
    void DrawHeadBase(const Face& f, glm::vec3 skin)
    {
        const std::vector<glm::vec2> jaw = {
            f.F(-0.12f, -0.06f), f.F(0.19f, -0.04f), f.F(0.17f, -0.18f),
            f.F(0.05f, -0.25f), f.F(-0.10f, -0.17f)
        };

        // outlines of skull and jaw first, so they join into one silhouette
        Dot(f.center, 0.21f * f.k + OUTLINE_WIDTH, OUTLINE);
        ShapeOutline(jaw);
        Dot(f.center, 0.21f * f.k, skin);
        Shape(jaw, skin);

        // ear
        OutlinedDot(f.F(-0.09f, -0.01f), 0.05f * f.k, skin);
        Dot(f.F(-0.085f, -0.01f), 0.022f * f.k, skin * 0.75f);
    }

    void DrawNose(const Face& f, glm::vec3 skin)
    {
        OutlinedShape({ f.F(0.19f, 0.03f), f.F(0.28f, -0.06f),
                        f.F(0.23f, -0.09f), f.F(0.17f, -0.07f) }, skin);
    }

    // Wide-open eyes with angry eyebrows pointing down toward the nose.
    void DrawAngryEyes(const Face& f)
    {
        OutlinedDot(f.F(0.10f, 0.04f), 0.050f * f.k, WHITE);
        Dot(f.F(0.12f, 0.035f), 0.022f * f.k, OUTLINE);

        OutlinedDot(f.F(0.19f, 0.045f), 0.036f * f.k, WHITE);
        Dot(f.F(0.205f, 0.04f), 0.018f * f.k, OUTLINE);

        Line(f.F(0.03f, 0.125f), f.F(0.15f, 0.085f), 0.045f * f.k, HAIR);
        Line(f.F(0.175f, 0.09f), f.F(0.24f, 0.12f), 0.038f * f.k, HAIR);
    }

    // Shouting open mouth with teeth and tongue.
    void DrawShoutMouth(const Face& f)
    {
        OutlinedShape({ f.F(0.07f, -0.11f), f.F(0.22f, -0.10f),
                        f.F(0.20f, -0.20f), f.F(0.10f, -0.20f) }, MOUTH);
        Shape({ f.F(0.08f, -0.115f), f.F(0.21f, -0.105f),
                f.F(0.205f, -0.13f), f.F(0.085f, -0.135f) }, WHITE);
        Shape({ f.F(0.11f, -0.19f), f.F(0.19f, -0.19f),
                f.F(0.18f, -0.165f), f.F(0.12f, -0.165f) }, TONGUE);
    }

    // Cheeky wide grin.
    void DrawGrin(const Face& f)
    {
        OutlinedShape({ f.F(0.05f, -0.09f), f.F(0.23f, -0.08f),
                        f.F(0.19f, -0.16f), f.F(0.09f, -0.17f) }, MOUTH);
        Shape({ f.F(0.065f, -0.095f), f.F(0.22f, -0.085f),
                f.F(0.20f, -0.115f), f.F(0.075f, -0.12f) }, WHITE);
    }

    // EASY: undercut with messy spiky top, short beard, gold earring.
    void DrawEasyHead(const Face& f, const Look& look)
    {
        DrawHeadBase(f, look.skin);

        // short beard along the jaw
        Shape({ f.F(-0.10f, -0.06f), f.F(0.04f, -0.06f), f.F(0.22f, -0.07f),
                f.F(0.18f, -0.19f), f.F(0.05f, -0.255f), f.F(-0.09f, -0.17f) },
              glm::mix(look.skin, HAIR, 0.55f));

        // shaved sides
        Shape({ f.F(-0.20f, 0.04f), f.F(-0.16f, 0.15f), f.F(-0.04f, 0.20f),
                f.F(0.08f, 0.19f), f.F(0.00f, 0.08f), f.F(-0.13f, 0.02f) },
              glm::mix(look.skin, HAIR, 0.35f));

        DrawAngryEyes(f);
        DrawNose(f, look.skin);
        DrawShoutMouth(f);

        Dot(f.F(-0.09f, -0.06f), 0.016f, GOLD);  // earring

        // messy hair on top: outlines of every piece first, then the fills
        const std::vector<std::vector<glm::vec2>> hair = {
            { f.F(-0.14f, 0.13f), f.F(-0.04f, 0.24f), f.F(0.10f, 0.26f),
              f.F(0.22f, 0.19f), f.F(0.20f, 0.12f), f.F(0.02f, 0.14f) },
            { f.F(-0.12f, 0.17f), f.F(-0.22f, 0.31f), f.F(-0.02f, 0.23f) },
            { f.F(-0.04f, 0.22f), f.F(0.00f, 0.38f), f.F(0.08f, 0.24f) },
            { f.F(0.05f, 0.24f), f.F(0.17f, 0.36f), f.F(0.16f, 0.22f) },
            { f.F(0.14f, 0.22f), f.F(0.31f, 0.27f), f.F(0.21f, 0.14f) }
        };

        for (const auto& piece : hair) ShapeOutline(piece);
        for (const auto& piece : hair) Shape(piece, HAIR);

        Line(f.F(-0.02f, 0.22f), f.F(0.08f, 0.29f), 0.02f, HAIR_SHINE);
        Line(f.F(0.10f, 0.23f), f.F(0.17f, 0.30f), 0.02f, HAIR_SHINE);
    }

    // MEDIUM: round messy mop of brown hair with shaved lines on the side,
    // dirt on the cheek, cheeky grin.
    void DrawKidHead(const Face& f, const Look& look)
    {
        DrawHeadBase(f, look.skin);

        // hair cap (stays above the eyebrows)
        const std::vector<glm::vec2> cap = {
            f.F(-0.22f, 0.00f), f.F(-0.19f, 0.17f), f.F(-0.07f, 0.24f),
            f.F(0.09f, 0.24f), f.F(0.19f, 0.19f), f.F(0.21f, 0.15f), f.F(-0.10f, 0.12f)
        };

        // big round tufts on top
        const glm::vec2 tufts[] = {
            {-0.16f, 0.14f}, {-0.09f, 0.22f}, { 0.01f, 0.25f}, { 0.11f, 0.23f}, { 0.18f, 0.18f}
        };

        ShapeOutline(cap);
        for (const glm::vec2& c : tufts)
            Dot(f.F(c.x, c.y), 0.085f * f.k + OUTLINE_WIDTH, OUTLINE);

        Shape(cap, KID_HAIR);
        for (const glm::vec2& c : tufts)
            Dot(f.F(c.x, c.y), 0.085f * f.k, KID_HAIR);

        // a cowlick sticking up, and shine
        OutlinedShape({ f.F(-0.03f, 0.29f), f.F(0.02f, 0.40f), f.F(0.05f, 0.28f) }, KID_HAIR);
        Line(f.F(0.02f, 0.27f), f.F(0.11f, 0.29f), 0.02f, glm::mix(KID_HAIR, WHITE, 0.25f));

        // shaved lines on the side
        const glm::vec3 shaved = glm::mix(look.skin, KID_HAIR, 0.3f);
        Line(f.F(-0.18f, 0.10f), f.F(-0.10f, 0.14f), 0.018f, shaved);
        Line(f.F(-0.19f, 0.05f), f.F(-0.11f, 0.09f), 0.018f, shaved);

        DrawAngryEyes(f);
        DrawNose(f, look.skin);
        DrawGrin(f);

        Dot(f.F(0.11f, -0.04f), 0.03f * f.k, glm::mix(look.skin, DIRT, 0.55f));  // dirt smudge
    }

    // HARD: messy curly hair, thick moustache, stubble.
    void DrawHardHead(const Face& f, const Look& look)
    {
        DrawHeadBase(f, look.skin);

        // stubble
        Shape({ f.F(-0.10f, -0.07f), f.F(0.20f, -0.06f), f.F(0.17f, -0.18f),
                f.F(0.05f, -0.25f), f.F(-0.09f, -0.17f) },
              glm::mix(look.skin, HAIR, 0.20f));

        DrawAngryEyes(f);
        DrawNose(f, look.skin);
        DrawShoutMouth(f);

        // thick moustache over the mouth
        OutlinedLine(f.F(0.09f, -0.095f), f.F(0.24f, -0.09f), 0.05f, HAIR);
        Dot(f.F(0.08f, -0.12f), 0.025f, HAIR);

        // curly hair: overlapping outlined circles look like curls
        const glm::vec2 curls[] = {
            {-0.20f, 0.03f}, {-0.18f, 0.12f}, {-0.12f, 0.20f}, {-0.03f, 0.25f},
            { 0.07f, 0.26f}, { 0.15f, 0.23f}, { 0.20f, 0.17f}, {-0.08f, 0.17f}, { 0.04f, 0.20f}
        };

        for (const glm::vec2& c : curls)
            OutlinedDot(f.F(c.x, c.y), 0.075f * f.k, HAIR);

        for (const glm::vec2& c : curls)
            Dot(f.F(c.x + 0.02f, c.y + 0.03f), 0.018f * f.k, HAIR_SHINE);
    }

    // ===========================================================================
    // Weapons
    // ===========================================================================

    // A slipper centered at `center`, pointing along `axis`.
    void SlipperShape(glm::vec2 center, glm::vec2 axis, glm::vec3 color)
    {
        glm::vec2 side(-axis.y, axis.x);

        OutlinedLine(center - axis * 0.11f, center + axis * 0.11f, 0.10f, color);
        Line(center - axis * 0.08f, center + axis * 0.08f, 0.04f, color * 0.75f);  // footprint

        // V-shaped strap
        glm::vec2 front = center + axis * 0.07f;
        Line(front, center - axis * 0.02f + side * 0.045f, 0.025f, color * 0.55f);
        Line(front, center - axis * 0.02f - side * 0.045f, 0.025f, color * 0.55f);
    }

    // Big cartoon kitchen knife held in the fist, blade along the forearm.
    void DrawKnife(glm::vec2 elbow, glm::vec2 hand)
    {
        glm::vec2 dir = Direction(elbow, hand);
        glm::vec2 n(-dir.y, dir.x);  // the blunt back of the blade is on this side

        // handle
        OutlinedLine(hand - dir * 0.07f, hand + dir * 0.05f, 0.06f, glm::vec3(0.16f, 0.11f, 0.08f));

        // blade (wide at the handle, pointed at the tip)
        OutlinedShape({ hand + dir * 0.06f + n * 0.035f,
                        hand + dir * 0.40f + n * 0.030f,
                        hand + dir * 0.47f + n * 0.020f,
                        hand + dir * 0.38f - n * 0.040f,
                        hand + dir * 0.06f - n * 0.075f }, METAL);

        // shine along the back of the blade
        Line(hand + dir * 0.09f + n * 0.012f, hand + dir * 0.38f + n * 0.010f, 0.012f, WHITE);
        // metal ring between handle and blade
        Line(hand + dir * 0.06f + n * 0.03f, hand + dir * 0.06f - n * 0.07f, 0.025f, METAL_DARK);
    }

    // Y-shaped wooden slingshot in the hand. The band goes back to the
    // pulling hand when loaded, and hangs between the prongs when not.
    void DrawSlingshot(glm::vec2 hand, glm::vec2 pullHand, bool loaded)
    {
        glm::vec2 fork = hand + glm::vec2(0.0f, 0.05f);
        glm::vec2 tipA = hand + glm::vec2(0.04f, 0.18f);
        glm::vec2 tipB = hand + glm::vec2(-0.05f, 0.16f);

        OutlinedLine(hand + glm::vec2(0.0f, -0.08f), fork, 0.04f, WOOD);
        OutlinedLine(fork, tipA, 0.035f, WOOD);
        OutlinedLine(fork, tipB, 0.035f, WOOD);

        glm::vec2 pouch = loaded ? pullHand : hand + glm::vec2(-0.02f, 0.12f);
        const glm::vec3 band(0.30f, 0.16f, 0.10f);
        Line(tipA, pouch, 0.018f, band);
        Line(tipB, pouch, 0.018f, band);

        if (loaded)
            OutlinedDot(pouch, 0.035f, PEBBLE);
    }

    // ===========================================================================
    // The three characters
    // ===========================================================================

    void DrawEasy(const Pose& p)
    {
        const Look& look = EASY_LOOK;
        const Skeleton s = BuildSkeleton(p);

        DrawArm(s.farShoulder, s.farElbow, s.farHand, look, true, false);
        DrawFist(s.farHand, look, true);
        DrawLeg(s.farHip, s.farKnee, s.farFoot, look, true);
        DrawLeg(s.nearHip, s.nearKnee, s.nearFoot, look, false);
        DrawPelvis(s, look);
        DrawTankTop(s, look);

        // stains on the white tank top
        const glm::vec3 stain = glm::mix(look.shirt, DIRT, 0.45f);
        Dot(s.hip + glm::vec2(0.07f, 0.22f), 0.035f, stain);
        Dot(s.hip + glm::vec2(0.13f, 0.10f), 0.022f, stain);
        Dot(s.shoulder + glm::vec2(0.12f, -0.30f), 0.028f, stain);

        DrawNeck(s, look);

        // gold chain hanging on the chest
        const glm::vec2 sh = s.shoulder;
        for (int i = 0; i <= 8; ++i)
        {
            float t = i / 8.0f;
            glm::vec2 a = sh + glm::vec2(-0.03f, 0.05f);
            glm::vec2 c = sh + glm::vec2(0.07f, -0.22f);
            glm::vec2 b = sh + glm::vec2(0.15f, 0.00f);
            glm::vec2 pos = glm::mix(glm::mix(a, c, t), glm::mix(c, b, t), t);
            Dot(pos, 0.024f, (i % 2 == 0) ? GOLD : GOLD_DARK);
        }
        OutlinedDot(sh + glm::vec2(0.07f, -0.13f), 0.03f, GOLD);

        DrawEasyHead({ s.head, 1.0f }, look);

        // near arm: watch, and the slipper ready to throw
        DrawArm(s.nearShoulder, s.nearElbow, s.nearHand, look, false, false);

        glm::vec2 wrist = glm::mix(s.nearElbow, s.nearHand, 0.72f);
        Dot(wrist, 0.065f, OUTLINE);
        Dot(wrist, 0.035f, METAL);

        if (p.weapon > 0.5f)
        {
            glm::vec2 dir = Direction(s.nearElbow, s.nearHand);
            SlipperShape(s.nearHand + dir * 0.10f, dir, look.slipper);
        }

        DrawFist(s.nearHand, look, false);
    }

    void DrawKid(const Pose& p)
    {
        const Look& look = KID_LOOK;
        const Skeleton s = BuildSkeleton(p);

        DrawLeg(s.farHip, s.farKnee, s.farFoot, look, true);
        DrawLeg(s.nearHip, s.nearKnee, s.nearFoot, look, false);
        DrawPelvis(s, look);
        DrawTShirt(s, look);
        DrawNeck(s, look);
        DrawKidHead({ s.head, 1.15f }, look);

        // Both arms are in front: one pulls the band, one holds the slingshot.
        DrawArm(s.farShoulder, s.farElbow, s.farHand, look, true, true);
        DrawFist(s.farHand, look, true);

        DrawArm(s.nearShoulder, s.nearElbow, s.nearHand, look, false, true);
        DrawSlingshot(s.nearHand, s.farHand, p.weapon > 0.5f);
        DrawFist(s.nearHand, look, false);
    }

    void DrawHard(const Pose& p)
    {
        const Look& look = HARD_LOOK;
        const Skeleton s = BuildSkeleton(p);

        DrawArm(s.farShoulder, s.farElbow, s.farHand, look, true, false);
        DrawFist(s.farHand, look, true);
        DrawLeg(s.farHip, s.farKnee, s.farFoot, look, true);
        DrawLeg(s.nearHip, s.nearKnee, s.nearFoot, look, false);
        DrawPelvis(s, look);
        DrawTankTop(s, look);

        // sweat stains
        Dot(s.shoulder + glm::vec2(0.02f, -0.17f), 0.06f, look.shirtShade);
        Dot(s.hip + glm::vec2(0.10f, 0.24f), 0.045f, look.shirtShade);

        DrawNeck(s, look);
        DrawHardHead({ s.head, 1.0f }, look);

        DrawArm(s.nearShoulder, s.nearElbow, s.nearHand, look, false, false);
        DrawKnife(s.nearElbow, s.nearHand);
        DrawFist(s.nearHand, look, false);
    }

    // ===========================================================================
    // Public functions
    // ===========================================================================

    // Sets where the next character is drawn and how big it is.
    void PlaceCharacter(float x, float laneY, EnemyType type)
    {
        g_scale = CHARACTER_SCALE;
        if (type == EnemyType::MediumRanged)
            g_scale *= KID_SCALE;

        // Lift the drawing so the soles of the slippers touch the ground line.
        g_origin = glm::vec2(x, laneY + GROUND_OFFSET + 1.08f * g_scale);
    }

    void Draw(float x, float y, EnemyType type, EnemyAnim anim, float animTime)
    {
        PlaceCharacter(x, y, type);

        const Pose pose = GetPose(type, anim, animTime);

        switch (type)
        {
        case EnemyType::EasyRanged:
            DrawEasy(pose);
            break;

        case EnemyType::MediumRanged:
            DrawKid(pose);
            break;

        case EnemyType::HardMelee:
            DrawHard(pose);
            break;
        }
    }

    glm::vec2 GetAttackPoint(float x, float y, EnemyType type)
    {
        PlaceCharacter(x, y, type);

        const Skeleton s = BuildSkeleton(GetPose(type, EnemyAnim::Attack, ATTACK_HIT_TIME));

        // The slingshot's pebble starts from the fork, a bit above the hand.
        if (type == EnemyType::MediumRanged)
            return ToWorld(s.nearHand + glm::vec2(0.0f, 0.14f));

        return ToWorld(s.nearHand);
    }

    void DrawSlipper(float x, float y, float spin)
    {
        g_scale = CHARACTER_SCALE;
        g_origin = glm::vec2(x, y);
        SlipperShape(glm::vec2(0.0f), glm::vec2(std::cos(spin), std::sin(spin)), EASY_LOOK.slipper);
    }

    void DrawPebble(float x, float y)
    {
        g_scale = CHARACTER_SCALE;
        g_origin = glm::vec2(x, y);
        OutlinedDot(glm::vec2(0.0f), 0.045f, PEBBLE);
        Dot(glm::vec2(-0.012f, 0.015f), 0.015f, WHITE);
    }
}
