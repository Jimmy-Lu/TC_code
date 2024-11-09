from abc import ABC

import torch.nn as nn
import torch.nn.functional as F

from dgl.nn.pytorch import GraphConv
from data_prepare.data_dgl import num_features, num_classes


class GCN_DGL(nn.Module):
    def __init__(self, g,
                 num_layers=1,
                 in_dim=num_features,
                 num_hidden=8,
                 num_classes=num_classes,
                 activation=F.elu
                 ):
        super(GCN_DGL, self).__init__()
        self.g = g
        self.num_layers = num_layers
        self.gcn_layers = nn.ModuleList()
        self.activation = activation
        self.gcn_layers.append(
            GraphConv(in_dim, num_hidden, norm='both', weight=True, bias=True, activation=self.activation,
                      allow_zero_in_degree=True))
        # hidder layers
        for l in range(1, num_layers):
            self.gcn_layers.append(
                GraphConv(num_hidden, num_hidden, norm='both', weight=True, bias=True, activation=self.activation,
                          allow_zero_in_degree=True))
        self.gcn_layers.append(
            GraphConv(num_hidden, num_classes, norm='both', weight=True, bias=True, activation=None,
                      allow_zero_in_degree=True))

    def forward(self, h):
        for l in range(self.num_layers):
            h = self.gcn_layers[l](self.g, h).flatten(1)
        # output projection
        logits = self.gcn_layers[-1](self.g, h).mean(1)
        return logits