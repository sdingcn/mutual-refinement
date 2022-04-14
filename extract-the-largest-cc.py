import sys
import os.path
from collections import deque

def get_raw_edges(lines):
    raw_edges = []
    for line in lines:
        line.strip()
        first, tail1 = line.split('->')
        second, tail2 = tail1.split('[')
        _, label, __ = tail2.split('"')
        raw_edges.append((first, second, label))
    return raw_edges

def number_nodes(raw_edges):
    ctr = 0
    node_to_number = {}
    number_to_node = {}
    for first, second, label in raw_edges:
        if first not in node_to_number:
            node_to_number[first] = ctr
            number_to_node[ctr] = first
            ctr += 1
        if second not in node_to_number:
            node_to_number[second] = ctr
            number_to_node[ctr] = second
            ctr += 1
    return node_to_number, number_to_node

def build_graph(edges, n):
    G = [[] for i in range(n)]
    for first, second, label in edges:
        G[first].append(second)
        G[second].append(first)
    return G

def get_largest_cc(G):
    n = len(G)
    vis = [False for i in range(n)]
    def bfs(v):
        nonlocal vis
        Q = deque([v])
        s = set()
        while len(Q) > 0:
            v = Q.popleft()
            s.add(v)
            for w in G[v]:
                if not vis[w]:
                    vis[w] = True
                    Q.append(w)
        return s
    ccs = []
    for i in range(n):
        if not vis[i]:
            ccs.append(bfs(i))
    ccs.sort(key = lambda s : len(s), reverse = True)
    if len(ccs) > 0:
        return ccs[0]
    else:
        return set()

def raw_edge_to_line(e):
    first, second, label = e
    return '{}->{}[label="{}"]'.format(first, second, label)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        sys.exit('Usage: python3 {} <graph-file>'.format(sys.argv[0]))
    with open(sys.argv[1], 'r') as f:
        lines = f.readlines()
    raw_edges = get_raw_edges(lines)
    node_to_number, number_to_node = number_nodes(raw_edges)
    edges = []
    for first, second, label in raw_edges:
        edges.append((node_to_number[first], node_to_number[second], label))
    G = build_graph(edges, len(node_to_number))
    lcc = get_largest_cc(G)
    lcc_raw_edges = []
    for first, second, label in raw_edges:
        if node_to_number[first] in lcc:
            lcc_raw_edges.append((first, second, label))
    with open(sys.argv[1] + '.cc', 'w') as f:
        for e in lcc_raw_edges:
            f.write(raw_edge_to_line(e) + '\n')
