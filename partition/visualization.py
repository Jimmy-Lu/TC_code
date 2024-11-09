import time

from pyhygcn.configs import strings

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
    'CP': '../profiling/datasets/directed/cit-Patents.csv',
    'SL': '../profiling/datasets/directed/soc-LiveJournal1.csv',
    'EO': '../profiling/datasets/undirected/europe_osm.csv',
    'HW': '../profiling/datasets/undirected/hollywood.csv',
    'ST': '../profiling/datasets/undirected/soc-twitter-2010.csv',
    'AD': '../profiling/datasets/coAuthorsDBLP.mtx',
    'AK': '../profiling/datasets/ak2010.mtx',
    'KA': '../profiling/datasets/karate.csv',  # for test only
}


def read_graph_original(name_dataset: strings.dataset, thres, gs):
    path = dict_path_dataset_original[name_dataset.value] + '.ori.viz'
    # path = dict_path_dataset_original[name_dataset.value] + '.new.viz'
    path = dict_path_dataset_original[name_dataset.value] + '.new.new.ppopp_thres_{}_n{}.viz'.format(thres, gs)
    g = nx.read_edgelist(path, create_using=nx.DiGraph)  # type: nx.DiGraph
    print(path)
    return g


def read_graph_reordered(name_dataset: strings.dataset):
    name_reorder = strings.reorder.TVC
    path = 'dataset.' + name_dataset.value + '/tiling.Sparse_Reorder.' + name_reorder.value + '/adj.txt'
    g = nx.read_edgelist(path, create_using=nx.DiGraph)  # type: nx.DiGraph
    print(path)
    return g


def main():
    # name_dataset = strings.dataset.KA
    name_dataset = strings.dataset.AK
    G = read_graph_original(name_dataset, 0.2, 2048)
    # G = read_graph_reordered(name_dataset)
    
    print(G.number_of_nodes(), G.number_of_edges())
    print(type(G))
    draw_adjacency_matrix(G)


if __name__ == '__main__':
    t1 = time.time()
    main()
    t2 = time.time()
    print('render latency: {} s.'.format(t2 - t1))
