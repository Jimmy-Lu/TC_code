import dgl.function as fn
import torch

from pyhygcn.configs import strings
from pyhygcn import configs as di

name_dataset = strings.dataset.cora
size_feature_element = 2

graph_dgl = di.get_dateset(name_dataset)
print(graph_dgl, type(graph_dgl))
h_1 = torch.ones((int(graph_dgl.number_of_nodes() / 2), size_feature_element))
h_2 = torch.zeros((int(graph_dgl.number_of_nodes() / 2), size_feature_element))
# h = torch.ones(
#     (graph_dgl.number_of_nodes(), size_feature_element))
h = torch.cat((h_1, h_2), 0)


def gat(graph, feat):
    we = torch.ones(
        (size_feature_element, size_feature_element)) * 0.1
    wv = torch.ones(
        (size_feature_element, 1)) * 0.1
    
    def apply_edge(edges):
        m = torch.matmul(edges.src['h'], wv)
        n = torch.matmul(edges.dst['h'], wv)
        e = m + n
        e = torch.exp(torch.relu(e))
        return {'m': e}
    
    feat = torch.matmul(feat, we)
    graph.ndata['h'] = feat
    graph.apply_edges(apply_edge)
    graph.update_all(fn.copy_e('m', 'm'), fn.sum(msg='m', out='l'))
    graph.update_all(fn.u_mul_e('h', 'm', 'e'), fn.sum(msg='e', out='h'))
    h0 = graph.ndata['l']
    h1 = graph.ndata['h']
    h2 = torch.divide(h1, h0)
    for i in range(12):
        print(h2[i][0])


if __name__ == '__main__':
    gat(graph_dgl, h)
