import os.path as osp

import torch_geometric.transforms as T
from torch_geometric.datasets import Planetoid

dataset_pyg = 'Cora'
path = osp.join(osp.dirname(osp.realpath(__file__)), '../..', 'data', dataset_pyg)
dataset_pyg = Planetoid(path, dataset_pyg, transform=T.NormalizeFeatures())
num_features = dataset_pyg.num_features
num_classes = dataset_pyg.num_classes
data_pyg = dataset_pyg[0]
num_vertices = data_pyg.x.shape[0]
num_edges = data_pyg.edge_index.shape[1]
