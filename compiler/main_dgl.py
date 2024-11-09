import torch

from models.softmax_gat import GAT_TEST
from models.gat_dgl import GAT_DGL
from models.gcn_dgl import GCN_DGL
from models.gin_dgl import GIN_DGL
from models.sage import SAGE
from build_graph import build_graph, staged_code
from data_prepare.data_dgl import dgl_graph


def main():
    g = dgl_graph
    x = torch.nn.Parameter(torch.ones_like(g.ndata['feat']), requires_grad=True)

    use_gpu = False
    if use_gpu is True:
        x = x.cuda()
        device = x.device
        g = g.to(device)

    # model = GAT_TEST(g)
    # model = GAT_DGL(g)
    # model = GCN_DGL(g)
    # model = SAGE(g)
    model = GIN_DGL(g)
    model.eval()

    if use_gpu is True:
        model = model.cuda()

    y = model(x)
    # trace, out = torch.jit._get_trace_graph(model, x)
    # torch_graph = torch.onnx._optimize_trace(trace, torch.onnx.OperatorExportTypes.ONNX)

    # cpl = Decoupler()
    # cpl.extract(y, 'GAT-DGL')
    hl_graph = build_graph(model, x)
    staged_code(hl_graph, "gat")
    hl_graph.save("result/test_gat.pdf")


if __name__ == '__main__':
    main()
