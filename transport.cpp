/*

 * Ghana Road Transport Network Programming Challenge - 2026
 * Ashesi University Department of Computer Science
 *
 * Authors: Edward Ofosu Mensah, Shadrach Nti Agyei
 * Language: C++17
 * Covers  : Questions 1 – 10 (Parts A through E + Advanced)
 *
 * Assumptions: (per the spec)
 *  1. The graph is treated as bidirectional (undirected): per the spec
 *  2. Fuel consumption = 8 km/litre, fuel price = GHS 11.95,
 *  3. Time cost = GHS 0.50 per minute (as given).
 *  

*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <set>
#include <climits>
#include <algorithm>
#include <cstdio>
using namespace std;

// Constants
constexpr double FUEL_CONSUMPTION = 8.0;   // km per litre
constexpr double FUEL_PRICE       = 11.95; // GHS per litre
constexpr double TIME_COST_PER_MIN= 0.50;  // GHS per minute
constexpr int INF = INT_MAX / 2; // infinity

// Helper: strip leading/trailing whitespace
static string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// Helper: join a vector of strings with a separator
static string join(const vector<string>& v, const string& sep) {
    string out;
    for (int i = 0; i < (int)v.size(); ++i) {
        if (i) out += sep;
        out += v[i];
    }
    return out;
}

// Data Structures
struct Edge {
    int to, dist, time;
};

// Main graph class
class TransportGraph {
public:
    // adj[node] = list of Edge
    vector<vector<Edge>> adj;
    vector<string>       towns;          // main graph index → name
    unordered_map<string,int> townIndex; // main graph name  → index
    int N = 0; // number of nodes
    int E = 0; // number of directed edges

    //helpers
    int getOrAddTown(const string& nameofTown) {
        auto it = townIndex.find(nameofTown); // check if the town already exists
        if (it != townIndex.end()) return it->second;
        int id = N++;
        towns.push_back(nameofTown);
        adj.emplace_back();
        townIndex[nameofTown] = id;
        return id;
    }
    
    bool edgeExists(int u, int v) const {
        for (auto& e : adj[u]) if (e.to == v) return true;
        return false;
    }
    // Q1: Load graph from file
    bool loadFromFile(const string& filename) {
        ifstream f(filename);
        if (!f.is_open()) return false;
        string line;
        while (getline(f, line)) {
            istringstream ss(line);
            string src, dst, d_str, t_str;
            if (!getline(ss, src,   ',')) continue;
            if (!getline(ss, dst,   ',')) continue;
            if (!getline(ss, d_str, ',')) continue;
            if (!getline(ss, t_str      )) continue;
            src = trim(src); dst = trim(dst);
            int dist = stoi(d_str), ttime = stoi(t_str);
            int u = getOrAddTown(src);
            int v = getOrAddTown(dst);
            if (!edgeExists(u, v)) {
                adj[u].push_back({v, dist, ttime});
                adj[v].push_back({u, dist, ttime});
                E++;
            }
        }
        return true;
    }
    
  // Q2: Print adjacency list 
  void printAdjacencyList() const {
    cout << "\n━━━━  ADJACENCY LIST (first 10 towns shown)  ━━━━\n";
    int shown = min(N, 10);
    for (int i = 0; i < shown; ++i) {
        cout << "  [" << towns[i] << "]\n";
        for (auto& e : adj[i])
            printf("      → %-22s  dist=%4d km  time=%4d min\n",
                   towns[e.to].c_str(), e.dist, e.time);
    }
    if (N > 10) cout << "  ... (and " << (N-10) << " more towns)\n";
}

  // Q3: Get neighbours 
  void getNeighbours(const string& town) const {
    auto it = townIndex.find(town);
    if (it == townIndex.end()) {
        cout << "  Town \"" << town << "\" not found.\n";
        return;
    }
    int u = it->second;
    if (adj[u].empty()) {
        cout << "  No neighbours for " << town << "\n";
        return;
    }
    cout << "\n  Neighbours of " << town << ":\n";
    cout << "  " << string(55,'-') << "\n";
    printf("  %-22s  %10s  %10s\n","Town","Dist (km)","Time (min)");
    cout << "  " << string(55,'-') << "\n";
    for (auto& e : adj[u])
        printf("  %-22s  %10d  %10d\n",
               towns[e.to].c_str(), e.dist, e.time);
}

// Q4: Dijkstra's algorithm
struct DijkstraResult {
    int cost;
    vector<string> path;
    bool reachable;
};

DijkstraResult dijkstra(const string& srcName, const string& dstName, int weight) const {
    auto it1 = townIndex.find(srcName);
    auto it2 = townIndex.find(dstName);
    if (it1==townIndex.end()||it2==townIndex.end())
        return {INF,{},.reachable=false}; // return inf cost, empty path, and false reachable

    int src = it1->second, dst = it2->second;
    vector<int> dist(N, INF);
    vector<int> prev(N, -1);
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<>> pq;
    dist[src] = 0;
    pq.push({0, src});
    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        for (auto& e : adj[u]) {
            int w = (weight == 0) ? e.dist : e.time;
            if (dist[u] + w < dist[e.to]) {
                dist[e.to] = dist[u] + w;
                prev[e.to] = u;
                pq.push({dist[e.to], e.to});
            }
        }
    }
    if (dist[dst] == INF) return {INF,{},.reachable=false};
    vector<string> path;
    for (int v = dst; v != -1; v = prev[v]) path.push_back(towns[v]);
    reverse(path.begin(), path.end());
    return {dist[dst], path, true};
}

void printDijkstraResult(const DijkstraResult& r, const string& src, const string& dst, int weight) const {
    cout << "\n━━━━  DIJKSTRA'S ALGORITHM (weight=" << weight << ")  ━━━━\n";
    cout << "  Source: " << src << "\n";
    cout << "  Destination: " << dst << "\n";
    cout << "  Cost: " << r.cost << "\n";
    cout << "  Path: " << join(r.path, " → ") << "\n";
}   

    //get total distance AND time for a given path (by name)
    pair<int,int> pathCosts(const vector<string>& path) const {
        int totalDist=0, totalTime=0;
        for (int i=0;i+1<(int)path.size();i++){
            int u = townIndex.at(path[i]);
            int v = townIndex.at(path[i+1]);
            for (auto& e : adj[u]) {
                if (e.to==v){ totalDist+=e.dist; totalTime+=e.time; break; }
            }
        }
        return {totalDist, totalTime};
    }

// Q5: K-Shortest Paths algorithm
struct KSPResult {
    int dist, time;
    vector<string> path;
};

// Dijkstra returning cost + path (internal helper using node indices)
pair<int,vector<int>> dijkstraIdx(int src, int dst, const set<int>& blockedNodes, const set<pair<int,int>>& blockedEdges, int weight) const {
    vector<int> d(N, INF), prev(N, -1);
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<>> pq;
    d[src]=0; pq.push({0,src});
    while (!pq.empty()) {
        auto [cost,u]=pq.top(); pq.pop();
        if (cost>d[u]) continue;
        if (blockedNodes.count(u) && u!=src && u!=dst) continue;
        for (auto& e : adj[u]) {
            if (blockedNodes.count(e.to) && e.to!=dst) continue;
            if (blockedEdges.count({u,e.to})) continue;
            int w = (weight==0)?e.dist:e.time;
            if (d[u]+w < d[e.to]){
                d[e.to]=d[u]+w;
                prev[e.to]=u;
                pq.push({d[e.to],e.to});
            }
        }
    }
    if (d[dst]==INF) return {INF,{}};
    vector<int> path;
    for (int at=dst;at!=-1;at=prev[at]) path.push_back(at);
    reverse(path.begin(), path.end());
    return {d[dst], path};
}

vector<KSPResult> yenKSP(const string& srcName, const string& dstName, int K, int weight=0) const {
    auto it1=townIndex.find(srcName);
    auto it2=townIndex.find(dstName);
    if (it1==townIndex.end()||it2==townIndex.end()) return {};
    int src = it1->second, dst = it2->second;
    vector<KSPResult> results;

    // Helper: convert index-path to KSPResult
    auto toKSP = [&](const vector<int>& p) -> KSPResult {
        vector<string> names;
        for (int v : p) names.push_back(towns[v]);
        auto [d, t] = pathCosts(names);
        return {d, t, names};
    };

    // Seed with the single shortest path
    auto [c0, p0] = dijkstraIdx(src, dst, {}, {}, weight);
    if (c0 == INF || p0.empty()) return {};
    results.push_back(toKSP(p0));

    // Candidate set: {cost, path}
    set<pair<int,vector<int>>> B;

    for (int k = 1; k < K; ++k) {
        // prevPath in node indices
        vector<int> prevPath;
        for (auto& nm : results.back().path) prevPath.push_back(townIndex.at(nm));

        for (int i = 0; i + 1 < (int)prevPath.size(); ++i) {
            int spurNode = prevPath[i];
            vector<int> rootPath(prevPath.begin(), prevPath.begin() + i + 1);

            set<pair<int,int>> blockedEdges;
            set<int>           blockedNodes;

            // Block edges used by previous A-paths sharing the same root
            for (auto& res : results) {
                vector<int> rp;
                for (auto& nm : res.path) rp.push_back(townIndex.at(nm));
                if ((int)rp.size() > i &&
                    vector<int>(rp.begin(), rp.begin()+i+1) == rootPath) {
                    blockedEdges.insert({rp[i],   rp[i+1]});
                    blockedEdges.insert({rp[i+1], rp[i]});   // undirected
                }
            }
            // Block all root-path nodes except the spur node itself
            for (int j = 0; j < i; ++j) blockedNodes.insert(rootPath[j]);

            auto [sc, sp] = dijkstraIdx(spurNode, dst, blockedNodes, blockedEdges, weight);
            if (sc == INF || sp.empty()) continue;

            // Total path = root[0..i] + spur[1..]
            vector<int> total = rootPath;
            total.insert(total.end(), sp.begin()+1, sp.end());

            // Compute cost for ordering in B
            int pathCost = 0;
            for (int j = 0; j+1 < (int)total.size(); ++j) {
                int u = total[j], v = total[j+1];
                for (auto& e : adj[u])
                    if (e.to == v) { pathCost += (weight==0)?e.dist:e.time; break; }
            }
            B.insert({pathCost, total});
        }

        if (B.empty()) break;
        auto best = *B.begin(); B.erase(B.begin());
        results.push_back(toKSP(best.second));
    }
    return results;
}

void printKSP(const vector<KSPResult>& paths, const string& src, const string& dst, int weight) const {
    cout << "\n━━━━  K-SHORTEST PATHS (weight=" << weight << ")  ━━━━\n";
    cout << "  Source: " << src << "\n";
    cout << "  Destination: " << dst << "\n";
    cout << "  Paths: " << paths.size() << "\n";
    for (int i=0;i<(int)paths.size();i++){
        cout << "  Path " << i+1 << ":\n";
        cout << "    Distance: " << paths[i].dist << " km\n";
        cout << "    Time: " << paths[i].time << " min\n";
        cout << "    Path: " << join(paths[i].path, " → ") << "\n";
    }
}

// Q7: Cost functions
double fuelCost(int distKm) const {
    return (distKm / FUEL_CONSUMPTION) * FUEL_PRICE;
}
double totalCost(int distKm, int timeMin) const {
    return fuelCost(distKm) + timeMin * TIME_COST_PER_MIN;
}

// Q8: Cost analysis — cheapest route by total GHS cost
void printCostAnalysis(const string& src, const string& dst) const {
    // Cheapest by distance weight, then cheapest by time weight
    auto rd = dijkstra(src, dst, 0);
    auto rt = dijkstra(src, dst, 1);

    cout << "\n━━━━  COST ANALYSIS: " << src << " → " << dst << "  ━━━━\n";

    auto printRoute = [&](const DijkstraResult& r, const char* label) {
        if (!r.reachable) { cout << "  " << label << ": unreachable\n"; return; }
        auto [d, t] = pathCosts(r.path);
        double fc   = fuelCost(d);
        double tc   = t * TIME_COST_PER_MIN;
        double tot  = fc + tc;
        printf("  %-25s  dist=%4d km  time=%4d min\n", label, d, t);
        printf("    Fuel cost   : GHS %8.2f\n", fc);
        printf("    Time cost   : GHS %8.2f\n", tc);
        printf("    TOTAL cost  : GHS %8.2f\n", tot);
        cout << "    Route: " << join(r.path, " → ") << "\n";
    };

    printRoute(rd, "Shortest-distance route");
    printRoute(rt, "Shortest-time route");
}

// ─── Interactive menu ─────────────────────────────────────────────────────────
void runInteractive() const {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║   Ghana Road Transport Network  (2026)       ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";
    cout << "  Nodes: " << N << "   Edges: " << E << "\n";

    while (true) {
        cout << "\n┌─── MENU ──────────────────────────────────┐\n";
        cout << "│  1. Print adjacency list (first 10 towns) │\n";
        cout << "│  2. Show neighbours of a town             │\n";
        cout << "│  3. Shortest path (distance)              │\n";
        cout << "│  4. Shortest path (time)                  │\n";
        cout << "│  5. K-shortest paths                      │\n";
        cout << "│  6. Cost analysis (fuel + time cost)      │\n";
        cout << "│  0. Quit                                  │\n";
        cout << "└───────────────────────────────────────────┘\n";
        cout << "Choice: ";

        int choice;
        if (!(cin >> choice)) { cin.clear(); cin.ignore(1000,'\n'); continue; }
        cin.ignore(1000, '\n');

        if (choice == 0) { cout << "Goodbye!\n"; break; }

        string src, dst;
        switch (choice) {
        case 1:
            printAdjacencyList();
            break;

        case 2:
            cout << "Town name: ";
            getline(cin, src);
            getNeighbours(src);
            break;

        case 3:
        case 4: {
            int w = (choice == 3) ? 0 : 1;
            cout << "Source town: ";      getline(cin, src);
            cout << "Destination town: "; getline(cin, dst);
            auto r = dijkstra(src, dst, w);
            if (!r.reachable)
                cout << "  No route found.\n";
            else {
                auto [d, t] = pathCosts(r.path);
                printf("\n  %-12s : %d %s\n", (w==0?"Distance":"Time"),
                       r.cost, (w==0?"km":"min"));
                printf("  %-12s : %d km\n",   "Total dist", d);
                printf("  %-12s : %d min\n",  "Total time", t);
                cout << "  Route: " << join(r.path, " → ") << "\n";
            }
            break;
        }

        case 5: {
            int K;
            cout << "Source town: ";      getline(cin, src);
            cout << "Destination town: "; getline(cin, dst);
            cout << "K (number of paths): "; cin >> K; cin.ignore(1000,'\n');
            cout << "Weight — 0=distance, 1=time: "; int w; cin >> w; cin.ignore(1000,'\n');
            auto paths = yenKSP(src, dst, K, w);
            if (paths.empty()) cout << "  No paths found.\n";
            else               printKSP(paths, src, dst, w);
            break;
        }

        case 6:
            cout << "Source town: ";      getline(cin, src);
            cout << "Destination town: "; getline(cin, dst);
            printCostAnalysis(src, dst);
            break;

        default:
            cout << "  Invalid choice.\n";
        }
    }
}

}; // end class TransportGraph

// ─── main ────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    string filename = "ghana_cities_graph_2026.txt";
    if (argc > 1) filename = argv[1];

    TransportGraph G;
    if (!G.loadFromFile(filename)) {
        cerr << "Error: could not open \"" << filename << "\"\n";
        return 1;
    }
    cout << "Loaded graph: " << G.N << " towns, " << G.E << " edges.\n";

    G.runInteractive();
    return 0;
}
