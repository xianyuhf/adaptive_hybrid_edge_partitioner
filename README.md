Adaptive Hybrid Edge Partitioner
=============================================
The code corresponds to article "AHEP: An Adaptive Hybrid Edge Partitioner for Large-Scale Graphs in Heterogeneous Environment".

An Adaptive Hybrid Edge Partitioner (AHEP) is a novel approach designed to address the challenges of edge partitioning in heterogeneous environments by combining in-memory and streaming partitioning while dynamically maintaining the upper bounds of the partitions. Many traditional partitioning methods fail to account for the heterogeneity across machines (including processor performance, network bandwidth, and memory size), leading to a significant increase in the computing and communication cost of distributed graph processing algorithm. AHEP overcomes these limitations through an adaptive hybrid approach. It first employs Heterogeneous Cohesive Neighbor Expansion (HCNE) as an effective heterogeneous in-memory partitioning algorithm for edges involving low-degree vertices. For the remaining edges, AHEP employs Heterogeneous Cohesive Streaming Greedy (HCSG), a heterogeneous streaming algorithm to minimize the cost of distributed graph processing algorithm. Experiments on real-world graphs demonstrate that AHEP outperforms six state-of-the-art partitioning methods. These results validate that AHEP can effectively reduce the costs of distributed graph processing in resource-constrained environments.

### Experimental environment 
---------------------

We conduct experiments on a heterogeneous cluster with two types of node normal nodes and bottleneck nodes. Each normal node has a 13th Gen Intel® Core™ i9-13900K CPU @ 5.6 GHz, 64 GB of RAM, a 1000 Mbit network adapter and a 10 TB disk RAID. Each bottleneck node has a Intel(R) Core(TM) i7-7700HQ CPU @ 2.8 GHz, 16 GB of RAM, a 500 Mbit network adapter and a 1 TB disk RAID. The nodes are connected via Gigabit Ethernet. The hardware configuration is manually changed to simulate the hardware heterogeneity.

### Datasets
---------------------

Computer, Physics, Flickr and Reddit are obtained from the Deep Graph Library ([DGL](https://www.dgl.ai/)). The ogbn-arxiv and ogbn-products datasets are obtained from the Open Graph Benchmark ([OGB](https://ogb.stanford.edu/)).

### Compilation and Usage
---------------------

We tested our program on Ubuntu 18.04, and it requires the following
packages: `cmake`, `glog`, `gflags`, `boost`:
```
sudo apt-get install libgoogle-glog-dev libgflags-dev libboost-all-dev
```

Compilation:
```
git clone <this repository>
cd <repository_name>
mkdir release && cd release
cmake ..
make -j8
```

Usage:
```
$ ./main --help
main: -filename <path to the input graph> [-method hep] [-hdf <threshold / \tau>] [-p <number of partitions>] 

```

### Example.
Partition the Orkut graph into 8 parts using HEP with \tau = 10.0:

```
$ ./main -p 8 -method ahep -hdf 10.0 -filename /path/to/com-orkut.ungraph.txt


$executePath/$executeName -p $p -method $method -hdf 100 -hybrid_NE 1 -topo 4 -filename ${inputfile[0]} -output ${outputfile[0]} -log_dirs ${log_dirs[0]} -log_info $log_info
```

### Acknowledgements.
---------------------
The implementation incorporates elements from the NE reference implementation by Qin Liu (https://github.com/ansrlab/edgepart) and the HEP reference implementation by Ruben Mayer (https://github.com/mayerrn/hybrid_edge_partitioner).