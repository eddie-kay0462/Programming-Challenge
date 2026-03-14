# Complexity Analysis (Question 10)

## Algorithm Complexities

| Algorithm | Used For | Time Complexity |
|-----------|----------|-----------------|
| File parsing | Q1 | O(E) |
| Adjacency list lookup | Q2, Q3 | O(deg(v)) |
| Dijkstra (binary heap) | Q4, Q5, Q7, Q8 | O((V + E) log V) |
| Yen's K-Shortest Paths | Q6, Q9 | O(K * V * (V + E) log V) |
| Edge update/remove | Q9 | O(deg(v)) |

Where V = number of towns, E = number of edges, K = number of paths requested.

## Scalability Analysis

For a road network, E is roughly proportional to V (sparse graph, average degree ~6 for our dataset: 551 edges / 183 towns ≈ 3 edges per town per direction). We assume E ≈ 6V for estimates.

### Dijkstra: O((V + E) log V) ≈ O(V log V)

| V (towns) | Estimated operations (V log V) | Relative to V=183 |
|-----------|-------------------------------|---------------------|
| 100       | 664                           | 0.4x |
| 183       | 1,352                         | 1.0x (baseline) |
| 500       | 4,483                         | 3.3x |
| 1,000     | 9,966                         | 7.4x |
| 2,000     | 21,931                        | 16.2x |
| 5,000     | 61,439                        | 45.4x |

### Yen's K=3: O(3 * V * (V + E) log V) ≈ O(V² log V)

| V (towns) | Estimated operations (3 * V² * log V) | Relative to V=183 |
|-----------|---------------------------------------|---------------------|
| 100       | 199,316                               | 0.3x |
| 183       | 714,093                               | 1.0x (baseline) |
| 500       | 6,724,785                             | 9.4x |
| 1,000     | 29,897,352                            | 41.9x |
| 2,000     | 131,587,200                           | 184.3x |
| 5,000     | 922,185,889                           | 1291.4x |

## Key Observations

1. **Dijkstra scales well** — going from 183 to 5000 towns is only ~45x more work. For a modern machine doing ~10⁸ operations/second, even 5000 nodes completes in under 1ms.

2. **Yen's KSP is the bottleneck** — the V² factor means 5000 nodes is ~1300x slower than 183 nodes. At 5000 nodes this is still feasible (under 1 second), but would become problematic beyond ~50,000 nodes.

3. **Road networks are sparse** — the E ≈ 6V assumption holds because each town connects to a bounded number of neighbors. Dense graphs (E ≈ V²) would make Dijkstra O(V² log V) and Yen's O(V³ log V).

4. **Memory is not a concern** — the adjacency list uses O(V + E) space. At 5000 nodes with E ≈ 30,000 edges, this is negligible.

## Scalability Plot Data (CSV)

```
V,Dijkstra_ops,Yen_K3_ops
100,664,199316
183,1352,714093
500,4483,6724785
1000,9966,29897352
2000,21931,131587200
5000,61439,922185889
```

To generate the plot, save the CSV above and use any plotting tool (Excel, matplotlib, gnuplot):

```
gnuplot -e "set terminal png; set output 'scalability.png'; set xlabel 'Towns (V)'; set ylabel 'Operations'; set logscale y; plot 'data.csv' using 1:2 with linespoints title 'Dijkstra', '' using 1:3 with linespoints title 'Yen K=3'"
```
