# Ghana Road Transport Network — 2026

**Ashesi University, Department of Computer Science**
Authors: Edward Ofosu Mensah, Shadrach Nti Agyei
Language: C++17

---

## Overview

This program models the Ghana road transport network as a weighted graph. Each town is a node and each road is an edge carrying two weights: distance (km) and travel time (minutes). The program supports shortest-path queries, top-3 shortest paths, cost analysis with recommendations, and live graph editing through an interactive terminal menu.

---

## Files

| File | Description |
|---|---|
| `transport.h` | TransportGraph class — all data structures, algorithms, and menu |
| `transport.cpp` | Entry point (`main`) |
| `test_transport.cpp` | 23 modular tests |
| `ghana_cities_graph_2026.txt` | Edge list: 183 towns, 551 roads |
| `complexity_analysis.md` | Q10 — time complexity analysis and scalability estimates |
| `Makefile` | Build targets |

---

## Build

Requires a C++17-compliant compiler (GCC 7+, Clang 5+, MSVC 2017+).

```bash
make transport    # build main program
make test         # build and run all tests
make clean        # remove binaries
```

Or manually:
```bash
g++ -std=c++17 -O2 -o transport transport.cpp
```

---

## Run

```bash
./transport                          # uses ghana_cities_graph_2026.txt by default
./transport path/to/other_graph.txt  # optional custom data file
```

---

## Data File Format

Plain-text CSV, one edge per line. Header rows (containing "source" and "destination") are automatically skipped.

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
- Each line adds edges in both directions (undirected).

---

## Assumptions

1. **Bidirectional roads** — The problem states "directed road network" but also says "you may also assume that there is route from Tema back to Abeka." We treat every edge as bidirectional (each CSV row creates edges in both directions).
2. **Same weight both directions** — A road has the same distance and travel time in both directions.
3. **No negative weights** — All distances and times are positive, which is required for Dijkstra's correctness.
4. **"Best route" = lowest total cost** — When recommending a route (Q8, Q9), we compare total cost (fuel + time) and recommend the cheapest option.
5. **Top 3 paths by distance** — Q6 asks for top 3 shortest-distance paths. We use Yen's K-Shortest Paths algorithm with K=3, weighted by distance.
6. **Graph editing is in-memory only** — Q9 edge modifications (add/remove/update) affect the running program but do not write back to the data file.
7. **Scalability estimates assume sparse graphs** — Q10 analysis assumes E ≈ 6V (consistent with the dataset's average degree), not dense graphs.

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
--- Menu ---
 1. Print adjacency list
 2. Show neighbours
 3. Shortest path (distance)
 4. Shortest path (time)
 5. Top 3 shortest-distance paths
 6. Cost analysis + recommendation
 7. Full query (top 3 + time + costs + recommendation)
 8. Add road
 9. Remove road
10. Update road
11. Export graph (DOT file)
 0. Quit
```

### Option 1 — Adjacency list
Prints the first 10 towns with all their direct neighbours, distances, and travel times.

### Option 2 — Neighbours of a town
Lists every directly connected town with distance and travel time.

### Option 3 — Shortest path by distance (Q4)
Dijkstra minimising total kilometres. Reports distance, time, and full route.

### Option 4 — Shortest path by time (Q5)
Dijkstra minimising total travel time. Reports time, distance, and full route.

### Option 5 — Top 3 shortest-distance paths (Q6)
Yen's algorithm with K=3. Shows each path's town sequence, distance, and time.

### Option 6 — Cost analysis + recommendation (Q7, Q8)
Compares shortest-distance vs shortest-time routes side by side:
- Fuel cost (GHS) = `distance / 8 × 11.95`
- Time cost (GHS) = `time × 0.50`
- Total cost = fuel cost + time cost
- Recommends the route with lower total cost and states the savings.

### Option 7 — Full query (Q9)
Combined output for any town pair:
- Top 3 shortest-distance paths with full cost breakdown per path
- Fastest-time path
- Final recommendation (lowest total cost across all paths)

### Options 8–10 — Graph editing (Q9)
Modify the graph at runtime without restarting:
- **Add road**: creates a new bidirectional edge
- **Remove road**: deletes an existing edge (simulates road closure)
- **Update road**: changes distance/time on an existing edge

### Option 11 — Export graph (Q2.3)
Writes a Graphviz DOT file (`graph.dot`). Render with:
```bash
dot -Tpng graph.dot -o graph.png
```

---

## Algorithms

| Question | Algorithm | Complexity |
|---|---|---|
| Q1 | File parsing into adjacency list | O(E) |
| Q2, Q3 | Adjacency list lookup | O(deg(v)) |
| Q4, Q5 | Dijkstra's algorithm (binary heap) | O((V + E) log V) |
| Q6 | Yen's K-Shortest Paths (K=3) | O(KV(V + E) log V) |
| Q7 | Fuel cost formula | O(1) |
| Q8 | Cost analysis (two Dijkstra runs) | O((V + E) log V) |
| Q9 | Edge operations + full query | O(deg(v)) + O(KV(V + E) log V) |

See `complexity_analysis.md` for Q10 scalability discussion (100 to 5000 nodes).

---

## Tests

23 tests covering:
- File loading (valid and invalid files)
- Graph construction (town count, edge count, lookup)
- Neighbour queries
- Dijkstra correctness (distance and time, both directions)
- Path total calculations
- Yen's KSP ordering and count
- Fuel cost and total cost formulas
- Edge operations (add, remove, update)

Run with `make test`.

---

## Known Limitations

- Town name matching is case-sensitive (`Accra` ≠ `accra`).
- Adjacency list display is capped at 10 towns.
- Graph edits are in-memory only (not persisted to file).
