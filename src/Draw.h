#pragma once
#include <glm/glm.hpp>
#include <string>
#include "GameConfig.h"

// Visual library for the game. Everything here is built out of the shape
// primitives in Renderer2D (quads, triangles, ellipses) so no single object is
// ever one bare primitive.
//
// THEME: "Guardians of the Sun Disc" - ancient Egypt. Medjay guardians defend
// the Sun Temple (home base) from the cursed army of Set.
namespace Draw
{
    // Per-object colour effects applied to every part of a model.
    struct Fx
    {
        float gray  = 0.0f;   // 1 = fully grayscale (locked cards)
        float red   = 0.0f;   // 0..1 damage tint: more red as health drops
        float flash = 0.0f;   // brief yellow-white flash when hit (u_Highlight)
        float alpha = 1.0f;
    };

    glm::vec3 Apply(glm::vec3 c, const Fx& fx);

    // Pose handed to the character models. The model's origin is the centre of
    // the feet; the model faces +X when facing > 0 and -X when facing < 0.
    struct Pose
    {
        glm::vec2 foot{0.0f};
        float facing = 1.0f;
        float scale  = 1.0f;
        float time   = 0.0f;     // animation clock, seconds
        float walk   = 0.0f;     // walk-cycle phase, radians
        float moving = 0.0f;     // 0 = standing, 1 = full walk/run cycle
        float attack = -1.0f;    // -1 = idle, 0..1 = progress of the attack animation
        float air    = 0.0f;     // 0..1 how high above the ground (legs tuck)
        float duck   = 0.0f;     // 0..1 how crouched
        Fx    fx;
    };

    // ---- Player characters (face right) ----
    void Melee(const Pose& p);       // Medjay swordsman with khopesh + shield
    void Archer(const Pose& p);      // Nubian archer with bow and quiver
    void Guardian(const Pose& p);    // stone guardian statue with tower shield

    // ---- Enemy characters (face left) ----
    void Vulture(const Pose& p);     // EASY: Vulture spirit, attacks from above
    void Scorpion(const Pose& p);    // MEDIUM: Giant scorpion, attacks from below
    void Mummy(const Pose& p);       // HARD: Mummy brute, charges and smashes

    // Contact shadow under a character.
    void Shadow(glm::vec2 foot, float radius, float alpha);

    // Height of the model above the lane centre for a hovering enemy.
    constexpr float VULTURE_HOVER = 0.38f;

    // ---- Projectiles, items, effects ----
    void Arrow(glm::vec2 pos, float time);                 // player arrow (flies right)
    void SpiritBolt(glm::vec2 pos, float time);            // vulture projectile (flies left, HIGH)
    void StingerBolt(glm::vec2 pos, float time);           // scorpion projectile (flies left, LOW)
    void Coin(glm::vec2 pos, float time, float scale = 1.0f, const Fx& fx = Fx());
    void HitSpark(glm::vec2 pos, float progress, glm::vec3 color);   // progress 0..1
    void HealthBar(glm::vec2 centre, float width, int hp, int maxHp, bool friendly);

    // ---- Environment ----
    void Sky(float time);                                  // sky, sun, far pyramids, dunes, palms, obelisks
    void LaneFloor(int lane, float highlight, bool selected);
    void HomeBase(float time);                             // the Sun Temple on the left
    void Foreground(float time);                           // torches, rocks, cacti along the borders
    void Plaque(glm::vec2 centre, glm::vec2 size, float highlight = 0.0f, const Fx& fx = Fx());

    // Small portrait used on the player cards.
    void CardPortrait(PlayerType type, glm::vec2 centre, float time, const Fx& fx);

    // ---- Low level helper shared by Draw*.cpp ----
    void DrawLine(glm::vec2 a, glm::vec2 b, float width, glm::vec3 color, float alpha = 1.0f);
}
