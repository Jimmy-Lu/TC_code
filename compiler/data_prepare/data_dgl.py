from dgl.data import CoraGraphDataset
data = CoraGraphDataset(verbose=False)
dgl_graph = data[0]
num_features = dgl_graph.ndata['feat'].shape[1]
num_classes = data.num_classes
