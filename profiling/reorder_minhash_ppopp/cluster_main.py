import math
import time

from chelp import ps
from clustering import clustering, generate_adj, read_graph
from tap import Tap


def cluster(G, lsh_thres, group_size):
    numv, nume, lists, path = G
    gs = group_size
    if gs == -1:
        # clustering over the whole graph
        gs = numv
        num_group = 1
    elif gs >= numv:
        print('Group size larger than vertex#.')
        return
    else:
        num_group = math.ceil(numv / gs)
    
    mapping_encode = [-1 for _ in range(numv)]
    for i in range(num_group):
        begin = i * gs
        end = min(numv, (i + 1) * gs)
        clustering(lists, begin, end, mapping_encode, lsh_thres)
    t1 = time.time_ns()
    generate_adj(numv, nume, lists, mapping_encode, path, group_size, lsh_thres)
    t2 = time.time_ns()
    print('adj gen time: {} msec'.format(math.ceil((t2 - t1) / 1000000)))


def main_all():
    # datasets = ['ak', 'ad']
    datasets = ['cp']
    lsh_thresholds = [0.2, 0.3, 0.1, ]
    group_sizes = [1024, 2048, 4096, 8192, -1]
    
    for lsh_threshold in lsh_thresholds:
        for d in datasets:
            print('========================================')
            path = 'datasets/' + ps[d] + '.new'
            G = read_graph(path)
            print('{}: v{} e{}'.format(d, G[0], G[1]))
            print("lsh_thres:", lsh_threshold)
            print('========================================')
            for gs in group_sizes:
                print('group size: {}'.format(gs))
                t1 = time.time_ns()
                cluster(G, lsh_threshold, gs)
                t2 = time.time_ns()
                print('reorder time: {} msec'.format(math.ceil((t2 - t1) / 1000000)))
                print('-----------------------------------')
    print('========================================')


class ArgumentParser(Tap):
    thres: float = 0.2
    gs: int = 1024
    d: str = 'ak'


def main_one():
    args = ArgumentParser(description='simulation - processing').parse_args()
    
    path = 'datasets/' + ps[args.d] + '.new'
    G = read_graph(path)
    print('{}: v{} e{}'.format(args.d, G[0], G[1]))
    print("lsh_thres:", args.thres)
    print('-----------------------------------')
    print('group size: {}'.format(args.gs))
    t1 = time.time_ns()
    cluster(G, args.thres, args.gs)
    t2 = time.time_ns()
    print('reorder time: {} msec'.format(math.ceil((t2 - t1) / 1000000)))


if __name__ == '__main__':
    main_one()
    # main_all()
