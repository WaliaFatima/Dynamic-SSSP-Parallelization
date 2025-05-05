/*----------------------------------------------------------------------------------------
PDC Project Phase 2 Implementation - SSSP Research Paper (OpenMP)

Member 1: Mustafa Irfan (i210626)
Member 2: Walia Fatima (i210838)
Member 3: Hassaan Qadir (i210883)
Section: G
-----------------------------------------------------------------------------------------*/

#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <sstream>
#include <omp.h>
#include <limits>
#include <algorithm>

using namespace std;

using pii = pair<int, int>;
const int INF = numeric_limits<int>::max();  // Representation of infinity
using Graph = vector<vector<pii>>;           // Adjacency list: node -> list of (neighbor, weight)

vector<int> dist, parent;
vector<bool> affected;

// Load graph from file, store edges, and construct undirected adjacency list
Graph loadWeightedGraph(const string& filename, int& numVertices, vector<pii>& edges) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(1);
    }

    int u, v, max_node = 0;
    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;  // Skip comments
        istringstream iss(line);
        if (iss >> u >> v) {
            edges.emplace_back(u, v);  // Store edge
            max_node = max(max_node, max(u, v));  // Track largest node index
        }
    }

    numVertices = max_node + 1;
    Graph G(numVertices);
    for (auto& e : edges) {
        // Add undirected edges with weight 1
        G[e.first].emplace_back(e.second, 1);
        G[e.second].emplace_back(e.first, 1);
    }

    return G;
}

// Standard Dijkstra from a single source
void initialDijkstra(const Graph& G, int source) {
    int n = G.size();
    dist.assign(n, INF);       // Initialize distances to INF
    parent.assign(n, -1);      // No parent initially
    dist[source] = 0;

    priority_queue<pii, vector<pii>, greater<>> pq;
    pq.emplace(0, source);     // Start from source

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;

        for (auto [v, w] : G[u]) {
            if (dist[v] > dist[u] + w) {
                dist[v] = dist[u] + w;
                parent[v] = u;
                pq.emplace(dist[v], v);
            }
        }
    }
}

// Parallel dynamic update to Dijkstra using OpenMP
void updateDijkstra(Graph& G, const vector<pii>& delEdges, const vector<pii>& insEdges, int numThreads) {
    int n = G.size();
    affected.assign(n, false);  // Track which nodes are affected by changes

    // Handle deleted edges and mark affected vertices
    for (auto [u, v] : delEdges) {
        // Remove both directions
        G[u].erase(remove_if(G[u].begin(), G[u].end(), [v](pii e) { return e.first == v; }), G[u].end());
        G[v].erase(remove_if(G[v].begin(), G[v].end(), [u](pii e) { return e.first == u; }), G[v].end());

        // If edge was part of the SSSP tree, mark as affected
        if ((parent[v] == u && dist[v] != INF) || (parent[u] == v && dist[u] != INF)) {
            int x = dist[u] > dist[v] ? u : v;
            dist[x] = INF;
            parent[x] = -1;
            affected[x] = true;
        }
    }

    // Handle inserted edges and update affected nodes
    for (auto [u, v] : insEdges) {
        G[u].emplace_back(v, 1);
        G[v].emplace_back(u, 1);

        if (dist[u] + 1 < dist[v]) {
            dist[v] = dist[u] + 1;
            parent[v] = u;
            affected[v] = true;
        }
        if (dist[v] + 1 < dist[u]) {
            dist[u] = dist[v] + 1;
            parent[u] = v;
            affected[u] = true;
        }
    }

    omp_set_num_threads(numThreads);  // Set OpenMP thread count
    bool changed = true;

    // Iteratively update affected vertices until convergence
    while (changed) {
        changed = false;

        #pragma omp parallel for schedule(dynamic)
        for (int u = 0; u < n; ++u) {
            if (!affected[u]) continue;
            affected[u] = false;

            for (auto [v, w] : G[u]) {
                if (dist[u] > dist[v] + w) {
                    #pragma omp critical
                    {
                        if (dist[u] > dist[v] + w) {
                            dist[u] = dist[v] + w;
                            parent[u] = v;
                            affected[u] = true;
                            changed = true;
                        }
                    }
                }
            }
        }
    }
}

int main() {
    int numVertices;
    vector<pii> edgeList;

    // Load the graph from file
    Graph G = loadWeightedGraph("roadNet-CA.txt", numVertices, edgeList);
    int totalEdges = edgeList.size();

    // Print graph stats
    cout << "--------------------------------------------------------\n";
    cout << " Graph Info:\n";
    cout << "   Total Nodes   : " << numVertices << "\n";
    cout << "   Total Edges   : " << totalEdges << "\n";
    cout << "--------------------------------------------------------\n";

    // Run initial Dijkstra from source = 0
    cout << "\n[Initial Dijkstra from node 0]" << endl;
    double start_init = omp_get_wtime();
    initialDijkstra(G, 0);
    double end_init = omp_get_wtime();
    cout << "   Time Taken    : " << (end_init - start_init) << " seconds\n";
    cout << "   dist[10]      : " << dist[10] << "\n";

    // Prepare edge updates
    vector<pii> deletions(edgeList.begin(), edgeList.begin() + 500);
    vector<pii> insertions = {
        {0, 10}, {50, 300}, {1000, 1050}, {2000, 2500}, {12345, 6789}
    };

    cout << "\n[Simulating dynamic update...]" << endl;
    cout << "   Edge deletions : " << deletions.size() << endl;
    cout << "   Edge insertions: " << insertions.size() << "\n";

    // Log results to CSV
    ofstream log("dijkstra_performance.csv");
    log << "Threads,UpdateTime,RecomputeTime,Speedup,UpdatedNodes,UnreachableNodes\n";

    // Test with different OpenMP thread counts
    vector<int> thread_counts = {1, 2, 4, 8};

    for (int threads : thread_counts) {
        cout << "\n[Parallel Update with " << threads << " thread(s)]" << endl;

        // Restore graph and distance info
        Graph G_updated = G;
        vector<int> dist_backup = dist;
        vector<int> parent_backup = parent;

        // Run dynamic update
        double start = omp_get_wtime();
        dist = dist_backup;
        parent = parent_backup;
        updateDijkstra(G_updated, deletions, insertions, threads);
        double end = omp_get_wtime();
        double updateTime = end - start;

        // Analyze updated result
        int updated_count = 0, unreachable_count = 0;
        for (int i = 0; i < dist.size(); ++i) {
            if (dist[i] == INF) ++unreachable_count;
            else if (parent[i] != -1) ++updated_count;
        }

        // Compare with full recomputation for fairness
        Graph G_fresh = G;
        double recompute_start = omp_get_wtime();
        initialDijkstra(G_fresh, 0);
        double recompute_end = omp_get_wtime();
        double recomputeTime = recompute_end - recompute_start;

        double speedup = recomputeTime / updateTime;

        // Print and log performance data
        cout << "   Time Taken     : " << updateTime << " seconds" << endl;
        cout << "   Recompute Time : " << recomputeTime << " seconds" << endl;
        cout << "   Speedup        : " << speedup << "x" << endl;
        cout << "   dist[10]       : " << (dist[10] == INF ? -1 : dist[10]) << endl;
        cout << "   Nodes updated  : " << updated_count << endl;
        cout << "   Unreachable    : " << unreachable_count << endl;

        log << threads << "," << updateTime << "," << recomputeTime << "," << speedup << ","
            << updated_count << "," << unreachable_count << "\n";
    }

    log.close();

    cout << "\n--------------------------------------------------------\n";
    cout << " Dynamic Dijkstra with OpenMP completed.\n";
    cout << " Results saved to dijkstra_performance.csv\n";
    cout << "--------------------------------------------------------\n";

    return 0;
}
