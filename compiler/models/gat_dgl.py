import torch.nn as nn
import torch.nn.functional as F

from dgl.nn.pytorch import GATConv
from data_prepare.data_dgl import num_features, num_classes


class GAT_DGL(nn.Module):
    def __init__(self, g,
                 num_layers=1,
                 in_dim=num_features,
                 num_hidden=8,
                 num_classes=num_classes,
                 heads=8,
                 activation=F.elu,
                 feat_drop=0.6,
                 attn_drop=0.6,
                 negative_slope=0.2,
                 residual=False):
        super(GAT_DGL, self).__init__()
        self.g = g
        self.num_layers = num_layers
        self.gat_layers = nn.ModuleList()
        self.activation = activation
        # input projection (no residual)
        heads = ([heads] * num_layers) + [1]
        self.gat_layers.append(GATConv(
            in_dim, num_hidden, heads[0],
            feat_drop, attn_drop, negative_slope, False, self.activation))
        # hidden layers
        for l in range(1, num_layers):
            # due to multi-head, the in_dim = num_hidden * num_heads
            self.gat_layers.append(GATConv(
                num_hidden * heads[l - 1], num_hidden, heads[l],
                feat_drop, attn_drop, negative_slope, residual, self.activation))
        # output projection
        self.gat_layers.append(GATConv(
            num_hidden * heads[-2], num_classes, heads[-1],
            feat_drop, attn_drop, negative_slope, residual, None))
    
    def forward(self, h):
        for l in range(self.num_layers):
            h = self.gat_layers[l](self.g, h).flatten(1)
        # output projection
        logits = self.gat_layers[-1](self.g, h).mean(1)
        return logits
