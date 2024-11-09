import torch.nn.functional as F
import torch.nn as nn
# from torch_geometric.nn import GATConv
from models.gat_conv_pyg import GATConv
from data_prepare.data_pyg import dataset_pyg


class GAT_PYG(nn.Module):
    def __init__(self):
        super(GAT_PYG, self).__init__()
        
        self.conv1 = GATConv(dataset_pyg.num_features, 8, heads=8, dropout=0.6)
        # self.conv1 = GATConv(dataset_pyg.num_features, 8, heads=8, dropout=0.6)
        # On the Pubmed dataset, use heads=8 in conv2.
        self.conv2 = GATConv(8 * 8, dataset_pyg.num_classes, heads=1, concat=False,
                             dropout=0.6)
    
    def forward(self, x, edge_index):
        x = F.dropout(x, p=0.6, training=self.training)
        x = F.elu(self.conv1(x, edge_index))
        x = F.dropout(x, p=0.6, training=self.training)
        x = self.conv2(x, edge_index)
        return F.log_softmax(x, dim=-1)


import torch
import torch.fx


class MyModule(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.param = torch.nn.Parameter(torch.rand(3, 4))
        self.linear = torch.nn.Linear(4, 5)
    
    def forward(self, x):
        h = self.linear(x + self.linear.weight)
        h = h.relu()
        h = torch.sum(h, dim=-1)
        ret = torch.topk(h, 3)
        
        return ret


def get_children(model: torch.nn.Module):
    # get children form model!
    children = list(model.children())
    flatt_children = []
    if children == []:
        # if model has no children; model is last child! :O
        return model
    else:
        # look for children from children... to the last child!
        for child in children:
            try:
                flatt_children.extend(get_children(child))
            except TypeError:
                flatt_children.append(get_children(child))
    return flatt_children

# visualisation = {}
#
#
# def hook_fn(m, i, o):
#     visualisation[m] = o
#
#
# def get_all_layers(net):
#     for name, layer in net._modules.items():
#         # If it is a sequential, don't register a hook on it
#         # but recursively register hook on all it's module children
#         if isinstance(layer, torch.nn.Sequential):
#             get_all_layers(layer)
#         else:
#             # it's a non sequential. Register a hook
#             layer.register_forward_hook(hook_fn)
#
