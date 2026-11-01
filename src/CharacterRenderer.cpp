
#include "CharacterRenderer.h"
#include "Renderer2D.h"

#include <glm/glm.hpp>
#include <vector>
#include <cmath>
#include <initializer_list>

namespace CharacterRenderer
{
    // ---------------------------------------------------------------------------
    // Shared colors
    // ---------------------------------------------------------------------------
    const glm::vec3 OUTLINE     (0.04f,  0.03f,  0.025f);
    const glm::vec3 SKIN        (0.76f,  0.39f,  0.22f);
    const glm::vec3 SKIN_LIGHT  (0.94f,  0.58f,  0.35f);
    const glm::vec3 SKIN_DARK   (0.48f,  0.23f,  0.14f);

    const glm::vec3 HAIR        (0.045f, 0.035f, 0.03f);
    const glm::vec3 HAIR_LIGHT  (0.18f,  0.13f,  0.09f);

    const glm::vec3 SHIRT       (0.88f,  0.84f,  0.72f);  // tank-top white/cream
    const glm::vec3 SHIRT_DARK  (0.63f,  0.59f,  0.49f);

    const glm::vec3 RED_SHIRT   (0.72f,  0.12f,  0.10f);  // slingshot red shirt
    const glm::vec3 RED_DARK    (0.48f,  0.07f,  0.06f);
    const glm::vec3 RED_DIRT    (0.36f,  0.22f,  0.10f);  // dirt patches

    const glm::vec3 PANTS       (0.12f,  0.16f,  0.14f);
    const glm::vec3 PANTS_LIGHT (0.23f,  0.27f,  0.23f);
    const glm::vec3 PANTS_TORN  (0.18f,  0.22f,  0.18f);

    const glm::vec3 GOLD        (0.95f,  0.63f,  0.12f);
    const glm::vec3 METAL       (0.72f,  0.78f,  0.80f);
    const glm::vec3 WHITE       (0.98f,  0.96f,  0.89f);

    // Hard fat enemy colors
    const glm::vec3 FAT_SHIRT   (0.14f,  0.13f,  0.18f);  // dark shirt
    const glm::vec3 FAT_SHIRT_L (0.24f,  0.23f,  0.30f);
    const glm::vec3 BELLY       (0.65f,  0.33f,  0.18f);  // exposed belly

    // Wood / club
    const glm::vec3 WOOD        (0.36f,  0.22f,  0.10f);

    // Sandal
    const glm::vec3 SANDAL      (0.55f,  0.38f,  0.20f);

    // ---------------------------------------------------------------------------
    // Drawing helpers  (all in character-relative coordinates)
    // ---------------------------------------------------------------------------

    void Polygon(float x, float y,
                 std::initializer_list<glm::vec2> points,
                 glm::vec3 color)
    {
        std::vector<glm::vec2> worldPoints;
        for (const glm::vec2& p : points)
            worldPoints.push_back(glm::vec2(x + p.x, y + p.y));
        Renderer2D::DrawPolygon(worldPoints.data(),
                                static_cast<int>(worldPoints.size()),
                                color);
    }

    void DrawSegment(float x, float y,
                     glm::vec2 start, glm::vec2 end,
                     float thickness, glm::vec3 color)
    {
        glm::vec2 d = end - start;
        float len = glm::length(d);
        if (len <= 0.0001f) return;

        glm::vec2 n(-d.y / len, d.x / len);
        n *= thickness * 0.5f;

        Polygon(x, y, {
            start + n, end + n,
            end   - n, start - n
        }, color);
    }

    void Circle(float x, float y, glm::vec2 pos, float r, glm::vec3 color)
    {
        Renderer2D::DrawCircle(glm::vec2(x + pos.x, y + pos.y), r, color);
    }

    // ===========================================================================
    // EASY ENEMY  –  Street knife guy, FACING LEFT
    // Tank top, gold chain, messy spiky hair, knife raised in the left hand.
    // All X coordinates are flipped from the original right-facing design.
    // ===========================================================================

    void DrawEasy(float x, float y)
    {
        // --- Back leg (right leg swung back) ---
        glm::vec2 hipB (-0.12f, -0.25f);
        glm::vec2 kneeB(-0.42f, -0.54f);
        glm::vec2 ankB (-0.73f, -0.65f);

        DrawSegment(x, y, hipB,  kneeB, 0.27f, PANTS);
        DrawSegment(x, y, kneeB, ankB,  0.20f, PANTS);
        DrawSegment(x, y, ankB,  glm::vec2(-0.88f, -0.48f), 0.12f, SKIN_LIGHT);

        // sandal (back)
        Polygon(x, y, {
            {-0.78f, -0.61f},
            {-0.96f, -0.45f},
            {-1.00f, -0.53f},
            {-0.84f, -0.72f}
        }, SANDAL);

        // --- Front leg (left leg kicking forward) ---
        glm::vec2 hipF ( 0.10f, -0.25f);
        glm::vec2 kneeF( 0.35f, -0.57f);
        glm::vec2 ankF  ( 0.63f, -0.86f);

        DrawSegment(x, y, hipF,  kneeF, 0.30f, PANTS);
        DrawSegment(x, y, kneeF, ankF,  0.21f, PANTS);

        // knee skin patch
        Polygon(x, y, {
            { 0.27f, -0.53f},
            { 0.15f, -0.57f},
            { 0.25f, -0.67f},
            { 0.38f, -0.62f}
        }, SKIN_LIGHT);

        DrawSegment(x, y, ankF, glm::vec2(0.83f, -0.91f), 0.13f, SKIN_LIGHT);

        // sandal (front)
        Polygon(x, y, {
            { 0.77f, -0.85f},
            { 0.91f, -0.88f},
            { 1.00f, -0.96f},
            { 0.79f, -1.02f},
            { 0.68f, -0.93f}
        }, SANDAL);

        // --- Torso (tank top, facing left) ---
        Polygon(x, y, {  // neck
            { 0.19f,  0.48f},
            {-0.09f,  0.48f},
            {-0.10f,  0.76f},
            { 0.20f,  0.76f}
        }, SKIN);

        Polygon(x, y, {  // shirt body
            { 0.34f,  0.49f},
            { 0.20f,  0.62f},
            {-0.17f,  0.60f},
            {-0.39f,  0.42f},
            {-0.28f,  0.02f},
            {-0.16f, -0.34f},
            { 0.34f, -0.32f},
            { 0.51f,  0.12f}
        }, SHIRT);

        Polygon(x, y, {  // shirt shadow
            {-0.16f,  0.44f},
            {-0.34f,  0.32f},
            {-0.22f, -0.27f},
            {-0.02f, -0.30f}
        }, SHIRT_DARK);

        Polygon(x, y, {  // shirt fold
            { 0.30f,  0.03f},
            { 0.05f, -0.02f},
            { 0.25f, -0.09f},
            { 0.39f, -0.06f}
        }, SHIRT_DARK);

        Polygon(x, y, {  // waistband
            { 0.35f, -0.31f},
            {-0.20f, -0.34f},
            {-0.24f, -0.43f},
            { 0.36f, -0.41f}
        }, OUTLINE);

        // --- Arms ---
        // back arm (right arm, relaxed / fist)
        DrawSegment(x, y, glm::vec2(-0.26f, 0.42f), glm::vec2(-0.61f, 0.20f), 0.23f, SKIN);
        DrawSegment(x, y, glm::vec2(-0.61f, 0.20f), glm::vec2(-0.47f, -0.02f), 0.17f, SKIN_LIGHT);
        Circle(x, y, glm::vec2(-0.47f, -0.02f), 0.11f, SKIN);

        // front arm (left arm, raised – holding knife)
        DrawSegment(x, y, glm::vec2( 0.39f, 0.40f), glm::vec2( 0.68f, 0.16f), 0.22f, SKIN);
        DrawSegment(x, y, glm::vec2( 0.68f, 0.16f), glm::vec2( 0.62f, 0.42f), 0.16f, SKIN_LIGHT);
        Circle(x, y, glm::vec2( 0.62f, 0.42f), 0.13f, SKIN);

        // finger hints on raised hand
        Circle(x, y, glm::vec2( 0.70f, 0.44f), 0.055f, SKIN_LIGHT);
        Circle(x, y, glm::vec2( 0.59f, 0.48f), 0.055f, SKIN_LIGHT);

        // knife blade (pointing up)
        Polygon(x, y, {
            { 0.70f, 0.49f},
            { 0.80f, 0.94f},
            { 0.91f, 1.00f},
            { 0.87f, 0.46f}
        }, METAL);

        // knife handle
        Polygon(x, y, {
            { 0.73f, 0.48f},
            { 0.86f, 0.46f},
            { 0.88f, 0.27f},
            { 0.74f, 0.27f}
        }, OUTLINE);

        // gold necklace
        Circle(x, y, glm::vec2( 0.11f, 0.43f), 0.19f,  GOLD);
        Circle(x, y, glm::vec2( 0.11f, 0.46f), 0.145f, SHIRT);

        // gold bracelet (on raised hand)
        Circle(x, y, glm::vec2( 0.62f, 0.40f), 0.14f,  GOLD);
        Circle(x, y, glm::vec2( 0.62f, 0.40f), 0.105f, SKIN);

        // --- Head (facing left) ---
        Circle(x, y, glm::vec2( 0.20f, 0.83f), 0.29f, SKIN_LIGHT);

        Polygon(x, y, {  // jaw
            { 0.46f, 0.79f},
            { 0.36f, 0.57f},
            { 0.12f, 0.49f},
            {-0.05f, 0.64f},
            {-0.06f, 0.81f}
        }, SKIN);

        // ear
        Circle(x, y, glm::vec2(-0.04f, 0.79f), 0.10f,  SKIN);
        Circle(x, y, glm::vec2(-0.05f, 0.79f), 0.045f, SKIN_DARK);

        // eyes
        Circle(x, y, glm::vec2( 0.40f, 0.91f), 0.090f, OUTLINE);
        Circle(x, y, glm::vec2( 0.40f, 0.91f), 0.073f, WHITE);
        Circle(x, y, glm::vec2( 0.22f, 0.90f), 0.085f, OUTLINE);
        Circle(x, y, glm::vec2( 0.22f, 0.90f), 0.068f, WHITE);
        Circle(x, y, glm::vec2( 0.43f, 0.91f), 0.030f, OUTLINE);
        Circle(x, y, glm::vec2( 0.25f, 0.90f), 0.030f, OUTLINE);

        // eyebrows (angry)
        Polygon(x, y, {
            { 0.49f, 1.02f},
            { 0.27f, 1.07f},
            { 0.24f, 1.02f},
            { 0.46f, 0.98f}
        }, HAIR);
        Polygon(x, y, {
            { 0.27f, 1.04f},
            { 0.09f, 0.98f},
            { 0.10f, 0.94f},
            { 0.29f, 0.99f}
        }, HAIR);

        // nose
        Polygon(x, y, {
            { 0.49f, 0.83f},
            { 0.55f, 0.73f},
            { 0.44f, 0.68f},
            { 0.35f, 0.74f}
        }, SKIN);

        // open mouth
        Polygon(x, y, {
            { 0.43f, 0.62f},
            { 0.25f, 0.65f},
            { 0.20f, 0.54f},
            { 0.34f, 0.49f},
            { 0.43f, 0.54f}
        }, OUTLINE);
        Polygon(x, y, {
            { 0.40f, 0.61f},
            { 0.26f, 0.63f},
            { 0.25f, 0.59f},
            { 0.39f, 0.57f}
        }, WHITE);

        // messy spiky hair
        Polygon(x, y, {
            { 0.50f, 0.97f},
            { 0.58f, 1.16f},
            { 0.43f, 1.10f},
            { 0.37f, 1.25f},
            { 0.25f, 1.14f},
            { 0.13f, 1.25f},
            { 0.04f, 1.15f},
            {-0.09f, 1.18f},
            {-0.01f, 1.02f}
        }, HAIR);
        Polygon(x, y, {
            { 0.48f, 1.10f},
            { 0.38f, 1.20f},
            { 0.29f, 1.13f},
            { 0.40f, 1.12f}
        }, HAIR_LIGHT);
    }

    // ===========================================================================
    // MEDIUM ENEMY  –  Slingshot boy, FACING LEFT
    // Dirty red torn shirt, grey-green torn pants, sandals,
    // both hands stretched right holding slingshot aimed left.
    // ===========================================================================

    void DrawMedium(float x, float y)
    {
        // --- Back leg (right, pushed back in run stride) ---
        DrawSegment(x, y,
            glm::vec2(-0.10f, -0.22f),
            glm::vec2(-0.38f, -0.52f),
            0.25f, PANTS_TORN);
        DrawSegment(x, y,
            glm::vec2(-0.38f, -0.52f),
            glm::vec2(-0.58f, -0.80f),
            0.18f, PANTS_TORN);

        // torn pant edge
        Polygon(x, y, {
            {-0.52f, -0.76f},
            {-0.65f, -0.78f},
            {-0.68f, -0.86f},
            {-0.54f, -0.84f}
        }, PANTS);

        // sandal (back)
        Polygon(x, y, {
            {-0.53f, -0.80f},
            {-0.75f, -0.85f},
            {-0.88f, -0.94f},
            {-0.60f, -0.96f}
        }, SANDAL);

        // --- Front leg (left, kicking forward) ---
        DrawSegment(x, y,
            glm::vec2( 0.08f, -0.22f),
            glm::vec2( 0.32f, -0.53f),
            0.25f, PANTS_TORN);
        DrawSegment(x, y,
            glm::vec2( 0.32f, -0.53f),
            glm::vec2( 0.55f, -0.82f),
            0.18f, PANTS_TORN);

        // torn pant edge (front)
        Polygon(x, y, {
            { 0.48f, -0.76f},
            { 0.62f, -0.78f},
            { 0.65f, -0.87f},
            { 0.51f, -0.85f}
        }, PANTS);

        // sandal (front)
        Polygon(x, y, {
            { 0.50f, -0.80f},
            { 0.70f, -0.84f},
            { 0.83f, -0.93f},
            { 0.58f, -0.96f}
        }, SANDAL);

        // --- Torso (red torn shirt) ---
        Polygon(x, y, {
            { 0.28f,  0.55f},
            { 0.14f,  0.65f},
            {-0.15f,  0.63f},
            {-0.36f,  0.42f},
            {-0.28f, -0.28f},
            { 0.35f, -0.26f},
            { 0.48f,  0.12f}
        }, RED_SHIRT);

        Polygon(x, y, {  // dark shadow
            {-0.10f,  0.45f},
            {-0.28f,  0.33f},
            {-0.20f, -0.20f},
            {-0.02f, -0.23f}
        }, RED_DARK);

        // dirt patches
        Polygon(x, y, {
            { 0.15f,  0.10f},
            { 0.30f,  0.08f},
            { 0.28f, -0.05f},
            { 0.13f, -0.03f}
        }, RED_DIRT);
        Polygon(x, y, {
            {-0.18f, 0.20f},
            {-0.06f, 0.18f},
            {-0.08f, 0.07f},
            {-0.20f, 0.09f}
        }, RED_DIRT);

        // shirt tear at bottom
        Polygon(x, y, {
            {-0.28f, -0.26f},
            {-0.20f, -0.22f},
            {-0.22f, -0.35f},
            {-0.30f, -0.31f}
        }, RED_DARK);

        // waistband
        Polygon(x, y, {
            { 0.34f, -0.26f},
            {-0.29f, -0.28f},
            {-0.32f, -0.36f},
            { 0.36f, -0.34f}
        }, OUTLINE);

        // neck
        Polygon(x, y, {
            { 0.16f, 0.50f},
            {-0.08f, 0.50f},
            {-0.08f, 0.72f},
            { 0.16f, 0.72f}
        }, SKIN);

        // --- Arms (both extended right, pulling slingshot elastic) ---

        // right arm (back) – holds the slingshot handle
        DrawSegment(x, y,
            glm::vec2(-0.32f, 0.37f),
            glm::vec2(-0.62f, 0.28f),
            0.18f, SKIN);
        Circle(x, y, glm::vec2(-0.62f, 0.28f), 0.10f, SKIN_LIGHT);

        // left arm (front) – pulls elastic back
        DrawSegment(x, y,
            glm::vec2( 0.30f, 0.38f),
            glm::vec2( 0.58f, 0.25f),
            0.16f, SKIN);
        Circle(x, y, glm::vec2( 0.58f, 0.25f), 0.09f, SKIN_LIGHT);

        // --- Slingshot (Y-shape aiming LEFT) ---
        // handle
        DrawSegment(x, y,
            glm::vec2(-0.62f, 0.28f),
            glm::vec2(-0.88f, 0.24f),
            0.07f, WOOD);

        // Y-fork top prong
        DrawSegment(x, y,
            glm::vec2(-0.88f, 0.24f),
            glm::vec2(-1.04f, 0.42f),
            0.06f, WOOD);

        // Y-fork bottom prong
        DrawSegment(x, y,
            glm::vec2(-0.88f, 0.24f),
            glm::vec2(-1.04f, 0.07f),
            0.06f, WOOD);

        // elastic band – top (from prong to pulling hand)
        DrawSegment(x, y,
            glm::vec2(-1.04f, 0.42f),
            glm::vec2( 0.58f, 0.28f),
            0.025f, OUTLINE);

        // elastic band – bottom
        DrawSegment(x, y,
            glm::vec2(-1.04f, 0.07f),
            glm::vec2( 0.58f, 0.28f),
            0.025f, OUTLINE);

        // stone / pouch at tension point
        Circle(x, y, glm::vec2( 0.58f, 0.28f), 0.07f, SKIN_DARK);

        // --- Head (facing LEFT, small boy) ---
        Circle(x, y, glm::vec2( 0.18f, 0.87f), 0.30f, SKIN_LIGHT);

        Polygon(x, y, {  // jaw
            { 0.44f, 0.83f},
            { 0.37f, 0.60f},
            { 0.12f, 0.54f},
            {-0.06f, 0.68f},
            {-0.06f, 0.84f}
        }, SKIN);

        // dirt smudge on cheek
        Polygon(x, y, {
            { 0.28f, 0.72f},
            { 0.38f, 0.70f},
            { 0.36f, 0.62f},
            { 0.26f, 0.64f}
        }, RED_DIRT);
        Circle(x, y, glm::vec2( 0.10f, 0.80f), 0.055f, RED_DIRT);

        // ear
        Circle(x, y, glm::vec2(-0.08f, 0.83f), 0.09f, SKIN);

        // eyes (wide open / angry)
        Circle(x, y, glm::vec2( 0.37f, 0.93f), 0.080f, OUTLINE);
        Circle(x, y, glm::vec2( 0.37f, 0.93f), 0.064f, WHITE);
        Circle(x, y, glm::vec2( 0.18f, 0.92f), 0.075f, OUTLINE);
        Circle(x, y, glm::vec2( 0.18f, 0.92f), 0.060f, WHITE);
        Circle(x, y, glm::vec2( 0.40f, 0.93f), 0.026f, OUTLINE);
        Circle(x, y, glm::vec2( 0.21f, 0.92f), 0.026f, OUTLINE);

        // angry eyebrows
        Polygon(x, y, {
            { 0.46f, 1.03f},
            { 0.25f, 1.08f},
            { 0.23f, 1.02f},
            { 0.44f, 0.98f}
        }, HAIR);
        Polygon(x, y, {
            { 0.25f, 1.05f},
            { 0.07f, 0.99f},
            { 0.09f, 0.95f},
            { 0.27f, 1.00f}
        }, HAIR);

        // nose
        Polygon(x, y, {
            { 0.46f, 0.85f},
            { 0.52f, 0.76f},
            { 0.42f, 0.72f},
            { 0.33f, 0.77f}
        }, SKIN);

        // open shouting mouth
        Polygon(x, y, {
            { 0.42f, 0.64f},
            { 0.22f, 0.67f},
            { 0.17f, 0.57f},
            { 0.32f, 0.52f},
            { 0.42f, 0.57f}
        }, OUTLINE);
        Polygon(x, y, {
            { 0.39f, 0.63f},
            { 0.24f, 0.65f},
            { 0.23f, 0.60f},
            { 0.38f, 0.58f}
        }, WHITE);

        // messy dark hair
        Polygon(x, y, {
            {-0.10f, 1.00f},
            {-0.06f, 1.17f},
            { 0.08f, 1.10f},
            { 0.16f, 1.27f},
            { 0.28f, 1.16f},
            { 0.38f, 1.24f},
            { 0.46f, 1.12f},
            { 0.50f, 0.98f}
        }, HAIR);
        Polygon(x, y, {
            { 0.10f, 1.12f},
            { 0.20f, 1.22f},
            { 0.28f, 1.14f},
            { 0.18f, 1.11f}
        }, HAIR_LIGHT);
    }

    // ===========================================================================
    // HARD ENEMY  –  Large scary FAT melee guy, FACING LEFT
    // Huge exposed belly, dark heavy clothing, bald head with stubble,
    // swings a thick spiked wooden club.  Wide and intimidating.
    // ===========================================================================

    void DrawHard(float x, float y)
    {
        // --- Back leg (right, planted) ---
        DrawSegment(x, y,
            glm::vec2(-0.10f, -0.18f),
            glm::vec2(-0.35f, -0.52f),
            0.48f, PANTS);
        DrawSegment(x, y,
            glm::vec2(-0.35f, -0.52f),
            glm::vec2(-0.50f, -0.90f),
            0.36f, PANTS);

        // heavy boot (back)
        Polygon(x, y, {
            {-0.44f, -0.86f},
            {-0.70f, -0.92f},
            {-0.90f, -1.05f},
            {-0.52f, -1.08f}
        }, OUTLINE);
        Polygon(x, y, {
            {-0.44f, -0.90f},
            {-0.68f, -0.94f},
            {-0.80f, -1.02f},
            {-0.52f, -1.04f}
        }, PANTS_LIGHT);

        // --- Front leg (left, stepping forward) ---
        DrawSegment(x, y,
            glm::vec2( 0.10f, -0.18f),
            glm::vec2( 0.38f, -0.50f),
            0.48f, PANTS);
        DrawSegment(x, y,
            glm::vec2( 0.38f, -0.50f),
            glm::vec2( 0.52f, -0.88f),
            0.36f, PANTS);

        // heavy boot (front)
        Polygon(x, y, {
            { 0.46f, -0.84f},
            { 0.72f, -0.90f},
            { 0.92f, -1.03f},
            { 0.54f, -1.06f}
        }, OUTLINE);
        Polygon(x, y, {
            { 0.46f, -0.88f},
            { 0.70f, -0.92f},
            { 0.82f, -1.00f},
            { 0.54f, -1.02f}
        }, PANTS_LIGHT);

        // --- Exposed FAT BELLY (big protruding gut) ---
        Polygon(x, y, {
            {-0.60f,  0.10f},
            {-0.75f, -0.08f},
            {-0.72f, -0.28f},
            {-0.55f, -0.30f},
            { 0.55f, -0.30f},
            { 0.65f, -0.10f},
            { 0.55f,  0.10f}
        }, BELLY);

        // belly fold / navel
        Polygon(x, y, {
            {-0.50f, -0.10f},
            {-0.38f, -0.08f},
            {-0.40f, -0.18f},
            {-0.52f, -0.20f}
        }, SKIN_DARK);

        // belly shadow under gut
        Polygon(x, y, {
            {-0.70f, -0.20f},
            { 0.50f, -0.20f},
            { 0.58f, -0.30f},
            {-0.60f, -0.30f}
        }, SKIN_DARK);

        // --- Torso (dark shirt, very wide) ---
        Polygon(x, y, {
            {-0.60f,  0.48f},
            {-0.44f,  0.74f},
            { 0.40f,  0.74f},
            { 0.68f,  0.44f},
            { 0.65f,  0.08f},
            { 0.55f, -0.05f},
            {-0.62f,  0.08f},
            {-0.72f,  0.22f}
        }, FAT_SHIRT);

        // shirt creases
        Polygon(x, y, {
            {-0.55f, 0.60f},
            {-0.30f, 0.68f},
            {-0.26f, 0.32f},
            {-0.50f, 0.28f}
        }, FAT_SHIRT_L);
        Polygon(x, y, {
            { 0.06f, 0.68f},
            { 0.36f, 0.60f},
            { 0.38f, 0.28f},
            { 0.08f, 0.30f}
        }, FAT_SHIRT_L);

        // neck (thick)
        Polygon(x, y, {
            {-0.26f, 0.60f},
            { 0.18f, 0.60f},
            { 0.18f, 0.82f},
            {-0.26f, 0.82f}
        }, SKIN_DARK);

        // --- HEAD (large round, bald + stubble) ---
        Circle(x, y, glm::vec2( 0.00f, 1.04f), 0.42f, SKIN);

        Polygon(x, y, {  // jaw / chin
            {-0.38f, 1.00f},
            {-0.30f, 0.70f},
            {-0.04f, 0.60f},
            { 0.22f, 0.72f},
            { 0.28f, 1.02f}
        }, SKIN_LIGHT);

        // stubble / beard shadow
        Polygon(x, y, {
            {-0.35f, 0.88f},
            {-0.26f, 0.70f},
            {-0.02f, 0.62f},
            { 0.20f, 0.74f},
            { 0.26f, 0.90f}
        }, SKIN_DARK);

        // bald head sheen
        Circle(x, y, glm::vec2( 0.08f, 1.18f), 0.14f, SKIN_LIGHT);

        // small mean eyes (squinting)
        Circle(x, y, glm::vec2(-0.26f, 1.08f), 0.065f, OUTLINE);
        Circle(x, y, glm::vec2(-0.26f, 1.08f), 0.050f, WHITE);
        Circle(x, y, glm::vec2( 0.00f, 1.08f), 0.065f, OUTLINE);
        Circle(x, y, glm::vec2( 0.00f, 1.08f), 0.050f, WHITE);
        Circle(x, y, glm::vec2(-0.28f, 1.08f), 0.022f, OUTLINE);
        Circle(x, y, glm::vec2(-0.02f, 1.08f), 0.022f, OUTLINE);

        // heavy slanted eyebrows (angry)
        Polygon(x, y, {
            {-0.38f, 1.20f},
            {-0.14f, 1.24f},
            {-0.13f, 1.17f},
            {-0.40f, 1.14f}
        }, HAIR);
        Polygon(x, y, {
            {-0.14f, 1.22f},
            { 0.06f, 1.16f},
            { 0.04f, 1.10f},
            {-0.16f, 1.15f}
        }, HAIR);

        // wide flat nose
        Polygon(x, y, {
            {-0.35f, 0.98f},
            {-0.48f, 0.86f},
            {-0.30f, 0.78f},
            {-0.16f, 0.86f}
        }, SKIN_DARK);
        Circle(x, y, glm::vec2(-0.38f, 0.86f), 0.055f, SKIN_DARK);
        Circle(x, y, glm::vec2(-0.22f, 0.86f), 0.055f, SKIN_DARK);

        // grimace mouth (teeth showing)
        Polygon(x, y, {
            {-0.36f, 0.74f},
            {-0.04f, 0.74f},
            {-0.06f, 0.64f},
            {-0.34f, 0.64f}
        }, OUTLINE);
        Polygon(x, y, {
            {-0.34f, 0.73f},
            {-0.06f, 0.73f},
            {-0.07f, 0.67f},
            {-0.33f, 0.67f}
        }, WHITE);

        // ear
        Circle(x, y, glm::vec2( 0.38f, 1.04f), 0.11f,  SKIN);
        Circle(x, y, glm::vec2( 0.39f, 1.04f), 0.05f,  SKIN_DARK);

        // --- ARMS ---
        // back arm (right): holds club
        DrawSegment(x, y,
            glm::vec2(-0.52f, 0.52f),
            glm::vec2(-0.82f, 0.28f),
            0.38f, SKIN);
        Circle(x, y, glm::vec2(-0.82f, 0.28f), 0.20f, SKIN_LIGHT);

        // front arm (left): swinging forward
        DrawSegment(x, y,
            glm::vec2( 0.58f, 0.50f),
            glm::vec2( 0.86f, 0.20f),
            0.38f, SKIN);
        Circle(x, y, glm::vec2( 0.86f, 0.20f), 0.20f, SKIN_LIGHT);

        // --- CLUB (thick wooden spiked club, swinging left-upward) ---
        // handle
        DrawSegment(x, y,
            glm::vec2(-0.82f,  0.28f),
            glm::vec2(-1.10f,  0.60f),
            0.10f, WOOD);

        // club head (big block)
        Polygon(x, y, {
            {-1.08f, 0.50f},
            {-1.38f, 0.44f},
            {-1.48f, 0.72f},
            {-1.30f, 0.90f},
            {-0.98f, 0.84f}
        }, WOOD);

        // club highlight
        Polygon(x, y, {
            {-1.10f, 0.58f},
            {-1.30f, 0.54f},
            {-1.26f, 0.68f},
            {-1.08f, 0.70f}
        }, HAIR_LIGHT);

        // metal band around club
        Polygon(x, y, {
            {-1.08f, 0.48f},
            {-1.36f, 0.42f},
            {-1.38f, 0.52f},
            {-1.08f, 0.58f}
        }, METAL);

        // spikes
        Polygon(x, y, {
            {-1.20f, 0.88f},
            {-1.22f, 1.04f},
            {-1.16f, 1.02f},
            {-1.15f, 0.88f}
        }, METAL);
        Polygon(x, y, {
            {-1.36f, 0.82f},
            {-1.44f, 0.97f},
            {-1.38f, 0.96f},
            {-1.30f, 0.82f}
        }, METAL);
    }

    // ===========================================================================
    // MAIN DISPATCH
    // ===========================================================================

    void Draw(float x, float y, EnemyType type)
    {
        switch (type)
        {
        case EnemyType::EasyRanged:
            DrawEasy(x, y);
            break;

        case EnemyType::MediumRanged:
            DrawMedium(x, y);
            break;

        case EnemyType::HardMelee:
            DrawHard(x, y);
            break;
        }
    }
}

