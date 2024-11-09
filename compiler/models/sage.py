import torch.nn as nn
import torch.nn.functional as F

from dgl.nn.pytorch import SAGEConv
from data_prepare.data_dgl import num_features, num_classes


class SAGE(nn.Module):
    def __init__(self, g,
                 num_layers=1,
                 in_dim=num_features,
                 num_hidden=8,
                 num_classes=num_classes,
                 feat_drop=0.6,
                 activation=F.elu,
                 aggregator_type='pool'
                 ):
        super(SAGE, self).__init__()
        self.g = g
        self.num_layers = num_layers
        self.sage_layers = nn.ModuleList()
        self.aggregator_type = aggregator_type
        self.activation = activation
        self.sage_layers.append(
            SAGEConv(in_dim, num_hidden, norm=None, feat_drop=feat_drop, bias=True, activation=self.activation,
                     aggregator_type=self.aggregator_type))
        # hidder layers
        for l in range(1, num_layers):
            self.gcn_layers.append(
                SAGEConv(num_hidden, num_hidden, norm=None, feat_drop=feat_drop, bias=True, activation=self.activation,
                         aggregator_type=self.aggregator_type))
        self.sage_layers.append(
            SAGEConv(num_hidden, num_classes, norm=None, feat_drop=feat_drop, bias=True, activation=None,
                     aggregator_type=self.aggregator_type))

    def forward(self, h):
        for l in range(self.num_layers):
            h = self.sage_layers[l](self.g, h).flatten(1)
        # output projection
        logits = self.sage_layers[-1](self.g, h).mean(1)
        return logits
