# Ghana Road Transport Network — 2026

**Ashesi University, Department of Computer Science**
Authors: Edward Ofosu Mensah, Shadrach Nti Agyei
Language: C++17

---

## Overview

This program models the Ghana road transport network as an undirected weighted graph. Each town is a node and each road is an edge carrying two weights: distance (km) and travel time (minutes). The program supports shortest-path queries, K-shortest paths, and cost analysis through an interactive terminal menu.

---

## Files

| File | Description |
|---|---|
| `transport.cpp` | Full source code (single-file) |
| `ghana_cities_graph_2026.txt` | Edge list: 183 towns, 551 roads |

---

## Build

Requires a C++17-compliant compiler (GCC 7+, Clang 5+, MSVC 2017+).

```bash
# GCC
g++ -std=c++17 -O2 -o transport transport.cpp

# Clang
clang++ -std=c++17 -O2 -o transport transport.cpp
```

---

## Run

```bash
./transport                          # uses ghana_cities_graph_2026.txt by default
./transport path/to/other_graph.txt  # optional custom data file
```

---

## Data File Format

Plain-text CSV, one edge per line, no header:

```
Source, Destination, Distance(km), Time(min)
```

Example:
```
Accra, Kumasi, 358, 255
Kumasi, Obuasi, 53, 57
```

- Whitespace around field values is ignored.
- Duplicate edges are silently skipped.
- The graph is treated as **undirected**: each line adds edges in both directions.

---

## Constants (spec-defined)

| Constant | Value |
|---|---|
| Fuel consumption | 8 km / litre |
| Fuel price | GHS 11.95 / litre |
| Time cost | GHS 0.50 / minute |

---

## Interactive Menu

```
┌─── MENU ──────────────────────────────────┐
│  1. Print adjacency list (first 10 towns) │
│  2. Show neighbours of a town             │
│  3. Shortest path (distance)              │
│  4. Shortest path (time)                  │
│  5. K-shortest paths                      │
│  6. Cost analysis (fuel + time cost)      │
│  0. Quit                                  │
└───────────────────────────────────────────┘
```

### Option 1 — Adjacency list
Prints the first 10 towns in load order with all their direct neighbours, distances, and travel times.

### Option 2 — Neighbours of a town
Prompts for a town name and lists every directly connected town with distance and travel time.

### Option 3 — Shortest path by distance
Runs Dijkstra's algorithm minimising total kilometres. Reports the optimal distance, total travel time, and the full route.

### Option 4 — Shortest path by time
Runs Dijkstra's algorithm minimising total travel time. Reports the optimal time, total distance, and the full route.

### Option 5 — K-shortest paths (Yen's algorithm)
Prompts for source, destination, K (number of paths), and weight (0 = distance, 1 = time). Returns the K least-cost simple paths ranked by the chosen weight, each showing distance, time, and the full route.

### Option 6 — Cost analysis
Runs both shortest-distance and shortest-time Dijkstra queries and displays a side-by-side breakdown:
- Fuel cost (GHS) = `distance / 8 × 11.95`
- Time cost (GHS) = `time × 0.50`
- Total cost (GHS) = fuel cost + time cost

---

## Algorithms

| Question | Algorithm | Complexity |
|---|---|---|
| Q1 | File parsing into adjacency list | O(E) |
| Q2 | Adjacency list traversal | O(deg(v)) |
| Q3 | Neighbour lookup | O(deg(v)) |
| Q4 | Dijkstra's algorithm (binary heap) | O((V + E) log V) |
| Q5 | Yen's K-Shortest Paths | O(KV(V + E) log V) |
| Q7 | Fuel and time cost formulas | O(1) |
| Q8 | Cost analysis (two Dijkstra runs) | O((V + E) log V) |

---

## Example Session

```
Loaded graph: 183 towns, 551 edges.

Choice: 3
Source town: Accra
Destination town: Kumasi

  Distance     : 510 km
  Total dist   : 510 km
  Total time   : 584 min
  Route: Accra → Berekum → Wenchi → Juaso → Nalerigu → Gambaga → Kumasi

Choice: 6
Source town: Accra
Destination town: Kumasi

━━━━  COST ANALYSIS: Accra → Kumasi  ━━━━
  Shortest-distance route    dist= 510 km  time= 584 min
    Fuel cost   : GHS   761.81
    Time cost   : GHS   292.00
    TOTAL cost  : GHS  1053.81
    Route: Accra → Berekum → Wenchi → Juaso → Nalerigu → Gambaga → Kumasi
  Shortest-time route        dist= 607 km  time= 499 min
    Fuel cost   : GHS   906.71
    Time cost   : GHS   249.50
    TOTAL cost  : GHS  1156.21
    Route: Accra → Konongo → Obuasi → Kumasi
```

---

## Known Limitations / Pending Work

- Questions 6, 9, and 10 are not yet implemented.
- The adjacency list display is capped at 10 towns; a full-print option is not yet exposed in the menu.
- Town name matching is case-sensitive (`Accra` ≠ `accra`).
