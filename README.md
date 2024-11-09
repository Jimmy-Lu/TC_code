# General GNN Accelerator.

## Installation Guide

### Anaconda Source Changing

```bash
# use Python3
wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh -O miniconda.sh
bash miniconda.sh -b -p $HOME/usr/miniconda
[edit ~/.bashrc]
export PATH=$HOME/usr/miniconda/bin:$PATH

# use SJTU conda mirror
conda config --remove-key channels
conda config --add channels https://mirror.sjtu.edu.cn/anaconda/pkgs/free
conda config --add channels https://mirror.sjtu.edu.cn/anaconda/pkgs/main
conda config --add channels https://mirror.sjtu.edu.cn/anaconda/pkgs/mro
conda config --add channels https://mirror.sjtu.edu.cn/anaconda/pkgs/msys2
conda config --add channels https://mirror.sjtu.edu.cn/anaconda/cloud/pytorch/
conda config --add channels https://mirror.sjtu.edu.cn/anaconda/cloud/conda-forge/ 
# show the channel urls
conda config --set show_channel_urls yes

# create a virtual environment
conda create -n sga python=3.8
conda activate sga
# remove a virtual environment
# conda env remove -n sga
```

### Core Packages

[PyTorch](https://pytorch.org/), [PyTorch Geo (PyG)](https://github.com/rusty1s/pytorch_geometric), [Open Graph Benchmark (OGB)](https://ogb.stanford.edu/)

```bash
conda install rdflib pyg -c pyg # pyg
pip install pytorch torchvision torchaudio # pytorch
pip install --upgrade ogb # ogb
```

### Misc.

```bash
git config --global core.autocrlf input # git push setting

conda install graphviz python-graphviz # graphviz

# Other common packages (optional)
conda install xlutils matplotlib pandas requests networkx scipy numba
```

## Run Simulation

1. We provide the scripts `build.sh` and `clean.sh` for convenience.

2. Then go to the folder of `cppmain/evaluation` and run `execution` or `partition` with proper arguments.

3. You can run `make` in `cppmain/evaluation`  to rebuild.

See examples in `cppmain/scripts`.


### Multi-Tasking

We recommend [`tmux`](https://zhuanlan.zhihu.com/p/98384704) to launch and manage processes in background. We do not use
the `nohup` command anymore for the difficulty in scaling up the experiments.

```bash
chmod +x *.sh
```

### Memory Leakage Analysis

We recommend [`valgrind`](http://senlinzhan.github.io/2017/12/31/valgrind/) to check the memory usage of the simulation.
Before starting up, you may put the following line into the `CMakeLists.txt` and rebuild to expose the information to valgrind.

```cmake
add_compile_options(-g)
```



## Graph Dataset (Directed)

| Name          | Vertex#   | Edge#      | Feature# | Class# |
| ------------- | --------- | ---------- | -------- | ------ |
| Cora (CR)     | 2,708     | 10,556     | 1,433    | 7      |
| CiteSeer (CS) | 3,327     | 9,104      | 3,703    | 6      |
| PubMed (PM)   | 19,717    | 88,648     | 500      | 3      |
| Flickr (FL)   | 89,250    | 899,756    | 500      | 7      |
| Arxiv (AX)    | 169,343   | 1,166,243  | 128      | 40     |
| Reddit (RD)   | 232,965   | 23,213,838 | 602      | 41     |
| Yelp (YP)     | 716,847   | 13,954,819 | 300      | -    |
| Product (PD)  | 2,449,029 | 61,859,140 | 100      | 47     |


### Legacy

| Name                  | Vertex#    | Edge#       |
| --------------------- | ---------- | ----------- |
| cit-Patents (CP)      | 3,774,768  | 16,518,948  |
| soc-LiveJournal1 (SL) | 4,847,571  | 43,369,619  |
| hollywood (HW)        | 1,139,905  | 57,515,616  |
| europe_osm (EO)       | 50,912,018 | 54,054,660  |
| soc-twitter-2010 (ST) | 21,297,772 | 265,025,809 |



## Change Log

### What is in the version 9:

- Tile coalesce
- Thread <= Stream

### What is in the version 8:

- Code refactored in hardware perspective.
  - Class == Module
- Online tiling via TilingUnit (TU).
  - Supporting CSR, CSC formats
- Compiler from DGL to IR.
- ISCA 2022 submission.

### What is in the version 7:

- Multiple graph reordering/clustering algorithms.
  - basic reordering (Sort, Random).
  - hub-based sorting (HubSort, HubCluster).
  - degree-based grouping (DBG).
  - minhash grouping (TVC).

### What is in the version 6:

- New controller for the multi-stream execution.
- MICRO 2021 submission.

### What is in the version 5:

- Instruction driven Architecture.
- Tile-based multi-stream execution.
- Degree-sorting-based vertex reordering.
- High-performance C++ simulator.

### What is in the version 4:

- Hint-guided execution with hilbert-curve tiling.
- Decoupling the hardware execution and software partition.
- [Ramulator](https://github.com/CMU-SAFARI/ramulator) DRAM simulator integration.

### What is in the version 3:

- Unified SA for both ApplyEdge & ApplyVertex
- Energy consumption in hardware execution.
- HPCA 2021 submission.

### What is in the version 2:

- An extra SA for SAGA-NN execution.
- Hilbert space-filing curve for graph partition.

### What is in the version 1:

- [HyGCN](https://ieeexplore.ieee.org/abstract/document/9065592/) performance simulation reproduction.
  - with pipelined SIMD and SA design
