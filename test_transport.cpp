#include "transport.h"
#include <cassert>
#include <cmath>

int tests_passed = 0;
int tests_failed = 0;

#define TEST(name) \
    cout << "  " << name << "... "; \
    try

#define PASS \
    cout << "PASSED\n"; tests_passed++;

#define FAIL(msg) \
    cout << "FAILED: " << msg << "\n"; tests_failed++;

// Build a small test graph:
//
//   A --10km/5min-- B --20km/15min-- D
//   |               |
//   30km/10min     5km/20min
//   |               |
//   C --15km/8min-- E
//
TransportGraph buildTestGraph() {
    TransportGraph G;
    G.addEdge("A", "B", 10, 5);
    G.addEdge("A", "C", 30, 10);
    G.addEdge("B", "D", 20, 15);
    G.addEdge("B", "E", 5, 20);
    G.addEdge("C", "E", 15, 8);
    return G;
}

void testGraphConstruction() {
    cout << "[Graph Construction]\n";
    auto G = buildTestGraph();

    TEST("town count") {
        assert(G.N == 5);
        PASS
    } catch(...) { FAIL("expected 5 towns") }

    TEST("edge count") {
        assert(G.E == 5);
        PASS
    } catch(...) { FAIL("expected 5 edges") }

    TEST("town lookup") {
        assert(G.townIndex.count("A"));
        assert(G.townIndex.count("B"));
        assert(G.townIndex.count("E"));
        assert(!G.townIndex.count("Z"));
        PASS
    } catch(...) { FAIL("town lookup failed") }
}

void testNeighbours() {
    cout << "[Neighbours]\n";
    auto G = buildTestGraph();

    TEST("A has 2 neighbours") {
        int u = G.townIndex["A"];
        assert((int)G.adj[u].size() == 2);
        PASS
    } catch(...) { FAIL("expected 2 neighbours for A") }

    TEST("B has 3 neighbours") {
        int u = G.townIndex["B"];
        assert((int)G.adj[u].size() == 3);
        PASS
    } catch(...) { FAIL("expected 3 neighbours for B") }
}

void testDijkstraDistance() {
    cout << "[Dijkstra - Distance]\n";
    auto G = buildTestGraph();

    TEST("A->D shortest distance = 30 (A->B->D)") {
        auto r = G.dijkstra("A", "D", 0);
        assert(r.found);
        assert(r.cost == 30);
        assert(r.path.size() == 3);
        assert(r.path[0] == "A" && r.path[1] == "B" && r.path[2] == "D");
        PASS
    } catch(...) { FAIL("wrong shortest distance path") }

    TEST("A->E shortest distance = 15 (A->B->E)") {
        auto r = G.dijkstra("A", "E", 0);
        assert(r.found);
        assert(r.cost == 15);
        PASS
    } catch(...) { FAIL("wrong shortest distance to E") }

    TEST("nonexistent town returns not found") {
        auto r = G.dijkstra("A", "Z", 0);
        assert(!r.found);
        PASS
    } catch(...) { FAIL("should not find path to Z") }
}

void testDijkstraTime() {
    cout << "[Dijkstra - Time]\n";
    auto G = buildTestGraph();

    TEST("A->E fastest = 18 min (A->C->E)") {
        auto r = G.dijkstra("A", "E", 1);
        assert(r.found);
        assert(r.cost == 18);
        assert(r.path[0] == "A" && r.path[1] == "C" && r.path[2] == "E");
        PASS
    } catch(...) { FAIL("wrong fastest path to E") }

    TEST("A->D fastest = 20 min (A->B->D)") {
        auto r = G.dijkstra("A", "D", 1);
        assert(r.found);
        assert(r.cost == 20);
        PASS
    } catch(...) { FAIL("wrong fastest path to D") }
}

void testPathTotals() {
    cout << "[Path Totals]\n";
    auto G = buildTestGraph();

    TEST("A->B->D totals = 30km, 20min") {
        vector<string> path = {"A", "B", "D"};
        auto [d, t] = G.pathTotals(path);
        assert(d == 30 && t == 20);
        PASS
    } catch(...) { FAIL("wrong path totals") }

    TEST("A->C->E totals = 45km, 18min") {
        vector<string> path = {"A", "C", "E"};
        auto [d, t] = G.pathTotals(path);
        assert(d == 45 && t == 18);
        PASS
    } catch(...) { FAIL("wrong path totals") }
}

void testYenKSP() {
    cout << "[Yen's KSP]\n";
    auto G = buildTestGraph();

    TEST("A->D top 3 returns paths sorted by distance") {
        auto paths = G.yenKSP("A", "D", 3, 0);
        assert(!paths.empty());
        assert(paths[0].dist <= paths[1].dist || paths.size() == 1);
        assert(paths[0].dist == 30); // A->B->D
        PASS
    } catch(...) { FAIL("KSP ordering wrong") }

    TEST("A->D returns at most 3 paths") {
        auto paths = G.yenKSP("A", "D", 3, 0);
        assert((int)paths.size() <= 3);
        PASS
    } catch(...) { FAIL("too many paths") }
}

void testFuelCost() {
    cout << "[Fuel Cost]\n";
    auto G = buildTestGraph();

    TEST("100km fuel cost = (100/8)*11.95 = 149.375") {
        double fc = G.calcFuelCost(100);
        assert(fabs(fc - 149.375) < 0.01);
        PASS
    } catch(...) { FAIL("wrong fuel cost") }

    TEST("0km fuel cost = 0") {
        double fc = G.calcFuelCost(0);
        assert(fabs(fc) < 0.01);
        PASS
    } catch(...) { FAIL("0km should be 0 cost") }
}

void testTotalCost() {
    cout << "[Total Cost]\n";
    auto G = buildTestGraph();

    TEST("100km, 60min total = 149.375 + 30.0 = 179.375") {
        double tc = G.calcTotalCost(100, 60);
        assert(fabs(tc - 179.375) < 0.01);
        PASS
    } catch(...) { FAIL("wrong total cost") }
}

void testEdgeOperations() {
    cout << "[Edge Operations]\n";
    auto G = buildTestGraph();

    TEST("update edge A->B to 50km/25min") {
        assert(G.updateEdge("A", "B", 50, 25));
        auto r = G.dijkstra("A", "B", 0);
        assert(r.found && r.cost == 50);
        PASS
    } catch(...) { FAIL("edge update failed") }

    TEST("remove edge A->C") {
        assert(G.removeEdge("A", "C"));
        assert(G.E == 4);
        int u = G.townIndex["A"];
        bool found = false;
        int cv = G.townIndex["C"];
        for (auto& e : G.adj[u])
            if (e.to == cv) found = true;
        assert(!found);
        PASS
    } catch(...) { FAIL("edge removal failed") }

    TEST("add new edge D->C 12km/7min") {
        G.addEdge("D", "C", 12, 7);
        assert(G.E == 5);
        assert(G.edgeExists(G.townIndex["D"], G.townIndex["C"]));
        assert(G.edgeExists(G.townIndex["C"], G.townIndex["D"]));
        PASS
    } catch(...) { FAIL("edge add failed") }

    TEST("update nonexistent edge returns false") {
        assert(!G.updateEdge("A", "Z", 1, 1));
        PASS
    } catch(...) { FAIL("should return false for missing edge") }
}

void testFileLoading() {
    cout << "[File Loading]\n";

    TEST("load test data from small file") {
        // write a temp file
        ofstream f("_test_data.txt");
        f << "X, Y, 10, 5\n";
        f << "Y, Z, 20, 15\n";
        f << "X, Z, 25, 12\n";
        f.close();

        TransportGraph G;
        assert(G.loadFromFile("_test_data.txt"));
        assert(G.N == 3);
        assert(G.E == 3);
        remove("_test_data.txt");
        PASS
    } catch(...) { remove("_test_data.txt"); FAIL("file loading failed") }

    TEST("load nonexistent file returns false") {
        TransportGraph G;
        assert(!G.loadFromFile("nonexistent_file_12345.txt"));
        PASS
    } catch(...) { FAIL("should fail on missing file") }
}

int main() {
    cout << "\n=== Transport Graph Tests ===\n\n";

    testFileLoading();
    testGraphConstruction();
    testNeighbours();
    testDijkstraDistance();
    testDijkstraTime();
    testPathTotals();
    testYenKSP();
    testFuelCost();
    testTotalCost();
    testEdgeOperations();

    cout << "\n=== Results: " << tests_passed << " passed, "
         << tests_failed << " failed ===\n";

    return tests_failed > 0 ? 1 : 0;
}
