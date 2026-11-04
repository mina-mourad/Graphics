# Soda vs Candy — DMET 502 Project 1 (simple, lab-level version)

Branch `simple/candy-vs-soda`, written from scratch on top of the course template
(CMake project + `external/` GLFW, GLM, GLAD, with `main.cpp` following the Lab 2 template).
Made with an AI assistant (Claude): get the doctor's permission in writing before submitting.

**Story:** the candy jar is full of sugar. The sodas in the fridge want it to stay sweet.
The candies defend the jar, shelf by shelf.

| | Players (candies) | Enemies (sodas) |
|---|---|---|
| Melee / Hard | Lollipop — swings a candy cane (5 lives) | Big cup — runs, whacks with its straw, 2 damage (7 lives) |
| Ranged / Easy | Bonbon — shoots candy pellets that pop on hit (5 lives) | Bottle — fizz up high, **duck** (5 lives) |
| Defense / Medium | Chocolate bar — stands still and blocks (7 hits) | Can — syrup down low, **jump** (5 lives) |

**Controls:** click shelves to select / unselect (several at once) · click a card or press `1` `2` to place on the selected shelves · chocolate bar: card or `3`, then click **anywhere on a shelf** (gray preview; click the card again to cancel) ·
`W`/`Up` jump · `S`/`Down` duck · `Esc` pause · `Space` skips the cutscene.
Start with 30 sugar (Lollipop 10, Bonbon 20, Chocolate 30; the bonbon and chocolate unlock after you collect that much sugar). Defeated sodas drop sugar cubes; walk a candy into them to collect.

## Checked against the project PDF
Everything in sections 2–5 of the description is implemented.
- **Environment (spec 4.2): 8 supermarket elements**, each only 2–4 simple shapes, all in `src/Shelves.cpp`:
  shelves with price tags, the soda fridge, SALE signs hanging from the shelves, bags of chips,
  a crate of oranges, cardboard boxes, a mop in a bucket and a trash bin. (The candy jar is the home base, so it is not counted.)

Two choices to know about:
- **Chocolate bar anywhere on a shelf** (team's choice). The PDF says a placed player appears at the
  left end of the selected lane; to follow it exactly, make the chocolate card use `PlaceCandies` like the other two.
- **Costs and unlocking (spec 3.2 + table on page 9):** Lollipop (Melee) 10, Bonbon (Long Range) 2 x 10 = 20,
  Chocolate (Defense) 3 x 10 = 30. You start with 3 x 10 = 30 sugar (three lollipops). Only the lollipop is unlocked
  at the start: the other cards unlock once you have *collected* that much sugar from defeated sodas ("as the player
  collects enough coins, the other player types are progressively unlocked"), and they lock again whenever your sugar
  is below their cost (spec 4.2). See `CardUnlocked()` in `src/Hud.cpp`.

## Files (one part each)

| File | What it does | Person |
|---|---|---|
| `main.cpp` | window, input callbacks, main loop | all |
| `src/Shapes.cpp` | the lab shader + square / triangle / circle | all |
| `src/Shelves.cpp` | store background and its 8 environment elements, shelves (lanes), candy jar, shelf selection | 1 |
| `src/Candies.cpp` | placing candies, what they do, drawing them | 2 |
| `src/Sodas.cpp` | sodas, the 3 waves, all flying shots | 3 |
| `src/Hud.cpp` | cards, sugar counter, health bars, sugar cubes | 4 |
| `src/Menus.cpp`, `src/Cutscene.cpp`, `src/Game.cpp` | menus, intro, game states | 4 / all |
| `src/Text.cpp`, `src/Sound.cpp` | block letters, sound | extra |

## What comes from the labs

**From the labs:**
- window / GLAD setup, shader compile + link: Lab 2 template
- VAO + VBO with position and color, `u_Transform`: Lab 3
- `u_Tint` (color) — Lab 2 template; `u_Highlight` mix to yellow (hover, selected shelves) — Lab 3
- several shapes in one VBO + an EBO for the square — Lab 3; circle as a `GL_TRIANGLE_FAN` — Lab 3
- `glm::translate / rotate / scale` — Lab 2 template; `scale(1/aspect)` so circles stay round — Lab 2 template
- key / mouse button / cursor position callbacks, pixels → -1..1 — Lab 3
- `triSign` / `pointInTriangle` for shelves, cards and buttons — Lab 3
- `deltaTime` from `glfwGetTime()`, `sin()` animation — Lab 2 template

**Not from the labs (kept as small as possible):**
- game rules: `struct`, `std::vector`, `rand()`, `enum` + `switch` for the states (plain C++)
- `Text.cpp`: letters drawn as small squares (the spec needs a coin counter and menu labels)
- red damage / gray locked cards: `glm::mix` on the color before drawing
- `Sound.cpp`: the labs have no sound, so it uses the one-header library **miniaudio** (`external/miniaudio/miniaudio.h`)

Not used at all: textures, images, transparency, 3D, any other library.

## Sounds (bonus)
Background music + 15 effects (click, shelf select, place, locked, whack, pellet, fizz, hurt, defeated,
sugar, jump, duck, wave, win, lose). Main menu → SOUND: volume `-` / `+` and mute.
Files are in `sounds/` (credits in `sounds/CREDITS.txt`):
- music "House In a Forest Loop" by HorrorPen, CC-BY 3.0 (credit required)
- effects by Kenney (kenney.nl), CC0

## Build
Same as the template: open the folder in VS Code, CMake, build and run `OpenGLStarter`.
The game finds the sound files through the `SOUND_DIR` path set in `CMakeLists.txt`.
