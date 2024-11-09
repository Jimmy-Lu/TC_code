import dgl.function as fn
import torch
import functools
from pyhygcn.configs import strings
from pyhygcn import configs as di

name_dataset = strings.dataset.cora
size_feature_element = 2

graph_dgl = di.get_dateset(name_dataset)
print(graph_dgl, type(graph_dgl))

h_1 = torch.ones((int(graph_dgl.number_of_nodes() / 2), size_feature_element))
h_2 = torch.zeros((int(graph_dgl.number_of_nodes() / 2), size_feature_element))
h = torch.cat((h_1, h_2), 0)


def r_gcn(graph, feat):
    weight_0 = torch.ones(size_feature_element, size_feature_element)
    etypes = [i % 3 for i in range(graph.number_of_edges())]
    etypes = torch.tensor(etypes)
    weight = torch.cat((weight_0 * 0.01, weight_0 * 0.02, weight_0 * 0.03), 0).view(-1, size_feature_element,
                                                                                    size_feature_element)
    
    def message_func(edges, etypes, weight):
        m = edges.src['h']
        weight = weight.index_select(0, etypes)
        m = m.view(-1, 1, size_feature_element)
        msg = torch.bmm(m, weight).squeeze()
        return {'msg': msg}
    
    graph.ndata['h'] = feat
    
    graph.update_all(functools.partial(message_func, etypes=etypes, weight=weight),
                     fn.sum(msg='msg', out='h'))
    rst = graph.dstdata['h']
    bias = torch.nn.parameter.Parameter(torch.ones(size_feature_element)) * 0.1
    rst = rst + bias
    rst = torch.relu(rst)
    
    print(rst[:12])


if __name__ == '__main__':
    r_gcn(graph_dgl, h)
