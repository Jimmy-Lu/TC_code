import time
import queue as Q
from datasketch import MinHash, MinHashLSH
from chelp import jd, Pair, makenum, root
from chelp import thres, per

DEBUG = False


def clustering(lists, begin, end, mapping_encode, lsh_thres):
    numv_l = end - begin
    global t0, t2, t4, t6, t7
    
    if DEBUG: t0 = time.time()
    lsh = MinHashLSH(threshold=lsh_thres, num_perm=per)
    allver = [None for _ in range(numv_l)]
    for i in range(numv_l):
        list_src = lists[i + begin]
        m = MinHash(num_perm=per)
        for src in list_src:
            m.update(str(src).encode('utf-8'))
        lsh.insert(str(i), m)
        allver[i] = m
    if DEBUG:
        # res = lsh.query(allver[0])
        # print(res)
        t1 = time.time()
        print("init LSH", t1 - t0)
    
    if DEBUG: t2 = time.time()
    sset = set()
    que = Q.PriorityQueue()
    for i in range(numv_l):
        if DEBUG and i % 100000 == 0:
            print(i + begin, end=' ')
        
        if len(lists[i + begin]) == 0:
            continue
        res = lsh.query(allver[i])
        # print(len(res), len(lists[i + begin]))
        for item in res:
            if int(item) == i or makenum(i, int(item), numv_l) in sset:
                continue
            que.put(Pair(i, int(item),
                         jd(lists[i + begin], lists[int(item) + begin])))
            sset.add(makenum(i, int(item), numv_l))
            # print("add", i, int(item))
    if DEBUG:
        t3 = time.time()
        print("queuesize:", que.qsize(), end="\t")
        print("query LSH", t3 - t2)
        print('-------------------------')
    
    cluster_id = [i for i in range(numv_l)]
    cluster_sz = [1 for _ in range(numv_l)]
    deleted = [0 for _ in range(numv_l)]
    
    num_cluster = numv_l
    
    if DEBUG: t4 = time.time()
    while (not que.empty()) and num_cluster > 0:
        item = que.get()
        p1 = item.p1
        p2 = item.p2
        # print(p1, p2)
        sset.remove(makenum(p1, p2, numv_l))
        if p1 == cluster_id[p1] and p2 == cluster_id[p2]:
            if deleted[p1] or deleted[p2]:
                continue
            if cluster_sz[p1] < cluster_sz[p2]:
                cluster_id[p1] = p2
                num_cluster = num_cluster - 1
                cluster_sz[p2] = cluster_sz[p1] + cluster_sz[p2]
                if cluster_sz[p2] >= thres:
                    deleted[p2] = 1
                    num_cluster = num_cluster - 1
            else:
                cluster_id[p2] = p1
                num_cluster = num_cluster - 1
                cluster_sz[p1] = cluster_sz[p1] + cluster_sz[p2]
                if cluster_sz[p1] >= thres:
                    deleted[p1] = 1
                    num_cluster = num_cluster - 1
        else:
            p1 = root(p1, cluster_id)
            p2 = root(p2, cluster_id)
            if deleted[p1] or deleted[p2]:
                continue
            if p1 != p2 and not makenum(p1, p2, numv_l) in sset:
                que.put(Pair(p1, p2,
                             jd(lists[p1 + begin], lists[p2 + begin])))
                sset.add(makenum(p1, p2, numv_l))
                # print("add", i, (int)(item))
    if DEBUG:
        t5 = time.time()
        print("clustering", t5 - t4)
        print('-------------------------')
    clusters = {}
    if DEBUG: t6 = time.time()
    for i in range(numv_l):
        root_i = root(i, cluster_id)
        if root_i in clusters:
            clusters[root_i].append(i)
        else:
            clusters[root_i] = [i]
    if DEBUG:
        t7 = time.time()
        print('cluster #:', len(clusters))
        print("put into clusters", t7 - t6)
        print('-------------------------')
    cnt = begin
    for k in clusters:
        for item in clusters[k]:
            mapping_encode[item + begin] = cnt
            cnt += 1
    if DEBUG: print("mapping generation", time.time() - t7)


def generate_adj(numv, nume, lists, mapping_encode, path, groupsize, lsh_thres, vis=False):
    global t8
    if groupsize == -1: groupsize = 'All'
    if DEBUG: t8 = time.time()
    path_target = path + ".ppopp_thres_" + str(lsh_thres) + "_n" + str(groupsize)
    print(path_target)
    with open(path_target, 'w') as f:
        f.write('{}\n{}\n'.format(numv, nume))
        for dst in range(numv):
            list_src = lists[dst]
            for src in list_src:
                f.write('{} {}\n'.format(mapping_encode[src], mapping_encode[dst]))
    if vis is True:
        with open(path_target + '.viz', 'w') as f:
            for dst in range(numv):
                list_src = lists[dst]
                for src in list_src:
                    f.write('{} {}\n'.format(mapping_encode[src], mapping_encode[dst]))
    if DEBUG: print("write back", time.time() - t8)


def read_graph(path_dataset):
    with open(path_dataset) as f:
        lines = f.readlines()
    numv = int(lines.pop(0).strip())
    nume = int(lines.pop(0).strip())
    lists = [[] for _ in range(numv)]
    for line in lines:
        items = line.strip().split(' ')
        src = int(items[0])
        dst = int(items[1])
        lists[dst].append(src)
    return numv, nume, lists, path_dataset


def map_verification():
    import math
    from chelp import ps
    
    gs = 2048
    path = 'datasets/' + ps['ak'] + '.new'
    G = read_graph(path)
    numv, nume, lists, path = G
    print('{}: v{} e{}'.format('ak', numv, nume))
    num_group = math.ceil(numv / gs)
    mapping_encode = [-1 for _ in range(numv)]
    for i in range(num_group):
        begin = i * gs
        end = min(numv, (i + 1) * gs)
        print(begin, end)
        clustering(lists, begin, end, mapping_encode, 0.2)
    visited = [False for _ in range(numv)]
    for i in range(numv):
        visited[mapping_encode[i]] = True
    bad = False
    for i in range(numv):
        if visited[i] is False:
            bad = True
            print(i, False)
    if bad:
        print('bad mapping')
    else:
        print('good mapping')


if __name__ == '__main__':
    map_verification()
