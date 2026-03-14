# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Ghana Road Transport Network solver — models 183 towns and 551 roads as an undirected weighted graph (distance in km, travel time in minutes). C++17 program with an interactive terminal menu for shortest-path queries, K-shortest paths (Yen's algorithm), cost analysis, graph editing, and route recommendation.

## Build & Run

```bash
make transport          # build main program
make test               # build and run all tests
make clean              # remove binaries
./transport             # uses ghana_cities_graph_2026.txt
./transport other.txt   # custom data file
```

## Architecture

- **`transport.h`** — `TransportGraph` class with all algorithms and the interactive menu. Helpers: `trim()`, `join()`, `Edge` struct.
- **`transport.cpp`** — just `main()`, loads data file and calls `G.run()`.
- **`test_transport.cpp`** — 23 modular tests using assert-based macros on a small hand-built graph. No external test framework.
- **Data file**: `ghana_cities_graph_2026.txt` — plain-text CSV (`Source, Destination, Distance(km), Time(min)`), one edge per line, no header. Each line adds edges in both directions.
- **Key types**: `Edge` struct (to, dist, time); `TransportGraph` holds `vector<vector<Edge>> adj`, town name <-> index mapping via `unordered_map<string,int> townIndex`.
- **Algorithms**: Dijkstra with binary heap; Yen's K-shortest simple paths. Both support weight mode: distance (0) or time (1).
- **Constants**: fuel 8 km/L, fuel price GHS 11.95/L, time cost GHS 0.50/min.

## Known Limitations

- Town name matching is case-sensitive.
- Adjacency list display is capped at 10 towns.
