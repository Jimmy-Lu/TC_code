import networkx as nx
import random

from typing import List


def generate(path: str):
    # read file coo
    with open(path) as f:
        lines = f.readlines()
    
    # parse metadata
    items = lines[0].strip().split(' ')
    num_v = int(items[0])
    CSR = [[] for _ in range(num_v)]  # type: List[List[str]]
    
    # read graph structure
    num_e = 0
    for line in lines[1:]:
        items = line.strip().split(' ')
        src, dst = int(items[0]), int(items[1])
        if src == dst:
            print(' skip:', src)
            continue
        if (dst + 1) not in CSR[src]:
            CSR[src].append(dst + 1)
            num_e += 1
            if (src + 1) not in CSR[dst]:
                CSR[dst].append(src + 1)
        
        if (src + 1) not in CSR[dst]:
            CSR[dst].append(src + 1)
            num_e += 1
            if (dst + 1) not in CSR[src]:
                CSR[src].append(dst + 1)
    
    # generate output string
    output = ''
    for i in range(len(CSR)):
        if len(CSR[i]) == 0:
            raise Exception(i)
        line = ''
        for nei in CSR[i]:
            line += '{} '.format(nei)
        output += line.strip() + '\n'
    
    # write to file
    print(num_e)
    with open(path + '.metis_in', 'w') as f:
        f.write('{} {}\n'.format(num_v, num_e))
        f.write(output)


def main():
    ps = [
        # 'ak2010.mtx',
        'coAuthorsDBLP.mtx',
        # 'directed/cit-Patents.csv',
        # 'undirected/hollywood.csv'
        # 'directed/soc-LiveJournal1.csv',
    ]
    for p_dataset in ps:
        p_dataset = '../datasets/' + p_dataset + '.coo'
        print(p_dataset)
        generate(p_dataset)
    print('finished.')


if __name__ == '__main__':
    main()
