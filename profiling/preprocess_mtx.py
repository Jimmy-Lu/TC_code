import networkx as nx
import random

random.seed(123)

coo = True
csr = False
csc = False
viz = False

names = {
    'ka': 'karate.csv',
    'ak': 'ak2010.mtx',
    'ad': 'coAuthorsDBLP.mtx',
    'cp': 'directed/cit-Patents.csv',
    'sl': 'directed/soc-LiveJournal1.csv',
    'hw': 'undirected/hollywood.csv'
}


def read_graph_from_file(path_dataset: str) -> nx.DiGraph:
    with open(path_dataset) as f:
        lines = f.readlines()
    
    COO = []
    
    def to_direct_graph(COO):
        COO_re = []
        for e in COO:
            if e[0] == e[1]: continue
            COO_re.append([e[1], e[0]])
        return COO + COO_re
    
    all_larger = True
    all_smaller = True
    
    for line in lines:
        if line[0] in ['#', '%']: continue
        items = line.strip().split(' ')
        if len(items) not in [2, 3]: continue
        COO.append([items[0], items[1]])
        src = int(items[0])
        dst = int(items[1])
        if all_smaller or all_larger:
            if src > dst:
                all_smaller = False
            elif src < dst:
                all_larger = False
    
    if all_smaller or all_larger:
        print('To directed graph')
        COO = to_direct_graph(COO)
    G = nx.from_edgelist(COO, nx.DiGraph)  # type: nx.DiGraph
    return G


def graph_vid_randomize(G: nx.DiGraph) -> nx.DiGraph:
    dst = list(range(G.number_of_nodes()))
    random.shuffle(dst)
    mapping = dict(zip(list(G), dst))
    return nx.relabel_nodes(G, mapping)


class CSC_CSR(object):
    def __init__(self):
        self.offset = []
        self.idx = []


class Generate(object):
    def __init__(self, COO: nx.DiGraph, nv, ne):
        self.COO = COO
        self.ne = ne
        self.nv = nv
        
        self.CSC = None  # type: CSC_CSR
        self.CSR = None  # type: CSC_CSR
    
    def generate_coo_file(self, path_dataset):
        cnt = 0
        edge_list = ''
        for e in self.COO:
            edge_list += '{} {}\n'.format(e[0], e[1])
            cnt += 1
        assert cnt == self.ne
        
        with open(path_dataset + '.coo', 'w') as f:
            f.write('{} {}\n'.format(self.nv,
                                     self.ne))
            f.write(edge_list)
    
    def generate_viz_file(self, path_dataset):
        edge_list = ''
        for e in self.COO:
            edge_list += '{} {}\n'.format(e[0], e[1])
        with open(path_dataset + '.viz', 'w') as f:
            f.write(edge_list)
    
    def generate_csc_file(self, path_dataset):
        cnt = 0
        col_ptr = ''
        for ptr in self.CSC.offset:
            col_ptr += '{} '.format(ptr)
            cnt += 1
        col_ptr = col_ptr + ' \n'
        assert cnt == self.nv + 1
        
        cnt = 0
        row_idx = ''
        for idx in self.CSC.idx:
            row_idx += '{} '.format(idx)
            cnt += 1
        row_idx = row_idx + ' \n'
        assert cnt == self.ne
        
        with open(path_dataset + '.csc', 'w') as f:
            f.write('{} {}\n'.format(self.nv,
                                     self.ne))
            f.write(col_ptr + row_idx)
    
    def generate_csr_file(self, path_dataset):
        cnt = 0
        row_ptr = ''
        for ptr in self.CSR.offset:
            row_ptr += '{} '.format(ptr)
            cnt += 1
        row_ptr = row_ptr + ' \n'
        assert cnt == self.nv + 1
        
        cnt = 0
        col_idx = ''
        for idx in self.CSR.idx:
            col_idx += '{} '.format(idx)
            cnt += 1
        col_idx = col_idx + ' \n'
        assert cnt == self.ne
        
        with open(path_dataset + '.csr', 'w') as f:
            f.write('{} {}\n'.format(self.nv,
                                     self.ne))
            f.write(row_ptr + col_idx)
    
    def coo_to_csc_csr(self):
        self.CSR = CSC_CSR()
        self.CSC = CSC_CSR()
        self.CSR.offset = [0 for _ in range(self.nv)]
        self.CSC.offset = [0 for _ in range(self.nv)]
        self.CSR.idx = [-1 for _ in range(self.ne)]
        self.CSC.idx = [-1 for _ in range(self.ne)]
        
        for e in self.COO:
            self.CSR.offset[e[0]] += 1
            self.CSC.offset[e[1]] += 1
        
        cnt_csr, cnt_csc = 0, 0
        for i in range(self.nv):
            cnt_csr += self.CSR.offset[i]
            cnt_csc += self.CSC.offset[i]
            self.CSR.offset[i] = cnt_csr
            self.CSC.offset[i] = cnt_csc
        assert cnt_csr == self.ne
        assert cnt_csc == self.ne
        self.CSR.offset = [0] + self.CSR.offset
        self.CSC.offset = [0] + self.CSC.offset
        
        for e in self.COO:
            self.CSR.idx[self.CSR.offset[e[0]]] = e[1]
            self.CSC.idx[self.CSC.offset[e[1]]] = e[0]
            self.CSR.offset[e[0]] += 1
            self.CSC.offset[e[1]] += 1
        self.CSR.offset.pop(self.nv)
        self.CSC.offset.pop(self.nv)
        self.CSR.offset = [0] + self.CSR.offset
        self.CSC.offset = [0] + self.CSC.offset
    
    def generate_files(self, path_dataset):
        formats, functions = [], []
        
        if csc:
            formats.append('csc')
            functions.append(self.generate_csc_file)
        if csr:
            formats.append('csr')
            functions.append(self.generate_csr_file)
        if coo:
            formats.append('coo')
            functions.append(self.generate_coo_file)
        if viz:
            formats.append('viz')
            functions.append(self.generate_viz_file)
        
        assert len(functions) == len(formats)
        for i in range(len(formats)):
            g_format = formats[i]
            print('Generating {} to {} ...'.format(
                g_format.upper(), path_dataset + '.' + formats[i]))
            functions[i](path_dataset)
            print('--- finished.')
        
        print('============================================')


def main():
    ps = [
        names['ad'],
    ]
    for n_dataset in ps:
        p_dataset = 'datasets/' + n_dataset
        print(p_dataset)
        graph_nx = read_graph_from_file(p_dataset)
        graph_nx = graph_vid_randomize(graph_nx)
        coo_nx = nx.to_edgelist(graph_nx)
        
        gen = Generate(coo_nx,
                       graph_nx.number_of_nodes(),
                       graph_nx.number_of_edges())
        gen.coo_to_csc_csr()
        gen.generate_files(p_dataset)


if __name__ == '__main__':
    main()
