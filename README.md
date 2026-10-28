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

## Branch `reference/full-game` — complete reference implementation

This branch contains a finished, playable version of the game ("Guardians of the Sun Disc", ancient-Egypt theme) kept as a **working backup / reference**. It was generated with an AI assistant (Claude); the team lead said the course doctor allowed this verbally. **Get that permission in writing before submitting anything from this branch** — the spec treats unauthorised AI use as a zero.

**Controls:** click a lane to select/deselect it (multi-select works), click a card or press `1` / `2` / `3` to place Melee / Ranged / Defense on every selected lane, `W`/`Up` jump (dodge low attacks), `S`/`Down` duck (dodge high attacks), click or walk over coins to collect them, `Esc` pause, `Space` skips the cutscene.

**Structure:** `Game.cpp` (state machine, waves, combat, HUD, menus), `Characters.cpp` (all character models), `Environment.cpp` (sky, lanes, scenery, temple), `Cutscene.cpp` (16 s intro), `Draw.cpp` (effects, health bars, props), `Text.cpp` (bitmap font), `Renderer2D.cpp` (quads, rotated quads, triangles, ellipses, alpha).

**Not implemented:** sound/audio bonus.
