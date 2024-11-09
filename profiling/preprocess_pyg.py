import networkx as nx
import torch_geometric as pyg


def load_ogbn(d_name: str):
    from ogb.nodeproppred import PygNodePropPredDataset
    dataset = PygNodePropPredDataset(
        name='ogbn-' + d_name, root='datasets/{}'.format(d_name.lower()))
    graph = dataset[0]  # pyg graph object
    return graph


def load_ccp(d_name: str):
    from torch_geometric.datasets import Planetoid
    names = {'cora': 'Cora', 'citeseer': 'CiteSeer', 'pubmed': 'PubMed'}
    dataset = Planetoid(
        name=names[d_name], root='datasets/{}'.format(d_name))
    graph = dataset[0]  # pyg graph object
    return graph


def load_reddit(d_name):
    assert d_name == 'reddit'
    from torch_geometric.datasets import Reddit
    dataset = Reddit(root='datasets/{}'.format(d_name))
    graph = dataset[0]  # pyg graph object
    return graph


def load_flickr(d_name):
    assert d_name == 'flickr'
    from torch_geometric.datasets import Flickr
    dataset = Flickr(root='datasets/{}'.format(d_name))
    graph = dataset[0]  # pyg graph object
    return graph


def load_yelp(d_name):
    assert d_name == 'yelp'
    from torch_geometric.datasets import Yelp
    dataset = Yelp(root='datasets/{}'.format(d_name))
    graph = dataset[0]  # pyg graph object
    return graph


def crossbar(d_name):
    if d_name == 'reddit':
        return load_reddit(d_name)
    elif d_name == 'flickr':
        return load_flickr(d_name)
    elif d_name == 'yelp':
        return load_yelp(d_name)
    elif d_name in ['arxiv', 'products']:
        return load_ogbn(d_name)
    elif d_name in ['cora', 'citeseer', 'pubmed']:
        return load_ccp(d_name)
    else:
        raise Exception(d_name)


def main():
    from preprocess_mtx import graph_vid_randomize, Generate
    names = [
        'cora',
        'citeseer',
        'pubmed',
        
        'arxiv',
        'products',
        'flickr',
        'yelp',
        'reddit',
    ]
    
    for n_dataset in names:
        graph_pyg = crossbar(n_dataset)
        
        graph_nx = pyg.utils.to_networkx(graph_pyg)
        graph_nx = graph_vid_randomize(graph_nx)
        coo_nx = nx.to_edgelist(graph_nx)
        
        gen = Generate(coo_nx,
                       graph_nx.number_of_nodes(),
                       graph_nx.number_of_edges())
        gen.coo_to_csc_csr()
        gen.generate_files('datasets/{}'.format(n_dataset))


if __name__ == '__main__':
    main()
