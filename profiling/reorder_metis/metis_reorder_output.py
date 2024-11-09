import networkx as nx
import random

from typing import List


def generate(path: str):
    # read file permutation
    with open(path + '.metis_in.iperm') as f:
        lines_map = f.readlines()
    
    # parse permutation
    permutation = []
    for line in lines_map:
        permutation.append(int(line.strip()))
        if permutation[-1] < 0:
            raise Exception(permutation[-1])
    
    # read file coo
    with open(path) as f:
        lines_coo = f.readlines()
    
    # parse metadata
    items = lines_coo[0].strip().split(' ')
    num_v = int(items[0])
    num_e = int(items[1])
    
    # reorder the graph according to permutation
    output = ''
    for line in lines_coo[1:]:
        items = line.strip().split(' ')
        src, dst = int(items[0]), int(items[1])
        new_line = '{} {}\n'.format(permutation[src], permutation[dst])
        output += new_line
    
    # write to file
    with open(path + '.metis_out', 'w') as f:
        f.write('{} {}\n'.format(num_v, num_e))
        f.write(output)


def main():
    ps = [
        # 'ak2010.mtx',
        'coAuthorsDBLP.mtx',
        # 'directed/cit-Patents.csv',
        # 'directed/soc-LiveJournal1.csv',
        # 'undirected/hollywood.csv'
    ]
    for p_dataset in ps:
        p_dataset = '../datasets/' + p_dataset + '.coo'
        print(p_dataset)
        generate(p_dataset)
    print('finished.')


if __name__ == '__main__':
    main()
