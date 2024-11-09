per = 64
thres = 64

ps = {
    'ka': 'karate.csv',
    'ak': 'ak2010.mtx',
    'ad': 'coAuthorsDBLP.mtx',
    'cp': 'directed/cit-Patents.csv',
    'sl': 'directed/soc-LiveJournal1.csv',
    'hw': 'undirected/hollywood.csv',
}


def jd(l1, l2):
    if len(l1) == 0 or len(l2) == 0:
        return 0
    s1 = set(l1)
    s2 = set(l2)
    return float(len(s1.intersection(s2))) / len(s1.union(s2))


def makenum(a, b, size):
    if a > b:
        return a * size + b
    else:
        return b * size + a


def root(i, cluster_id):
    while i != cluster_id[i]:
        cluster_id[i] = cluster_id[cluster_id[i]]
        i = cluster_id[i]
    return i


class Pair(object):
    def __init__(self, p1, p2, similarity):
        self.p1 = p1
        self.p2 = p2
        self.simi = similarity
    
    def __lt__(self, other):  # operator <
        return self.simi > other.simi
    
    def __str__(self):
        return str(self.p1) + ' ' + str(self.p2) + ' ' + str(self.simi)
        # return '(' + str(self.priority)+',\'' + self.description + '\')'
