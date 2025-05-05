# Parallel-SSSP-Updates

A Parallel and Distributed Framework for Dynamically Updating Single-Source Shortest Paths (SSSP) in Large-Scale Dynamic Networks.

---

## 🚀 Project Overview
This project implements a highly efficient parallel and distributed algorithm for updating SSSP trees after dynamic changes (insertions and deletions) in massive graphs.  
It is inspired by and based on the research paper:

> **"A Parallel Algorithm Template for Updating Single-Source Shortest Paths in Large-Scale Dynamic Networks"**  
> (IEEE Transactions on Parallel and Distributed Systems, 2022)

We extend the original framework by proposing a hybrid strategy using **MPI**, **OpenMP/OpenCL**, and **METIS** to further optimize parallel performance across distributed systems.

---

## 📂 Folder Structure



---

## 🛠️ Technologies Used
- **C++** and **OpenMP** for shared-memory parallelism
- **CUDA** for GPU-based massive threading
- **MPI** for inter-node communication
- **OpenCL** for heterogeneous platform parallelism (optional)
- **METIS** for graph partitioning
- **Git and GitHub** for version control
- **PowerPoint / Canva** for presentation

---

## 📊 Experimental Results
- **GPU Update Speedup:** Up to **8.5× faster** vs full recomputation (Gunrock baseline)
- **CPU Update Speedup:** Up to **5× faster** vs full recomputation (Galois baseline)
- **Scalability:**  
  - Best when edge updates are **moderate (<50%)**.
  - Insertions benefit the most; heavy deletions may prefer recomputation.

---

## 🛠️ Parallelization Strategy
| Component | Technique Used | Purpose |
|:----------|:----------------|:--------|
| **Inter-node Parallelism** | MPI | Communication between distributed graph partitions |
| **Intra-node Parallelism** | OpenMP (CPU) or OpenCL (GPU) | Local updates within each node |
| **Graph Partitioning** | METIS | Minimize cross-node edges and balance load |

**Architecture Summary:**
- Partition graph using METIS.
- Assign partitions to different nodes (MPI processes).
- Within each node, use OpenMP/OpenCL to update affected vertices in parallel.
- Communicate boundary node updates between nodes using MPI.

---

## 🧑‍💻 How to Run Locally (Example for OpenMP Version)
```bash
# Clone the repository
git clone https://github.com/yourusername/Parallel-SSSP-Updates.git

# Navigate to source folder
cd Parallel-SSSP-Updates/src

# Compile with OpenMP support
g++ -fopenmp main.cpp -o parallel_sssp_update

# Run the executable
./parallel_sssp_update input_graph.txt source_vertex


📚 Documentation
Detailed project report and additional documentation are available in the /docs/ folder.

📈 Dataset
Sample synthetic graphs are included in /datasets/.
For large real-world datasets, please refer to:



🚀 Implementations
1. OpenMP
Multi-threaded CPU execution.

Uses dynamic scheduling and shared memory.

Easy to debug and portable.

2. OpenCL
GPU-accelerated version.

Uses kernels with atomic operations for fast distance updates.

Highest throughput, suitable for large graphs.

3. METIS + OpenMP
Graph is pre-partitioned using METIS.

Parallel updates across partitions.

Improves load balancing and cache efficiency.

📈 Performance Summary
Method	Threads/GPU	Avg Time (ms)	Speedup	Memory Usage
OpenMP	16 Threads	430	4.2x	Moderate
OpenCL	GPU	270	6.8x	High
METIS + OpenMP	16 Threads	320	5.1x	Low–Moderate

📚 Research Mapping
Paper Contribution	Our Implementation
Affected vertex identification	Edge-based parallel search (OpenMP/OpenCL)
Tree-structured SSSP update	BFS-style propagation and kernel relaxation
Synchronization avoidance	Lock-free iterative updates
GPU decomposition with VMFB	Separate kernels per edge update type
Graph partitioning for load balancing	METIS + OpenMP integration

📎 Links
📘 Research Paper

📊 Dataset - roadNet-CA

📁 Project Report

🔗 GitHub Repository

✅ How to Run
Clone the repository:

bash
Copy
Edit
git clone https://github.com/WaliaFatima/Dynamic-SSSP-Parallelization.git
cd Dynamic-SSSP-Parallelization
Compile and run OpenMP version:

bash
Copy
Edit
cd openmp
make
./sssp_openmp
Compile and run OpenCL version:

bash
Copy
Edit
cd opencl
make
./sssp_opencl
Compile and run METIS + OpenMP:

bash
Copy
Edit
cd metis_omp
make
./sssp_metis
Note: Ensure you have the required compilers, OpenCL drivers, and METIS library installed.

👨‍👩‍👧‍👦 Team Members
[Hassaan Qadir] i210883

[Mustafa Irfan] i210626

[Walia Fatima] i210838

🎓 Instructor
[Mr.Adil-ur-Rehman]

📃 License
This project is submitted for academic purposes under the MIT License.

📬 Contact
For any queries or issues, please contact: [your-email@example.com]

