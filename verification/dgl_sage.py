import dgl.function as fn
import torch

from pyhygcn.configs import strings

name_dataset = strings.dataset.cora
size_feature_element = 2
from pyhygcn import configs as di

graph_dgl = di.get_dateset(name_dataset)
print(graph_dgl, type(graph_dgl))

h_1 = torch.ones((int(graph_dgl.number_of_nodes() / 2), size_feature_element))
h_2 = torch.zeros((int(graph_dgl.number_of_nodes() / 2), size_feature_element))
# h = torch.ones(
# (graph_dgl.number_of_nodes(), size_feature_element))
h = torch.cat((h_1, h_2), 0)


def graphsage(graph, feat):
    wn = torch.ones(
        (size_feature_element, size_feature_element)) * 0.1
    feat_src = feat_dst = feat
    h_self = feat_dst
    aggregate_fn = fn.copy_src('h', 'm')
    graph.srcdata['h'] = torch.relu(torch.matmul(feat_src, wn))
    graph.update_all(aggregate_fn, fn.max('m', 'neigh'))
    h_neigh = graph.dstdata['neigh']
    rst = torch.matmul(h_self, wn) + torch.matmul(h_neigh, wn)
    bias = torch.nn.parameter.Parameter(torch.ones(size_feature_element)) * 0.1
    rst = rst + bias
    rst = torch.relu(rst)
    for i in range(12):
        print(rst[i])


if __name__ == '__main__':
    graphsage(graph_dgl, h)