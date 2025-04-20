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

SNAP Datasets

SuiteSparse Matrix Collection

👨‍👩‍👧‍👦 Team Members
[Hassaan Qadir] — CPU and OpenMP Implementation

[Mustafa Irfan] — CUDA and GPU Acceleration

[Walia Fatima] — MPI + METIS Integration and Testing

🎓 Instructor
[Mr.Adil-ur-Rehman]

📃 License
This project is submitted for academic purposes under the MIT License.

📬 Contact
For any queries or issues, please contact: [your-email@example.com]

