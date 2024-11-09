import time
import networkx as nx
from matplotlib import pyplot, patches


def draw_adjacency_matrix(G):
    size = 0.001
    
    num_vertex = G.number_of_nodes()
    edge_list = nx.to_edgelist(G)
    # exit()
    fig = pyplot.figure(figsize=(5, 5), dpi=200)  # in inches
    pyplot.xlim((0, num_vertex))
    pyplot.ylim((0, num_vertex))
    ax = pyplot.gca()
    ax.xaxis.set_ticks_position('top')  # 将x轴的位置设置在顶部
    ax.invert_yaxis()  # y轴反向
    for e in edge_list:
        ax.add_patch(patches.Rectangle((int(e[0]), int(e[1])),
                                       size * num_vertex,  # Width
                                       size * num_vertex,  # Height
                                       facecolor="red",
                                       edgecolor=None,
                                       linewidth="0",
                                       alpha=0.2,
                                       ))
    fig.show()


dict_path_dataset_original = {

}


def read_graph_original(path: str):
    g = nx.read_edgelist(path, create_using=nx.DiGraph)  # type: nx.DiGraph
    print(path)
    return g


def main():
    paths = [
        # 'datasets/ak2010.mtx.coo.metis_out',
        'datasets/ak2010.mtx.coo.metis_in.part.64',
        # 'datasets/directed/cit-Patents.csv.coo.metis_out',
    ]
    for p in paths:
        G = read_graph_original(p)
        print(type(G))
        print(G.number_of_nodes(), G.number_of_edges())
        draw_adjacency_matrix(G)


if __name__ == '__main__':
    t1 = time.time()
    main()
    t2 = time.time()
    print('render latency: {} s.'.format(t2 - t1))
