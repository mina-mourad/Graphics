#include "MainMenu.h"
#include "Renderer2D.h"
#include "Input.h"
#include "GameConfig.h"
#include "Environment.h"
#include "CharacterRenderer.h"
#include "Hud.h"
#include "Audio.h"

#include <glm/glm.hpp>
#include <cmath>
#include <string>

namespace
{
    using glm::vec2;
    using glm::vec3;

    const vec2 BUTTON_SIZE(4.4f, 0.95f);

    // A wooden shop-sign button. Returns true when it is clicked.
    // (Hud::InBox splits the rectangle into two triangles, Lab 3 style.)
    bool Button(const std::string& label, vec2 center, vec2 size = BUTTON_SIZE)
    {
        const vec2 mouse = Input::GetMouseWorldPos();
        const bool hover = Hud::InBox(mouse, center, size);
        Hud::DrawPanel(center, size, hover ? 0.35f : 0.0f);
        Renderer2D::DrawTextShadow(label, center, 0.16f, hover ? vec3(1.0f, 1.0f, 0.7f) : vec3(1.0f, 0.95f, 0.85f));

        bool clicked = hover && Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        if (clicked)
            Audio::Play(Audio::Sfx::Click);
        return clicked;
    }

    // The street behind the menus, darkened.
    void Backdrop(float time, float darkness)
    {
        Renderer2D::SetFade(darkness);
        Environment::DrawBackground(time);
        for (int lane = NUM_LANES - 1; lane >= 0; --lane)
            Environment::DrawLane(lane, 0.0f);
        Environment::DrawHomeBase(time);
        Environment::DrawForeground(time);
        Renderer2D::SetFade(0.0f);
    }

    // Volume row: label, [-], ten blocks, [+].
    void VolumeRow(const std::string& label, float y, int value, void (*change)(int))
    {
        Renderer2D::DrawTextShadow(label, vec2(4.4f, y), 0.14f, vec3(1.0f, 0.93f, 0.75f), Renderer2D::Align::Left);
        if (Button("-", vec2(8.3f, y), vec2(0.8f, 0.75f))) { change(-1); Audio::Play(Audio::Sfx::Coin); }
        for (int i = 0; i < 10; ++i)
        {
            vec2 c(9.05f + i * 0.32f, y);
            Renderer2D::DrawQuad(c, vec2(0.26f, 0.5f), vec3(0.12f, 0.07f, 0.04f));
            if (i < value)
                Renderer2D::DrawQuad(c, vec2(0.2f, 0.44f), glm::mix(vec3(0.95f, 0.75f, 0.25f), vec3(0.9f, 0.3f, 0.2f), i / 9.0f));
        }
        if (Button("+", vec2(12.65f, y), vec2(0.8f, 0.75f))) { change(+1); Audio::Play(Audio::Sfx::Coin); }
    }
}

namespace MainMenu
{
    void Init() {}
    void Shutdown() {}

    Action Render(float time)
    {
        Backdrop(time, 0.35f);

        // the defenders in front of the koshk and a thug lurking on the right
        CharacterRenderer::DrawPlayer(2.6f, LANE_Y[0], PlayerType::Ranged, CharacterRenderer::PlayerAnim::Idle, time);
        CharacterRenderer::DrawPlayer(2.3f, LANE_Y[1], PlayerType::Defense, CharacterRenderer::PlayerAnim::Idle, time);
        CharacterRenderer::DrawPlayer(3.3f, LANE_Y[1], PlayerType::Melee, CharacterRenderer::PlayerAnim::Idle, time);
        CharacterRenderer::Draw(14.0f, LANE_Y[0], EnemyType::HardMelee, CharacterRenderer::EnemyAnim::Idle, time);
        CharacterRenderer::Draw(14.9f, LANE_Y[1], EnemyType::EasyRanged, CharacterRenderer::EnemyAnim::Idle, time);

        // title sign
        Hud::DrawPanel(vec2(8.5f, 7.35f), vec2(9.0f, 2.1f));
        float bob = 0.05f * std::sin(time * 2.0f);
        Renderer2D::DrawTextShadow("SHABAB EL HARA", vec2(8.5f, 7.65f + bob), 0.38f, vec3(1.0f, 0.82f, 0.25f));
        Renderer2D::DrawTextShadow("DEFEND THE KOSHK", vec2(8.5f, 6.85f), 0.15f, vec3(1.0f, 0.95f, 0.85f));

        Action a = Action::None;
        if (Button("START GAME", vec2(8.5f, 5.2f))) a = Action::Start;
        if (Button("SOUND", vec2(8.5f, 4.0f)))      a = Action::Sound;
        if (Button("EXIT", vec2(8.5f, 2.8f)))       a = Action::Exit;

        Renderer2D::DrawTextShadow("DMET 502 - COMPUTER GRAPHICS - WINTER 2026", vec2(8.5f, 0.3f), 0.105f, vec3(0.95f, 0.9f, 0.8f));
        return a;
    }

    Action RenderSound(float time)
    {
        Backdrop(time, 0.55f);
        Hud::DrawPanel(vec2(8.5f, 4.6f), vec2(10.4f, 6.6f));
        Renderer2D::DrawTextShadow("SOUND SETTINGS", vec2(8.5f, 7.25f), 0.24f, vec3(1.0f, 0.82f, 0.25f));

        VolumeRow("MASTER", 6.0f, Audio::GetMasterVolume(), Audio::ChangeMasterVolume);
        VolumeRow("MUSIC", 5.0f, Audio::GetMusicVolume(), Audio::ChangeMusicVolume);
        VolumeRow("EFFECTS", 4.0f, Audio::GetEffectsVolume(), Audio::ChangeEffectsVolume);

        Renderer2D::DrawTextShadow("MUTE ALL", vec2(4.4f, 3.0f), 0.14f, vec3(1.0f, 0.93f, 0.75f), Renderer2D::Align::Left);
        if (Button(Audio::IsMuted() ? "ON" : "OFF", vec2(9.6f, 3.0f), vec2(1.6f, 0.75f)))
            Audio::SetMuted(!Audio::IsMuted());

        Action a = Action::None;
        if (Button("BACK", vec2(8.5f, 1.85f), vec2(3.2f, 0.85f)) || Input::IsKeyPressed(GLFW_KEY_ESCAPE))
            a = Action::Back;
        return a;
    }

    Action RenderPause()
    {
        Hud::DrawPanel(vec2(8.5f, 4.4f), vec2(6.2f, 5.6f));
        Renderer2D::DrawTextShadow("PAUSED", vec2(8.5f, 6.4f), 0.32f, vec3(1.0f, 0.82f, 0.25f));
        Action a = Action::None;
        if (Button("CONTINUE", vec2(8.5f, 5.0f)))  a = Action::Continue;
        if (Button("RESTART", vec2(8.5f, 3.85f)))  a = Action::Restart;
        if (Button("MAIN MENU", vec2(8.5f, 2.7f))) a = Action::ToMainMenu;
        return a;
    }

    Action RenderEnd(bool won, float time)
    {
        Hud::DrawPanel(vec2(8.5f, 4.5f), vec2(10.0f, 5.6f));
        float pulse = 1.0f + 0.04f * std::sin(time * 6.0f);
        if (won)
        {
            Renderer2D::DrawTextShadow("THE KOSHK IS SAFE!", vec2(8.5f, 6.5f), 0.3f * pulse, vec3(1.0f, 0.85f, 0.3f));
            Renderer2D::DrawTextShadow("THE SLIPPER GANG RAN AWAY. TEA IS ON AM SAYED!", vec2(8.5f, 5.65f), 0.13f, vec3(1.0f, 0.95f, 0.85f));
        }
        else
        {
            Renderer2D::DrawTextShadow("THE KOSHK IS LOST!", vec2(8.5f, 6.5f), 0.3f * pulse, vec3(1.0f, 0.4f, 0.3f));
            Renderer2D::DrawTextShadow("A THUG REACHED AM SAYED'S KOSHK...", vec2(8.5f, 5.65f), 0.13f, vec3(1.0f, 0.95f, 0.85f));
        }
        Action a = Action::None;
        if (Button("RESTART", vec2(8.5f, 4.3f)))   a = Action::Restart;
        if (Button("MAIN MENU", vec2(8.5f, 3.1f))) a = Action::ToMainMenu;
        return a;
    }
}
