#include "HealthBar.h"
#include "Renderer2D.h"

namespace HealthBar
{
    void Draw(glm::vec2 center, int health, int maxHealth, bool isPlayer)
    {
        const float block = 0.11f;
        const float gap = 0.025f;
        const float width = maxHealth * block + (maxHealth - 1) * gap;
        const glm::vec3 full = isPlayer ? glm::vec3(0.30f, 0.85f, 0.30f) : glm::vec3(0.95f, 0.30f, 0.20f);
        const glm::vec3 empty(0.22f, 0.18f, 0.16f);

        // the effects (damage tint) must not change the bar's colors
        Renderer2D::SetDamage(0.0f);

        Renderer2D::DrawQuad(center, glm::vec2(width + 0.08f, block + 0.08f), glm::vec3(0.08f, 0.06f, 0.05f));
        float x = center.x - width * 0.5f + block * 0.5f;
        for (int i = 0; i < maxHealth; ++i)
        {
            bool on = i < health;
            Renderer2D::DrawQuad(glm::vec2(x, center.y), glm::vec2(block, block), on ? full : empty);
            if (on)   // a lighter top edge so the blocks look like little gems
                Renderer2D::DrawQuad(glm::vec2(x, center.y + block * 0.3f), glm::vec2(block, block * 0.3f), full * 1.25f);
            x += block + gap;
        }
    }
}
