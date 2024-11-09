import torch

from pyhygcn import configs as di
from pyhygcn.configs import strings
from verification.dgl_gat import gat
from verification.dgl_ggnn import ggnn
from verification.dgl_r_gcn import r_gcn
from verification.dgl_sage import graphsage

name_dataset = strings.dataset.cora
size_feature_element = 2

graph_dgl = di.get_dateset(name_dataset)
print(graph_dgl, type(graph_dgl))
h_1 = torch.ones((int(graph_dgl.number_of_nodes() / 2), size_feature_element))
h_2 = torch.zeros((int(graph_dgl.number_of_nodes() / 2), size_feature_element))
# h = torch.ones(
#     (graph_dgl.number_of_nodes(), size_feature_element))
h = torch.cat((h_1, h_2), 0)

if __name__ == '__main__':
    gat(graph_dgl, h)
    ggnn(graph_dgl, h)
    r_gcn(graph_dgl, h)
    graphsage(graph_dgl, h)
