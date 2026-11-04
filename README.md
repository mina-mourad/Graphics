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

## Who does what (4 people)

Each person **owns** some files: they explain them and are the one who changes them.
**Read only** = files (or single functions) you need to open to understand your own part; don't change
them, ask their owner. Everyone also reads `src/Game.h` (the shared numbers, types and lists).

| Person | Owns (explain + change) | Read only (to understand your part) | PDF sections |
|---|---|---|---|
| **1 – Window, drawing and the store** | `main.cpp`, `src/Shapes.h/.cpp`, `src/Text.h/.cpp`, `src/Shelves.h/.cpp` | `src/Game.h`; `src/Game.cpp` (where `UpdateShelves` / `DrawStore` are called); `PlaceChocolate()` in `src/Candies.cpp`; labs: Lab 1 Notes, Lab 2 Template, Lab 3 `main.cpp` + `Lab3ExtraNotes.cpp` | 3.1 playfield (lanes, home base, fridge), lane hover / select, 4.2 environment (8 elements) |
| **2 – Candies (players), cards and sugar** | `src/Candies.h/.cpp`, `src/Hud.h/.cpp` | `src/Game.h`, `src/Shapes.h`, `src/Text.h`; `pointInTriangle()` + `UpdateShelves()` in `src/Shelves.cpp`; `UpdateShots()` in `src/Sodas.cpp` (soda shots hitting candies, dodging); labs: Lab 2 Template, Lab 3 `main.cpp`, Lab 4 | 3.2 players (buy, place, walk, attack, jump / duck, red when hurt), collectibles, 4.1 health bars, 4.2 cards + sugar counter |
| **3 – Sodas (enemies), waves and the intro** | `src/Sodas.h/.cpp`, `src/Cutscene.h/.cpp` | `src/Game.h`, `src/Shapes.h`, `src/Text.h`; `DrawCandy()` and `UpdateCandies()` in `src/Candies.cpp` (the intro draws candies; lollipops hit sodas); `DrawHealth()` in `src/Hud.cpp`; `DrawStore()` in `src/Shelves.cpp` (the fridge door opens in the intro); labs: Lab 2 Template, Lab 3 `main.cpp`, Lab 4 | 2 cutscene, 3.3 enemies + the 3 waves, shots, sugar drops |
| **4 – Game flow, menus and sound** | `src/Game.h/.cpp`, `src/Menus.h/.cpp`, `src/Sound.h/.cpp`, `sounds/`, `CMakeLists.txt`, `README.md` | `main.cpp` (the loop calls `GameUpdate` / `GameDraw`, the callbacks fill the mouse and keys); `InBox()` in `src/Hud.cpp` (buttons); the `.h` files of persons 1–3 (the functions `Game.cpp` calls); labs: Lab 2 Template, Lab 3 `main.cpp` + `Lab3ExtraNotes.cpp` | 4.3 menus (main, pause, win / lose), game states, 5 sound (bonus), building the game |

## What comes from the labs

**In the code:** below every part that uses lab code there is a comment `// LAB CODE (lab file):` with the
lab lines copied exactly as they are written in the lab files (`Lab 1 Notes.pdf`, `Lab 2 Template/main.cpp`, `Lab 3 Notes/main.cpp`,
`Lab 3 Notes/Lab3ExtraNotes.cpp`, `Lab 4 Code/Lab4Camera.cpp`). Every function and file without lab code says
`// no lab code`. Short notes in `// (...)` explain where our line differs from the lab line.

**From the labs:**
- VBO + VAO, `glVertexAttribPointer`, `glDrawArrays`, the render loop (swap buffers, poll events): Lab 1
- window / GLAD setup, shader compile + link: Lab 2 template
- VAO + VBO with position and color, `u_Transform`: Lab 3
- `u_Tint` (color) — Lab 2 template; `u_Highlight` mix to yellow (hover, selected shelves) — Lab 3
- several shapes in one VBO + an EBO for the square — Lab 3; circle as a `GL_TRIANGLE_FAN` — Lab 3
- `glm::translate / rotate / scale` — Lab 2 template; `scale(1/aspect)` so circles stay round — Lab 2 template
- key / mouse button / cursor position callbacks, pixels → -1..1 — Lab 3
- `triSign` / `pointInTriangle` for shelves, cards and buttons — Lab 3
- `deltaTime` from `glfwGetTime()`, `sin()` animation — Lab 2 template
- moving with speed × `deltaTime` — Lab 4 (`float speed = MOVE_SPEED * deltaTime;`)

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
