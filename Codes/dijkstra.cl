//----------------------------------------------------------------------------------------
//PDC Project Phase 2 Implementation - SSSP Research Paper (OpenCL - dijkstra.cl)
//Member 1: Mustafa Irfan (i210626)
//Member 2: Walia Fatima (i210838)
//Member 3: Hassaan Qadir (i210883)
//Section: G
//-----------------------------------------------------------------------------------------

__kernel void dijkstra(
    __global const int* edges_u,    // Source nodes of edges
    __global const int* edges_v,    // Destination nodes of edges
    __global const int* weights,    // Weights of edges
    __global int* dist,             // Global distances from source
    __global int* updated,          // Flag to track if any node was updated
    const int edge_count            // Total number of edges
) {
    int i = get_global_id(0);  // Unique thread ID
    if (i >= edge_count) return;

    int u = edges_u[i];
    int v = edges_v[i];
    int w = weights[i];
    int dist_u = dist[u];

    // Only proceed if source node has a known distance
    if (dist_u != 2147483647) { // INT_MAX
        int new_dist = dist_u + w;

        // Atomically update distance to v if new one is shorter
        int old_dist = atomic_min(&dist[v], new_dist);

        // If distance was improved, mark the graph as updated
        if (new_dist < old_dist) {
            updated[0] = 1;
        }
    }
}
