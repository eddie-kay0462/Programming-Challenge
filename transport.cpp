
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

// fuel and time cost constants (from the assignment sheet)
#define FUEL_CONSUMPTION 8.0    // km per litre
#define FUEL_PRICE 11.95        // GHS per litre
#define TIME_COST 0.50          // GHS per min
const int INF = INT_MAX / 2;

// trim whitespace from both ends
string trim(const string& s) {
    int a = s.find_first_not_of(" \t\r\n");
    if (a == (int)string::npos) return "";
    int b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// joins a vector of strings with sep between them
string join(const vector<string>& v, const string& sep) {
    string res = "";
    for (int i = 0; i < (int)v.size(); i++) {
        if (i > 0) res += sep;
        res += v[i];
    }
    return res;
}

struct Edge {
    int to, dist, time;
};

class TransportGraph {
public:
    vector<vector<Edge>> adj;
    vector<string> towns;
    unordered_map<string,int> townIndex;
    int N = 0;
    int E = 0;

    int getOrAddTown(const string& name) {
        auto it = townIndex.find(name);
        if (it != townIndex.end()) return it->second;
        int id = N++;
        towns.push_back(name);
        adj.emplace_back();
        townIndex[name] = id;
        return id;
    }

    bool edgeExists(int u, int v) const {
        for (int i = 0; i < (int)adj[u].size(); i++)
            if (adj[u][i].to == v) return true;
        return false;
    }

    bool loadFromFile(const string& filename) {
        ifstream f(filename);
        if (!f.is_open()) {
            cerr << "could not open file: " << filename << "\n";
            return false;
        }
        string line;
        while (getline(f, line)) {
            if (line.empty()) continue;
            istringstream ss(line);
            string src, dst, ds, ts;
            if (!getline(ss, src, ',')) continue;
            if (!getline(ss, dst, ',')) continue;
            if (!getline(ss, ds,  ',')) continue;
            if (!getline(ss, ts      )) continue;
            src = trim(src);
            dst = trim(dst);
            int dist = stoi(ds);
            int ttime = stoi(ts);
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

    void printAdjacencyList() const {
        int shown = (N < 10) ? N : 10;
        cout << "\nAdjacency list (showing first " << shown << " towns):\n";
        for (int i = 0; i < shown; i++) {
            cout << "[" << towns[i] << "]\n";
            for (auto& e : adj[i])
                printf("   -> %-22s  dist=%d km  time=%d min\n",
                       towns[e.to].c_str(), e.dist, e.time);
        }
        if (N > 10)
            cout << "...(" << N-10 << " more towns not shown)\n";
    }

    void getNeighbours(const string& town) const {
        auto it = townIndex.find(town);
        if (it == townIndex.end()) {
            cout << "Town not found: " << town << "\n";
            return;
        }
        int u = it->second;
        if (adj[u].empty()) {
            cout << town << " has no neighbours\n";
            return;
        }
        cout << "\nNeighbours of " << town << ":\n";
        printf("  %-22s  %10s  %10s\n", "Town", "Dist(km)", "Time(min)");
        cout << string(46, '-') << "\n";
        for (auto& e : adj[u])
            printf("  %-22s  %10d  %10d\n", towns[e.to].c_str(), e.dist, e.time);
    }

    // result struct for dijkstra
    struct DijkResult {
        int cost;
        vector<string> path;
        bool found;
    };

    DijkResult dijkstra(const string& srcName, const string& dstName, int useTime) const {
        auto it1 = townIndex.find(srcName);
        auto it2 = townIndex.find(dstName);
        if (it1 == townIndex.end() || it2 == townIndex.end())
            return {INF, {}, false};

        int src = it1->second, dst = it2->second;
        vector<int> d(N, INF), prev(N, -1);
        priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
        d[src] = 0;
        pq.push(make_pair(0, src));

        while (!pq.empty()) {
            int cost = pq.top().first;
            int u    = pq.top().second;
            pq.pop();
            if (cost > d[u]) continue;
            for (int i = 0; i < (int)adj[u].size(); i++) {
                Edge e = adj[u][i];
                int w = useTime ? e.time : e.dist;
                if (d[u] + w < d[e.to]) {
                    d[e.to] = d[u] + w;
                    prev[e.to] = u;
                    pq.push(make_pair(d[e.to], e.to));
                }
            }
        }

        if (d[dst] == INF) return {INF, {}, false};

        vector<string> path;
        for (int v = dst; v != -1; v = prev[v])
            path.push_back(towns[v]);
        reverse(path.begin(), path.end());
        return {d[dst], path, true};
    }

    // get total dist and time for a path (needed separately since dijkstra only returns one)
    pair<int,int> pathTotals(const vector<string>& path) const {
        int td = 0, tt = 0;
        for (int i = 0; i+1 < (int)path.size(); i++) {
            int u = townIndex.at(path[i]);
            int v = townIndex.at(path[i+1]);
            for (auto& e : adj[u]) {
                if (e.to == v) { td += e.dist; tt += e.time; break; }
            }
        }
        return make_pair(td, tt);
    }

    // KSP stuff
    struct KSPEntry {
        int dist, time;
        vector<string> path;
    };

    // dijkstra on node indices — used internally by Yen's
    pair<int, vector<int>> _dijkIdx(int src, int dst,
                                     const set<int>& blockedNodes,
                                     const set<pair<int,int>>& blockedEdges,
                                     int useTime) const {
        vector<int> d(N, INF), prev(N, -1);
        priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
        d[src] = 0;
        pq.push({0, src});
        while (!pq.empty()) {
            auto [c, u] = pq.top(); pq.pop();
            if (c > d[u]) continue;
            if (blockedNodes.count(u) && u != src && u != dst) continue;
            for (auto& e : adj[u]) {
                if (blockedNodes.count(e.to) && e.to != dst) continue;
                if (blockedEdges.count({u, e.to})) continue;
                int w = useTime ? e.time : e.dist;
                if (d[u] + w < d[e.to]) {
                    d[e.to] = d[u] + w;
                    prev[e.to] = u;
                    pq.push({d[e.to], e.to});
                }
            }
        }
        if (d[dst] == INF) return {INF, {}};
        vector<int> path;
        for (int at = dst; at != -1; at = prev[at]) path.push_back(at);
        reverse(path.begin(), path.end());
        return {d[dst], path};
    }

    vector<KSPEntry> yenKSP(const string& srcName, const string& dstName, int K, int useTime = 0) const {
        auto it1 = townIndex.find(srcName);
        auto it2 = townIndex.find(dstName);
        if (it1 == townIndex.end() || it2 == townIndex.end()) return {};

        int src = it1->second, dst = it2->second;
        vector<KSPEntry> results;

        auto toEntry = [&](const vector<int>& p) -> KSPEntry {
            vector<string> names;
            for (int v : p) names.push_back(towns[v]);
            auto [d, t] = pathTotals(names);
            return {d, t, names};
        };

        auto [c0, p0] = _dijkIdx(src, dst, {}, {}, useTime);
        if (c0 == INF || p0.empty()) return {};
        results.push_back(toEntry(p0));

        set<pair<int, vector<int>>> B;

        for (int k = 1; k < K; k++) {
            vector<int> prevPath;
            for (auto& nm : results.back().path)
                prevPath.push_back(townIndex.at(nm));

            for (int i = 0; i+1 < (int)prevPath.size(); i++) {
                int spurNode = prevPath[i];
                vector<int> root(prevPath.begin(), prevPath.begin() + i + 1);

                set<pair<int,int>> bEdges;
                set<int> bNodes;

                for (auto& r : results) {
                    vector<int> rp;
                    for (auto& nm : r.path) rp.push_back(townIndex.at(nm));
                    if ((int)rp.size() > i &&
                        vector<int>(rp.begin(), rp.begin()+i+1) == root) {
                        bEdges.insert({rp[i], rp[i+1]});
                        bEdges.insert({rp[i+1], rp[i]});
                    }
                }
                for (int j = 0; j < i; j++) bNodes.insert(root[j]);

                auto [sc, sp] = _dijkIdx(spurNode, dst, bNodes, bEdges, useTime);
                if (sc == INF || sp.empty()) continue;

                vector<int> total = root;
                total.insert(total.end(), sp.begin()+1, sp.end());

                int pc = 0;
                for (int j = 0; j+1 < (int)total.size(); j++) {
                    int u = total[j], v = total[j+1];
                    for (auto& e : adj[u])
                        if (e.to == v) { pc += useTime ? e.time : e.dist; break; }
                }
                B.insert({pc, total});
            }

            if (B.empty()) break;
            auto best = *B.begin(); B.erase(B.begin());
            results.push_back(toEntry(best.second));
        }
        return results;
    }

    void printKSP(const vector<KSPEntry>& paths, const string& src, const string& dst) const {
        cout << "\nK-Shortest Paths from " << src << " to " << dst << ":\n";
        for (int i = 0; i < (int)paths.size(); i++) {
            cout << "  Path " << i+1 << ": dist=" << paths[i].dist
                 << " km, time=" << paths[i].time << " min\n";
            cout << "    " << join(paths[i].path, " -> ") << "\n";
        }
    }

    double calcFuelCost(int distKm) const {
        return (distKm / FUEL_CONSUMPTION) * FUEL_PRICE;
    }

    void printCostAnalysis(const string& src, const string& dst) const {
        auto rd = dijkstra(src, dst, 0);
        auto rt = dijkstra(src, dst, 1);

        cout << "\nCost Analysis: " << src << " -> " << dst << "\n";

        auto show = [&](const DijkResult& r, const char* label) {
            if (!r.found) { cout << label << ": no route\n"; return; }
            auto [d, t] = pathTotals(r.path);
            double fc  = calcFuelCost(d);
            double tc  = t * TIME_COST;
            printf("  %s\n", label);
            printf("    dist=%d km, time=%d min\n", d, t);
            printf("    fuel cost : GHS %.2f\n", fc);
            printf("    time cost : GHS %.2f\n", tc);
            printf("    total     : GHS %.2f\n", fc + tc);
            cout << "    route: " << join(r.path, " -> ") << "\n";
        };

        show(rd, "By shortest distance");
        show(rt, "By shortest time");
    }

    void run() {
        cout << "\nGhana Road Transport Network (2026)\n";
        cout << "Loaded: " << N << " towns, " << E << " edges\n";

        while (true) {
            cout << "\n--- Menu ---\n";
            cout << "1. Print adjacency list\n";
            cout << "2. Show neighbours\n";
            cout << "3. Shortest path (distance)\n";
            cout << "4. Shortest path (time)\n";
            cout << "5. K-shortest paths\n";
            cout << "6. Cost analysis\n";
            cout << "0. Quit\n";
            cout << "Choice: ";

            int choice;
            if (!(cin >> choice)) { cin.clear(); cin.ignore(1000,'\n'); continue; }
            cin.ignore(1000, '\n');

            if (choice == 0) { cout << "bye\n"; break; }

            string src, dst;
            if (choice == 1) {
                printAdjacencyList();
            } else if (choice == 2) {
                cout << "Town: "; getline(cin, src);
                getNeighbours(src);
            } else if (choice == 3 || choice == 4) {
                int w = (choice == 3) ? 0 : 1;
                cout << "Source: ";      getline(cin, src);
                cout << "Destination: "; getline(cin, dst);
                auto r = dijkstra(src, dst, w);
                if (!r.found) {
                    cout << "No route found.\n";
                } else {
                    auto [d, t] = pathTotals(r.path);
                    printf("  cost=%d %s\n", r.cost, w ? "min" : "km");
                    printf("  total dist=%d km, total time=%d min\n", d, t);
                    cout << "  route: " << join(r.path, " -> ") << "\n";
                }
            } else if (choice == 5) {
                int K, w;
                cout << "Source: ";      getline(cin, src);
                cout << "Destination: "; getline(cin, dst);
                cout << "K: ";           cin >> K; cin.ignore(1000, '\n');
                cout << "0=distance 1=time: "; cin >> w; cin.ignore(1000, '\n');
                auto paths = yenKSP(src, dst, K, w);
                if (paths.empty())
                    cout << "No paths found.\n";
                else
                    printKSP(paths, src, dst);
            } else if (choice == 6) {
                cout << "Source: ";      getline(cin, src);
                cout << "Destination: "; getline(cin, dst);
                printCostAnalysis(src, dst);
            } else {
                cout << "invalid choice\n";
            }
        }
    }
};

int main(int argc, char* argv[]) {
    string file = "ghana_cities_graph_2026.txt";
    if (argc > 1) file = argv[1];

    TransportGraph G;
    if (!G.loadFromFile(file)) return 1;

    G.run();
    return 0;
}
