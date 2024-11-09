import dgl.function as fn
import torch

from pyhygcn.configs import strings

name_dataset = strings.dataset.cora
size_feature_element = 128

from pyhygcn import configs as di
graph_dgl = di.get_dateset(name_dataset)
print(graph_dgl, type(graph_dgl))

we = torch.ones(
    (size_feature_element, size_feature_element))
wn = torch.ones(
    (size_feature_element, size_feature_element))
wv = torch.ones(
    (size_feature_element, 1))
wa = torch.ones(
    (1, size_feature_element))
h = torch.ones(
    (graph_dgl.number_of_nodes(), size_feature_element))


def apply_edge(edges):
    m = torch.matmul(edges.src['h'], we)
    m = torch.matmul(m, wv)
    m = m + wa
    return {'m': m}


graph_dgl.ndata['h'] = h
graph_dgl.apply_edges(apply_edge)
graph_dgl.update_all(fn.copy_e('m', 'm'), fn.sum(msg='m', out='h'))
h1 = graph_dgl.ndata['h']
h2 = torch.matmul(h1, wn)
# for i in range(12):
print(h2[2700:])
