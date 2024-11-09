class CheckPreprocess(object):
    def __init__(self, p_dataset):
        self.lol_csr = {}
        self.lol_csc = {}
        self.num_v = -1
        self.num_e = -1
        
        self.p_dataset = p_dataset
        self.read_coo()
        self.check_csr()
        self.check_csc()
    
    def read_coo(self):
        with open(self.p_dataset + '.coo', 'r') as f:
            lines = f.readlines()
        items = lines.pop(0).strip().split()
        self.num_v = int(items[0])
        self.num_e = int(items[1])
        assert self.num_e == len(lines)
        for line in lines:
            items = line.strip().split()
            # csr
            if items[0] in self.lol_csr.keys():
                self.lol_csr[items[0]].append(items[1])
            else:
                self.lol_csr[items[0]] = [items[1]]
            # csc
            if items[1] in self.lol_csc.keys():
                self.lol_csc[items[1]].append(items[0])
            else:
                self.lol_csc[items[1]] = [items[0]]
    
    def check_csc(self):
        with open(self.p_dataset + '.csc', 'r') as f:
            lines = f.readlines()
        items = lines.pop(0).strip().split()
        assert self.num_v == int(items[0])
        assert self.num_e == int(items[1])
        
        ptr = lines.pop(0).strip().split()
        idx = lines.pop(0).strip().split()
        assert len(ptr) == self.num_v + 1
        assert len(idx) == self.num_e
        
        for i in range(2, self.num_v):
            offset_start = int(ptr[i])
            offset_end = int(ptr[i + 1])
            i = str(i)
            if i not in self.lol_csc.keys():
                assert offset_end - offset_start == 0
            if offset_end - offset_start != len(self.lol_csc[i]):
                raise Exception(offset_end, offset_start, len(self.lol_csc[i]))
            for j in range(offset_start, offset_end):
                assert idx[j] in self.lol_csc[i]
    
    def check_csr(self):
        pass


def main():
    ps = [
        # 'karate.csv',
        'delaunay_n13.mtx',
        # 'ak2010.mtx',
        # 'coAuthorsDBLP.mtx',
        # 'directed/cit-Patents.csv',
        # 'directed/soc-LiveJournal1.csv',
        # 'undirected/hollywood.csv'
    ]
    for p_dataset in ps:
        p_dataset = 'datasets/' + p_dataset
        print(p_dataset)
        CheckPreprocess(p_dataset)
        print('-- pass')


if __name__ == '__main__':
    main()
