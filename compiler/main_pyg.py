import torch

from models.gat_pyg import GAT_PYG
from data_prepare.data_pyg import data_pyg, num_vertices, num_edges
from build_graph import build_graph


def main():
    x = torch.nn.Parameter(torch.ones_like(data_pyg.x))
    
    model = GAT_PYG()
    model.eval()
    
    use_gpu = False
    if use_gpu is True:
        model, x = model.cuda(), x.cuda()
        data_pyg.edge_index = data_pyg.edge_index.cuda()
    
    y = model(x)
    # trace, out = torch.jit._get_trace_graph(model, x)
    # torch_graph = torch.onnx._optimize_trace(trace, torch.onnx.OperatorExportTypes.ONNX)
    # print(torch_graph)
    # cpl = Decoupler()
    # cpl.extract(y, 'GAT-PyG')
    # cpl.decouple(num_vertices, num_edges, 'GAT-PyG')
    hl_graph = build_graph(model, x)
    hl_graph.save("test_gat.pdf")

if __name__ == '__main__':
    main()
