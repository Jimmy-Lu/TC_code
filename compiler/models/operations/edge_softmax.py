import torch.nn
from dgl import function as fn
import torch as th


class edge_softmax(torch.nn.Module):
    def __init__(self):
        super(edge_softmax, self).__init__()

    def forward(self, graph, e):
        graph.edata['e_in'] = e
        graph.update_all(fn.copy_e(e='e_in', out='m'),
                         fn.max(msg='m', out='h1'))
        graph.apply_edges(fn.e_sub_v('e_in', 'h1', 't'))
        t = graph.edata.pop('t')
        graph.edata['exp'] = th.exp(t)
        graph.update_all(fn.copy_e(e='exp', out='l'),
                         fn.sum(msg='l', out='h2'))
        graph.apply_edges(fn.e_div_v('exp', 'h2', 'h3'))
        out = graph.edata.pop('h3')
        return out
