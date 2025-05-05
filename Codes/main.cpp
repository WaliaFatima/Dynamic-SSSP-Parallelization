/*----------------------------------------------------------------------------------------
PDC Project Phase 2 Implementation - SSSP Research Paper (OpenCL - main.cpp)

Member 1: Mustafa Irfan (i210626)
Member 2: Walia Fatima (i210838)
Member 3: Hassaan Qadir (i210883)
Section: G
-----------------------------------------------------------------------------------------*/
#include <CL/cl.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include <climits>
#include <iomanip>

using namespace std;

#define MAX_SOURCE_SIZE (0x100000)  // Max size for kernel source buffer

// Helper function to check for OpenCL errors
void checkError(cl_int err, const string& msg) {
    if (err != CL_SUCCESS) {
        cerr << "ERROR: " << msg << " (" << err << ")" << endl;
        exit(1);
    }
}

int main() {
    // === 1. Load graph data from file ===
    string filename = "roadNet-CA.txt";
    vector<int> edges_u, edges_v, weights;
    int max_node = 0;

    cout << "\n[INFO] Loading graph from: " << filename << "...\n";
    ifstream infile(filename);
    string line;
    while (getline(infile, line)) {
        if (line[0] == '#') continue;  // Skip comments
        istringstream iss(line);
        int u, v;
        if (iss >> u >> v) {
            int w = rand() % 100 + 1;  // Assign random weight [1,100]
            edges_u.push_back(u);
            edges_v.push_back(v);
            weights.push_back(w);
            max_node = max(max_node, max(u, v));  // Track max node ID
        }
    }

    int n = max_node + 1;           // Total nodes
    int edge_count = edges_u.size();

    cout << "[INFO] Nodes: " << n << " | Edges: " << edge_count << endl;

    // Initialize distance array (SSSP source = 0)
    vector<int> dist(n, INT_MAX);
    dist[0] = 0;

    // === 2. Load OpenCL kernel source ===
    ifstream kernelFile("dijkstra.cl");
    string sourceStr((istreambuf_iterator<char>(kernelFile)),
                     istreambuf_iterator<char>());
    const char* source = sourceStr.c_str();
    size_t sourceSize = sourceStr.size();

    // === 3. OpenCL setup: platform, device, context, queue ===
    cl_int err;
    cl_platform_id platform;
    cl_device_id device;
    err = clGetPlatformIDs(1, &platform, NULL);
    checkError(err, "Getting platform");

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    checkError(err, "Getting device");

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    checkError(err, "Creating context");

    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);
    checkError(err, "Creating command queue");

    // === 4. Compile kernel program ===
    cl_program program = clCreateProgramWithSource(context, 1, &source, &sourceSize, &err);
    checkError(err, "Creating program");

    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        // Show compilation errors if any
        size_t len;
        char buffer[2048];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        cerr << "Build error:\n" << buffer << endl;
        exit(1);
    }

    // === 5. Create kernel object ===
    cl_kernel kernel = clCreateKernel(program, "dijkstra", &err);
    checkError(err, "Creating kernel");

    // === 6. Create memory buffers on GPU ===
    cl_mem u_buf = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                  sizeof(int) * edge_count, edges_u.data(), &err);
    cl_mem v_buf = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                  sizeof(int) * edge_count, edges_v.data(), &err);
    cl_mem w_buf = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                  sizeof(int) * edge_count, weights.data(), &err);
    cl_mem dist_buf = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                     sizeof(int) * n, dist.data(), &err);
    cl_mem updated_buf = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                        sizeof(int), NULL, &err);  // flag to track convergence

    // === 7. Set kernel arguments ===
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &u_buf);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &v_buf);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &w_buf);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &dist_buf);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), &updated_buf);
    clSetKernelArg(kernel, 5, sizeof(int), &edge_count);

    // === 8. Run kernel in a loop until no changes ===
    cout << "[INFO] Running OpenCL kernel...\n";
    auto start = chrono::high_resolution_clock::now();
    int updated = 1;

    while (updated) {
        updated = 0;
        // Write 0 to the 'updated' buffer
        clEnqueueWriteBuffer(queue, updated_buf, CL_TRUE, 0, sizeof(int), &updated, 0, NULL, NULL);
        // Execute kernel over all edges
        clEnqueueNDRangeKernel(queue, kernel, 1, NULL, (const size_t*)&edge_count, NULL, 0, NULL, NULL);
        clFinish(queue);
        // Read back 'updated' flag to decide if another pass is needed
        clEnqueueReadBuffer(queue, updated_buf, CL_TRUE, 0, sizeof(int), &updated, 0, NULL, NULL);
    }

    auto end = chrono::high_resolution_clock::now();

    // Read final distances back to CPU
    clEnqueueReadBuffer(queue, dist_buf, CL_TRUE, 0, sizeof(int) * n, dist.data(), 0, NULL, NULL);
    chrono::duration<double> elapsed = end - start;

    // Count reachable nodes (not INF)
    int reachable = 0;
    for (int d : dist)
        if (d != INT_MAX) reachable++;

    // === 9. Output results ===
    cout << "\n========= Dijkstra OpenCL Summary =========\n";
    cout << "Reachable Nodes : " << reachable << " / " << n << endl;
    cout << fixed << setprecision(6);
    cout << "Execution Time  : " << elapsed.count() << " seconds\n";
    cout << "Speed           : " << (int)(reachable / elapsed.count()) << " nodes/second\n";
    cout << "===========================================\n";

    // === Sample few shortest paths from source ===
    cout << "\nSample shortest distances from node 0:\n";
    for (int i = 0, shown = 0; i < n && shown < 10; ++i) {
        if (dist[i] != INT_MAX) {
            cout << "  Node " << setw(7) << i << " : " << dist[i] << endl;
            shown++;
        }
    }

    // === (Optional) CSV Logging ===
    ofstream log("opencl_results.csv", ios::app);
    log << n << "," << edge_count << "," << reachable << "," << elapsed.count() << "\n";
    log.close();

    // === Cleanup OpenCL resources ===
    clReleaseMemObject(u_buf);
    clReleaseMemObject(v_buf);
    clReleaseMemObject(w_buf);
    clReleaseMemObject(dist_buf);
    clReleaseMemObject(updated_buf);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;

    
}