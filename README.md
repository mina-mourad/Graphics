# Graphics — 2D Tower Defense (DMET 502)

## Getting started

1. Follow `Installation Guidelines - VS Code.pdf` / run `install_tools.py` once per machine (compiler, CMake, Ninja — not covered by this repo, see the template you downloaded).
2. **This repo is also missing the vendored library folder, `external/` (GLFW, GLM, GLAD).** Copy it in from the "OpenGL Template for VS Code" folder before building — `CMakeLists.txt` expects `external/glfw-3.5.1`, `external/glm`, and `external/glad/include` to exist.
3. Open the folder in VS Code, use the CMake panel, build and run the `OpenGLStarter` target.
4. **You should see one gray square on a dark background.** That's the whole engine smoke test — if you see it, the build system, shader, renderer, and window are all working.

## Where your code goes

- Add new `.cpp` files under `src/`.
- **Add every new file to `CMakeLists.txt`'s `add_executable(...)` list** — a file not listed there silently does not compile in. This is the most common "it works for me" bug on this kind of project.
- Draw your sprites (placeholder colored rectangles for now, real art later) with `Renderer2D::DrawQuad(pos, size, color, highlight)` — see `src/Renderer2D.h`.
- Read clicks/keys with the `Input::` functions — see `src/Input.h`.
- Use the constants in `src/GameConfig.h` (world size, lane Y positions, tile size, `PlayerType`/`EnemyType`) instead of hardcoding your own numbers, so everyone's coordinates agree.

## What this is / isn't

This is the minimum engine scaffold needed for Player Systems (Person 2), Enemy/Wave/Economy (Person 3), and UI/Art (Person 4) to start writing real game code in parallel. It deliberately does **not** include texture/image loading, text rendering, or audio — those come later and don't block anyone from starting.

---

## Branch `feature/cairo-street` — Shabab El Hara: Defend the Koshk

Continues `feature/continue-from-main` (the team's animated enemies) into a complete game. **Not merged into `main`.** It was built with an AI assistant (Claude) as a working reference/backup — get the doctor's permission in writing before submitting anything from it, because the spec treats AI use as a zero.

**Story:** in an old Cairo *hara* (alley), Am Sayed's *koshk* (kiosk) keeps the whole street going. The slipper gang wants to take it. The *shabab* of the street defend it.

**Players** (same drawing / pose system as the enemies): **El Bawab** (melee, galabeya + nabboot stick), **the tomato seller** (ranged, throws tomatoes that splat), **El Me'allem** (defense, mechanic behind a garage-shutter shield).
**Enemies** (team's): slipper thrower (high attack → duck), slingshot kid (low attack → jump), knife thug (runs, 2 damage).

**Controls:** click lanes to select/deselect (several at once) · click a card or press `1`/`2`/`3` to place on every selected lane · `W`/`Up` jump · `S`/`Down` duck · `Esc` or the pause button to pause · `Space` skips the cutscene.

**Rules implemented:** 3 lanes, start with 30 coins (Melee 10, Ranged 40, Defense 60; cards grayscale + unclickable when unaffordable), waves of 3/5/10 enemies in random lanes, coins dropped by enemies and collected when a player walks into them, red tint growing as health drops, health blocks under everyone, Defense shields players behind it, lose when a thug reaches the koshk (everything freezes), win after wave 3. Menus: main (start / sound / exit), sound settings, pause, win/lose. 16 s animated cutscene with dialogue.

**Sound bonus:** `src/Audio.cpp` synthesises everything in code — darbuka/oud/nay menu music, a mizmar + darbuka battle groove, tuk-tuk horn for each wave, slipper smacks, tomato splats, coin clinks, a zaghrouta when you win. Sound settings menu: master / music / effects volume and mute. Uses the single-header `external/miniaudio/miniaudio.h`.

**Close to the labs:** one shader with `u_Highlight`-style `mix()` (yellow hover, red damage, gray locked cards, black fades), unit quad + unit circle VBOs drawn with `glm::translate/rotate/scale` model matrices, `glm::ortho` projection, `GL_TRIANGLE_FAN` circles/polygons, the Lab 3 `triSign`/`pointInTriangle` hit test for lanes, cards and buttons, Lab 3 key callback for one-shot keys, `glfwGetTime()` delta time.

**Files:** `Game.cpp` (states), `PlayerSystem.cpp` (Person 2: lanes, placement, players), `EnemySystem.cpp` + `WaveManager.cpp` (Person 3), `CharacterRenderer.cpp` (all character models), `Environment.cpp` (street + koshk), `Hud.cpp` / `Economy.cpp` / `HealthBar.cpp` (UI, coins), `Cutscene.cpp`, `MainMenu.cpp` (all menus), `Audio.cpp`, `Renderer2D.cpp`, `Input.cpp`.
