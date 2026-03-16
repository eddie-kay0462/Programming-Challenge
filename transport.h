#pragma once

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
#include <cmath>
using namespace std;

#define FUEL_CONSUMPTION 8.0    // km per litre
#define FUEL_PRICE 11.95        // GHS per litre
#define TIME_COST 0.50          // GHS per min
const int INF = INT_MAX / 2;

string trim(const string& s) {
    int a = s.find_first_not_of(" \t\r\n");
    if (a == (int)string::npos) return "";
    int b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

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
            // skip CSV header
            if (line.find("source") != string::npos && line.find("destination") != string::npos)
                continue;
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

    struct KSPEntry {
        int dist, time;
        vector<string> path;
    };

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
        cout << "\nTop " << paths.size() << " Shortest-Distance Paths: "
             << src << " -> " << dst << "\n";
        for (int i = 0; i < (int)paths.size(); i++) {
            cout << "  Path " << i+1 << ": dist=" << paths[i].dist
                 << " km, time=" << paths[i].time << " min\n";
            cout << "    " << join(paths[i].path, " -> ") << "\n";
        }
    }

    double calcFuelCost(int distKm) const {
        return (distKm / FUEL_CONSUMPTION) * FUEL_PRICE;
    }

    double calcTotalCost(int distKm, int timeMin) const {
        return calcFuelCost(distKm) + timeMin * TIME_COST;
    }

    void printCostAnalysis(const string& src, const string& dst) const {
        auto rd = dijkstra(src, dst, 0);
        auto rt = dijkstra(src, dst, 1);

        if (!rd.found && !rt.found) {
            cout << "No route found between " << src << " and " << dst << "\n";
            return;
        }

        cout << "\n...... COST ANALYSIS: " << src << " -> " << dst << "  ......\n";

        double costDist = 0, costTime = 0;

        auto show = [&](const DijkResult& r, const char* label) -> double {
            if (!r.found) { cout << "  " << label << ": no route\n"; return 1e18; }
            auto [d, t] = pathTotals(r.path);
            double fc  = calcFuelCost(d);
            double tc  = t * TIME_COST;
            double total = fc + tc;
            printf("  %s\n", label);
            printf("    dist=%d km, time=%d min\n", d, t);
            printf("    fuel cost : GHS %.2f\n", fc);
            printf("    time cost : GHS %.2f\n", tc);
            printf("    TOTAL     : GHS %.2f\n", total);
            cout << "    route: " << join(r.path, " -> ") << "\n";
            return total;
        };

        costDist = show(rd, "Shortest-distance route");
        costTime = show(rt, "Shortest-time route");

        cout << "\n  >> RECOMMENDATION: ";
        if (costDist <= costTime) {
            printf("Take the shortest-distance route (saves GHS %.2f)\n", costTime - costDist);
            cout << "     Lower fuel cost outweighs the extra travel time.\n";
        } else {
            printf("Take the shortest-time route (saves GHS %.2f)\n", costDist - costTime);
            cout << "     Time savings more than offset the higher fuel cost.\n";
        }
    }

    // Q9: graph editing operations
    bool updateEdge(const string& srcName, const string& dstName, int newDist, int newTime) {
        auto it1 = townIndex.find(srcName);
        auto it2 = townIndex.find(dstName);
        if (it1 == townIndex.end() || it2 == townIndex.end()) return false;
        int u = it1->second, v = it2->second;
        bool found = false;
        for (auto& e : adj[u])
            if (e.to == v) { e.dist = newDist; e.time = newTime; found = true; break; }
        for (auto& e : adj[v])
            if (e.to == u) { e.dist = newDist; e.time = newTime; found = true; break; }
        return found;
    }

    bool removeEdge(const string& srcName, const string& dstName) {
        auto it1 = townIndex.find(srcName);
        auto it2 = townIndex.find(dstName);
        if (it1 == townIndex.end() || it2 == townIndex.end()) return false;
        int u = it1->second, v = it2->second;
        bool found = false;
        auto& au = adj[u];
        for (int i = 0; i < (int)au.size(); i++)
            if (au[i].to == v) { au.erase(au.begin() + i); found = true; break; }
        auto& av = adj[v];
        for (int i = 0; i < (int)av.size(); i++)
            if (av[i].to == u) { av.erase(av.begin() + i); found = true; break; }
        if (found) E--;
        return found;
    }

    void addEdge(const string& srcName, const string& dstName, int dist, int time) {
        int u = getOrAddTown(srcName);
        int v = getOrAddTown(dstName);
        if (!edgeExists(u, v)) {
            adj[u].push_back({v, dist, time});
            adj[v].push_back({u, dist, time});
            E++;
        }
    }

    // Q9: full query — top 3 paths + cost comparison + recommendation
    void fullQuery(const string& src, const string& dst) const {
        cout << "\n..............................................................................\n";
        cout << "  FULL QUERY: " << src << " -> " << dst << "\n";
        cout << "..............................................................................\n";

        auto paths = yenKSP(src, dst, 3, 0);
        if (paths.empty()) {
            cout << "No paths found.\n";
            return;
        }

        int bestIdx = 0;
        double bestCost = 1e18;

        cout << "\nTop " << paths.size() << " Shortest-Distance Paths:\n";
        for (int i = 0; i < (int)paths.size(); i++) {
            double fc = calcFuelCost(paths[i].dist);
            double tc = paths[i].time * TIME_COST;
            double total = fc + tc;

            printf("\n  Path %d: %s\n", i+1, join(paths[i].path, " -> ").c_str());
            printf("    Distance  : %d km\n", paths[i].dist);
            printf("    Time      : %d min\n", paths[i].time);
            printf("    Fuel cost : GHS %.2f\n", fc);
            printf("    Time cost : GHS %.2f\n", tc);
            printf("    TOTAL cost: GHS %.2f\n", total);

            if (total < bestCost) {
                bestCost = total;
                bestIdx = i;
            }
        }

        auto fastest = dijkstra(src, dst, 1);
        if (fastest.found) {
            auto [fd, ft] = pathTotals(fastest.path);
            cout << "\nFastest-Time Path:\n";
            printf("  %s\n", join(fastest.path, " -> ").c_str());
            printf("  Distance: %d km, Time: %d min\n", fd, ft);
            double fc = calcFuelCost(fd);
            double tc = ft * TIME_COST;
            printf("  Total cost: GHS %.2f\n", fc + tc);

            if (fc + tc < bestCost) {
                bestCost = fc + tc;
                bestIdx = -1;
            }
        }

        cout << "\n>> RECOMMENDATION: ";
        if (bestIdx == -1) {
            printf("Take the fastest-time path (lowest total cost: GHS %.2f)\n", bestCost);
        } else {
            printf("Take Path %d (lowest total cost: GHS %.2f)\n", bestIdx + 1, bestCost);
        }
    }

 

    void run() {
        cout << "\nGhana Road Transport Network (2026)\n";
        cout << "Loaded: " << N << " towns, " << E << " edges\n";

        while (true) {
            cout << "\n--- Menu: ---\n";
            cout << " 1. Print adjacency list\n";
            cout << " 2. Show neighbours\n";
            cout << " 3. Shortest path (distance)\n";
            cout << " 4. Shortest path (time)\n";
            cout << " 5. Top 3 shortest-distance paths\n";
            cout << " 6. Cost analysis + recommendation\n";
            cout << " 7. Full query (top 3 shortest-distance paths + time + costs + recommendation)\n";
            cout << " 8. Add road\n";
            cout << " 9. Remove road\n";
            cout << "10. Update road\n";
            cout << " 0. Quit\n";
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
                    cout << "\n  " << src << " ----> " << dst << "\n";
                    printf("  cost=%d %s\n", r.cost, w ? "min" : "km");
                    printf("  total dist=%d km, total time=%d min\n", d, t);
                    cout << "  route: " << join(r.path, " -> ") << "\n";
                }
            } else if (choice == 5) {
                cout << "Source: ";      getline(cin, src);
                cout << "Destination: "; getline(cin, dst);
                auto paths = yenKSP(src, dst, 3, 0);
                if (paths.empty())
                    cout << "No paths found.\n";
                else
                    printKSP(paths, src, dst);
            } else if (choice == 6) {
                cout << "Source: ";      getline(cin, src);
                cout << "Destination: "; getline(cin, dst);
                printCostAnalysis(src, dst);
            } else if (choice == 7) {
                cout << "Source: ";      getline(cin, src);
                cout << "Destination: "; getline(cin, dst);
                fullQuery(src, dst);
            } else if (choice == 8) {
                int d, t;
                cout << "Source: ";      getline(cin, src);
                cout << "Destination: "; getline(cin, dst);
                cout << "Distance (km): "; cin >> d; cin.ignore(1000, '\n');
                cout << "Time (min): ";    cin >> t; cin.ignore(1000, '\n');
                addEdge(src, dst, d, t);
                cout << "Road added: " << src << " <-> " << dst << "\n";
            } else if (choice == 9) {
                cout << "Source: ";      getline(cin, src);
                cout << "Destination: "; getline(cin, dst);
                if (removeEdge(src, dst))
                    cout << "Road removed: " << src << " <-> " << dst << "\n";
                else
                    cout << "Road not found.\n";
            } else if (choice == 10) {
                int d, t;
                cout << "Source: ";      getline(cin, src);
                cout << "Destination: "; getline(cin, dst);
                cout << "New distance (km): "; cin >> d; cin.ignore(1000, '\n');
                cout << "New time (min): ";    cin >> t; cin.ignore(1000, '\n');
                if (updateEdge(src, dst, d, t))
                    cout << "Road updated: " << src << " <-> " << dst << "\n";
                else
                    cout << "Road not found.\n";
            }  else {
                cout << "invalid choice\n";
            }
        }
    }
};
