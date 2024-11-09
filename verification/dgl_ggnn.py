import dgl.function as fn
import torch
import functools
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


def ggnn(graph, feat):
    weight_0 = torch.ones(size_feature_element, size_feature_element)
    etypes = [i % 3 for i in range(graph.number_of_edges())]
    etypes = torch.tensor(etypes)
    weight = torch.cat((weight_0 * 0.01, weight_0 * 0.02, weight_0 * 0.03), 0).view(-1, size_feature_element,
                                                                                    size_feature_element)
    
    def gru_simple(input, hidden):
        weight = torch.ones(size_feature_element, size_feature_element) * 0.1
        bias = torch.ones(1, size_feature_element) * 0.1
        r = torch.matmul(input, weight) + bias + torch.matmul(hidden, weight) + bias
        r = torch.sigmoid(r)
        z = torch.matmul(input, weight) + bias + torch.matmul(hidden, weight) + bias
        z = torch.sigmoid(z)
        n = torch.matmul(input, weight) + bias + torch.multiply(r, torch.matmul(hidden, weight) + bias)
        n = torch.tanh(n)
        h_new = torch.multiply(z, n) + torch.multiply(z, hidden)
        return h_new
    
    def message_func(edges, etypes, weight):
        m = edges.src['h']
        weight = weight.index_select(0, etypes)
        m = m.view(-1, 1, size_feature_element)
        msg = torch.bmm(m, weight).squeeze()
        return {'msg': msg}
    
    graph.ndata['h'] = feat
    graph.update_all(functools.partial(message_func, etypes=etypes, weight=weight),
                     fn.sum(msg='msg', out='a'))
    a = graph.ndata.pop('a')
    rst = gru_simple(a, feat)
    
    print(rst[:12])


if __name__ == '__main__':
    ggnn(graph_dgl, h)
