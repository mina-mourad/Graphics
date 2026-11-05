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

The files are split so that everyone gets the **same amount of difficulty**, not the same topic.
Each file got difficulty points (how much logic it has, how many tricky ideas, and how much the
TA is likely to ask about it). Code that is almost all lab code counts less, because everyone
studied it in the labs and the lab lines are written right beside it.

| File | Points | Why |
|---|---|---|
| `src/Sodas.cpp` | 5.5 | the 3 waves (a small state machine), enemy attacks, every shot: hits + dodging, drawing with sin / cos |
| `src/Candies.cpp` | 5 | most player rules (timers, cooldowns, walking, attacks, picking up sugar, removing from a vector), drawing with sin / cos |
| `src/Shapes.cpp` | 3.5 | shaders, VAO / VBO / EBO, uniforms, translate / rotate / scale - all lab code, with the lab lines beside it |
| `src/Shelves.cpp` | 3 | `pointInTriangle` math, lane hover / select, the store and its 8 environment elements |
| `src/Game.h` + `src/Game.cpp` | 3 | the shared structs and numbers, the game states (switch), the update order |
| `src/Hud.cpp` | 2.5 | the unlock rule, gray / hover cards, health bars, sugar cubes |
| `src/Text.cpp` | 2.5 | letters stored as bits, drawn as small squares (not lab code) |
| `main.cpp` | 2 | the Lab 2 template: window, input callbacks, deltaTime; plus mouse pixels -> our coordinates |
| `src/Cutscene.cpp` | 2 | a timeline: `Progress()` moves things between two times |
| `src/Menus.cpp` | 1.5 | a button = box hover + highlight + click |
| `src/Sound.cpp` | 1.5 | a few calls to the miniaudio library |

Each person **owns** some files (with their `.h`): they explain them and are the one who changes them.
**Read only** = files or single functions you open to understand your own part; don't change them,
ask their owner. Everyone also reads `src/Game.h` (the shared numbers, types and lists).

| Person | Owns (explain + change) | Points | Read only (to understand your part) | PDF sections |
|---|---|---|---|---|
| **1 – Window, drawing and text** | `main.cpp`, `src/Shapes.cpp`, `src/Text.cpp` | 8 | `src/Game.h`; `GameUpdate()` / `GameDraw()` in `src/Game.cpp` (what `main.cpp` calls every frame); one drawing function as an example of `Rect` / `Circle` use, e.g. `DrawChips()` in `src/Shelves.cpp`; labs: Lab 1 Notes, Lab 2 Template, Lab 3 `main.cpp` + `Lab3ExtraNotes.cpp` | OpenGL 3.3+, drawing every shape (nothing is a single primitive), mouse + keyboard input, text on screen |
| **2 – Candies, menus and sound** | `src/Candies.cpp`, `src/Menus.cpp`, `src/Sound.cpp`, `sounds/` | 8 | `src/Game.h`, `src/Shapes.h`, `src/Text.h`; `UpdateShelves()` in `src/Shelves.cpp` (shelf select, calls `PlaceChocolate`); `UpdateShots()` + `DrawSoda()` in `src/Sodas.cpp` (dodging; the soda on the main menu); `CardUnlocked()`, `DrawHealth()` + `InBox()` in `src/Hud.cpp`; `GameDraw()` in `src/Game.cpp` (what the menu buttons do); labs: Lab 2 Template, Lab 3 `main.cpp`, Lab 4 | 3.2 players (buy, place, walk, attack, jump / duck, red when hurt), picking up collectibles, 4.3 menus, 5 sound (bonus) |
| **3 – Sodas, waves and the HUD** | `src/Sodas.cpp`, `src/Hud.cpp` | 8 | `src/Game.h`, `src/Shapes.h`, `src/Text.h`; `UpdateCandies()`, `PlaceCandies()` + `DrawCandy()` in `src/Candies.cpp` (lollipops hit sodas, the cards place and draw candies); `pointInTriangle()` in `src/Shelves.cpp` (used by `InBox`); labs: Lab 2 Template, Lab 3 `main.cpp`, Lab 4 | 3.3 enemies + the 3 waves, shots, sugar drops, 4.1 health bars, 4.2 cards (locked / unlocked) + sugar counter |
| **4 – Store, game flow and intro** | `src/Shelves.cpp`, `src/Game.h`, `src/Game.cpp`, `src/Cutscene.cpp`, `CMakeLists.txt`, `README.md` | 8 | `main.cpp` (the loop calls `GameUpdate` / `GameDraw`, the callbacks fill the mouse and keys); `PlaceChocolate()` + `DrawCandy()` in `src/Candies.cpp`; `DrawSoda()` in `src/Sodas.cpp` (both drawn in the intro); the `.h` files of persons 1–3 (the functions `Game.cpp` calls); labs: Lab 2 Template, Lab 3 `main.cpp` | 3.1 playfield (lanes, home base, fridge), lane hover / select, 4.2 environment, 2 cutscene, game states (playing, paused, won, lost), building the game |

## What comes from the labs

**In the code:** below every part that uses lab code there is a comment `// LAB CODE (lab file):` with the
lab lines copied exactly as they are written in the lab files (`Lab 1 Notes.pdf`, `Lab 2 Template/main.cpp`, `Lab 3 Notes/main.cpp`,
`Lab 3 Notes/Lab3ExtraNotes.cpp`, `Lab 4 Code/Lab4Camera.cpp`). Every function and file without lab code says
`// no lab code`. Short notes in `// (...)` explain where our line differs from the lab line.

**From the labs:**
- red damage: `u_Red` in the fragment shader, a copy of the Lab 3 `u_Highlight` mix (toward red instead of yellow)
- gray locked cards: `u_Gray` in the fragment shader, the same mix toward the color's own gray (only `gray = (r + g + b) / 3` is not lab code)
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
