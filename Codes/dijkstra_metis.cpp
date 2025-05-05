/*----------------------------------------------------------------------------------------
PDC Project Phase 2 Implementation - SSSP Research Paper (METIS Implementation)

Member 1: Mustafa Irfan (i210626)
Member 2: Walia Fatima (i210838)
Member 3: Hassaan Qadir (i210883)
Section: G
-----------------------------------------------------------------------------------------*/
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <queue>
#include <limits>
#include <metis.h>
#include <iomanip>
#include <chrono>

using namespace std;

const int INF = numeric_limits<int>::max();

// Standard Dijkstra implementation using min-heap (priority queue)
void dijkstra(const vector<vector<pair<int, int>>>& graph, int src, vector<int>& dist) {
    dist.assign(graph.size(), INF);     // Initialize distances to INF
    dist[src] = 0;                      // Source node has distance 0
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> pq;
    pq.push({0, src});                  // Push source into priority queue

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;      // Skip stale entries

        for (auto [v, w] : graph[u]) {
            if (dist[v] > dist[u] + w) {
                dist[v] = dist[u] + w;  // Update shorter distance
                pq.push({dist[v], v});  // Push updated node into queue
            }
        }
    }
}

int main() {
    string filename = "roadNet-CA.txt";
    cout << "[INFO] Reading graph..." << endl;

    vector<int> edges_u, edges_v, weights;
    int max_node = 0;

    // === 1. Load Graph Data ===
    ifstream infile(filename);
    string line;
    while (getline(infile, line)) {
        if (line[0] == '#') continue;  // Skip comment lines
        istringstream iss(line);
        int u, v;
        if (iss >> u >> v) {
            int w = rand() % 100 + 1;  // Assign random weights [1–100]
            edges_u.push_back(u);
            edges_v.push_back(v);
            weights.push_back(w);
            max_node = max(max_node, max(u, v));  // Track highest node ID
        }
    }

    int n = max_node + 1;              // Total number of nodes
    int m = edges_u.size();            // Total number of edges

    // === 2. Build Adjacency List (for Dijkstra) ===
    vector<vector<pair<int, int>>> graph(n);
    for (int i = 0; i < m; ++i) {
        graph[edges_u[i]].emplace_back(edges_v[i], weights[i]);
        graph[edges_v[i]].emplace_back(edges_u[i], weights[i]);  // Undirected graph
    }

    // === 3. Convert to CSR Format for METIS ===
    vector<idx_t> xadj(n + 1, 0);   // Indexes where each node's adjacency list starts
    vector<idx_t> adjncy;           // Concatenated adjacency lists
    vector<idx_t> adjwgt;           // Corresponding weights for edges

    for (int u = 0; u < n; ++u) {
        for (auto [v, w] : graph[u]) {
            adjncy.push_back(v);    // Destination node
            adjwgt.push_back(w);    // Edge weight
        }
        xadj[u + 1] = adjncy.size();  // End of this node's adjacency
    }

    // === 4. Call METIS for Graph Partitioning ===
    cout << "[INFO] Partitioning graph using METIS..." << endl;
    idx_t num_vertices = n;
    idx_t num_parts = 4;    // Number of partitions to create
    idx_t ncon = 1;         // Number of balancing constraints
    idx_t objval;           // Stores objective value (edge cut metric)
    vector<idx_t> part(n);  // Output partition vector

    int result = METIS_PartGraphKway(&num_vertices,
                                     &ncon,
                                     xadj.data(),
                                     adjncy.data(),
                                     NULL, NULL,
                                     adjwgt.data(),
                                     &num_parts,
                                     NULL, NULL,
                                     NULL,
                                     &objval,
                                     part.data());

    if (result != METIS_OK) {
        cerr << "[ERROR] METIS partitioning failed!" << endl;
        return 1;
    }

    cout << "[INFO] METIS partitioning completed." << endl;
    cout << "[INFO] Objective value: " << objval << endl;

    // === 5. Run Dijkstra on Full Graph (Baseline) ===
    cout << "[INFO] Running Dijkstra on full graph..." << endl;

    auto start = chrono::high_resolution_clock::now();
    vector<int> dist;
    dijkstra(graph, 0, dist);  // Source node is 0
    auto end = chrono::high_resolution_clock::now();

    // === 6. Evaluate Results ===
    int reachable = 0;
    for (int d : dist)
        if (d != INF) reachable++;

    chrono::duration<double> elapsed = end - start;

    cout << "\n========= Dijkstra (METIS Baseline) =========\n";
    cout << "Reachable Nodes : " << reachable << " / " << n << endl;
    cout << "Execution Time  : " << elapsed.count() << " seconds" << endl;
    cout << "Speed           : " << int(reachable / elapsed.count()) << " nodes/second\n";
    cout << "=============================================\n";

    // === 7. Display Sample Distances ===
    cout << "\nSample shortest distances from node 0:\n";
    for (int i = 0; i < 10; ++i) {
        cout << "  Node " << setw(8) << i << " : " << dist[i] << endl;
    }

    return 0;
}
