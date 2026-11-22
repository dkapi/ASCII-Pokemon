# ASCII-Pokémon

A pokémon-flavoured ASCII roguelike in C. Explore a huge procedurally-generated
world, catch wild pokémon in the tall grass, battle the trainers in your way,
and build a team of up to six.

Rendered with **ncurses** — colored map, message log, party panel, and a
title screen.

## Build & run

Requires **CMake**, a C11 compiler, and **ncurses** (`libncursesw`).

```sh
cmake -S . -B build
cmake --build build
./build/worldgen
```

Run the binary from the repository root (world assets are generated at
runtime, but the working directory should be the repo root).

`-d` / `--debug` prints the hiker/rival Dijkstra cost maps before the title
screen. Terrain is domain-warped value-noise by default (flowing ridges and
rivers); pass `--no-warp` for plain noise, or `--voronoi` for the old
Voronoi-region fill.

## Controls

| Key | Action |
|---|---|
| `↑` `↓` `←` `→` or `n` / `s` / `e` / `w` | move one tile |
| `f` | fly to a map coordinate (`-200..200`, `0 0` = world center) |
| `h` or `?` | help / controls |
| `m` | message log (scrollable history) |
| `p` | party — view and switch your active pokémon |
| `r` | use a Repel (no wild encounters for 100 steps) |
| `q` | quit |

**In battle:** `1` / `2` use a move · `p` potion · `b` poké ball (wild only) ·
`r` run.

**In the mart:** `1` potion · `2` poké ball · `3` repel · `q` leave.

**Starter:** `1`–`6`.

## The game

- **World** — 401 × 401 maps, each 80 × 21, centered on `(0,0)`. Terrain comes
  from domain-warped value-noise (elevation + moisture; `--no-warp` for plain
  noise, `--voronoi` for the old Voronoi fill): mountain `%`,
  tree `^`, tall grass `:`, water `~`, clearing `.`, sand `=`, snow `*`, paths
  `#`, plus Pokémon Centers `C`, Marts `M`, and cave entrances `O`. A path (the
  "route") connects the map's four border gates so you can walk between maps.

- **Caves** — step on a cave entrance `O` to descend into a parallel underground
  layer. Cave maps are organically dug with cellular automata and are dark: you
  see only a round torch-lit radius around you. They spawn a **cave-only**
  encounter pool (Zubat, Geodude, Onix, Clefairy, …) several levels hotter than
  the overworld, and hold floor loot — `$` money and `!` potions/balls. Cave
  maps link sideways through border tunnels (the same gate system as the
  overworld), and you find `>` to climb back out at the entrance you came in
  through.

- **Buildings** — step on a Center `C` or Mart `M` to go inside. The Center has
  Nurse Joy `N` (heal) and a PC `P` (box); the Mart has a clerk `S` (shop). Step
  on the door `#` to leave.

- **Turns** — every actor is scheduled on a min-heap keyed by accumulated move
  cost (path 10, grass 20, water 7), so time passes fairly and in cost order.

- **NPCs** — seven trainer types: hiker, rival, pacer, sentry, wanderer,
  explorer, swimmer. Hikers and rivals pathfind toward you; wild and trainer
  levels rise with distance from the world center.

- **Pokémon** — the full Gen 1 + 2 Pokédex (251 species). Pick from six
  starters: Bulbasaur, Charmander, Squirtle, Chikorita, Cyndaquil, Totodile.
  Wild encounters are **biome-specific** — each terrain spawns its own pool
  (grass → bugs/grass, desert → ground/fire, snow → ice, clearings → normal) —
  and walking into a trainer starts a battle.

- **Battles** — damage = attack vs. defense from level, move power, and stats
  with ±15% variance, scaled by **type effectiveness** (17 Gen 1–2 types, 2×/½×/0×
  with "super effective!" / "not very effective…" feedback). Each pokémon knows
  `Tackle` plus one signature move. Weaken wilds and throw Poké Balls; potions
  heal mid-fight. Trainers field **2–3 pokémon** (scaling with distance) and
  send out the next when one faints; you can **switch** your active mid-battle
  (`s`). A beaten trainer stands down and no longer blocks the route.

- **Growth** — winning or catching awards **EXP**; pokémon level up (stats
  recomputed) and **evolve** at the right level (starters, Magikarp→Gyarados,
  …). The battle screen shows an ASCII sprite per type, type badges, and HP/EXP
  bars.

- **Party & money** — caught pokémon join your party (up to six); overflow goes
  to the **PC box** (100 slots), managed at a Center's PC. You start with $3000,
  three potions, and five balls; win trainer battles for cash, buy potions
  ($200), balls ($100), and Repels ($350) at the Mart, and heal the whole party
  at a Center. Use a Repel (`r`) to keep wild pokémon away for 100 steps.

## Not yet

- No saving/loading.
- Trade/item evolutions (only level-up + happiness evolutions are wired).
- No special/physical split (all moves use the physical formula).

## Credits

Built on a Dijkstra-map + Fibonacci-heap turn engine. The Gen 1–2 Pokédex is
generated from the veekun `pokedex` CSVs.