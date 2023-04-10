
import os
import os.path
import sys

Taint = [
    "backflash",
    "batterydoc",
    "droidkongfu",
    "fakebanker",
    "fakedaum",
    "faketaobao",
    "jollyserv",
    "loozfon",
    "phospy",
    "roidsec",
    "scipiex",
    "simhosy",
    "skullkey",
    "uranai",
    "zertsecurity"
]

Valueflow = [
    "cactus",
    "imagick",
    "leela",
    "nab",
    "omnetpp",
    "parest",
    "perlbench",
    "povray",
    "x264",
    "xz"
]

def scan_graph(fpath):
    nodes = set()
    edges = set()
    with open(fpath, "r") as f:
        lines = f.read().strip().splitlines()
        for line in lines:
            if "->" in line:
                n1, n2tail = line.split("->")
                n2, _ = n2tail.split("[")
                nodes.add(n1.strip())
                nodes.add(n2.strip())
                edges.add(line.strip())
    return { "v" : len(nodes), "e" : len(edges) }

def scan_mr_naive(fpath):
    result = {}
    with open(fpath, "r") as f:
        content = f.read()
        lines = list(map(lambda l : l.strip(), content.splitlines()))
        for line in lines:
            if line.startswith("Number of Reachable Pairs"):
                result["pair"] = int(line.split()[-1].strip())
            elif line.startswith("Total Time"):
                result["time"] = round(float(line.split()[-1].strip()), 3)
            elif line.startswith("Peak Space"):
                result["space"] = int(line.split()[-1].strip())
    if "pair" not in result:
        result["pair"] = '-'
    if "time" not in result:
        result["time"] = '-'
    if "space" not in result:
        result["space"] = '-'
    return result

def scan_mr_refine(fpath):
    result = {}
    with open(fpath, "r") as f:
        content = f.read()
        lines = list(map(lambda l : l.strip(), content.splitlines()))
        for line in lines:
            if line.startswith("Edge Set Reduction"):
                result["e0"] = int(line.split()[-3].strip())
                result["e1"] = int(line.split()[-1].strip())
            elif line.startswith("Number of Refinement Iterations"):
                result["iter"] = int(line.split()[-1].strip())
            elif line.startswith("Number of Reachable Pairs"):
                result["pair"] = int(line.split()[-1].strip())
            elif line.startswith("Total Time"):
                result["time"] = round(float(line.split()[-1].strip()), 3)
            elif line.startswith("Peak Space"):
                result["space"] = int(line.split()[-1].strip())
    if "e0" not in result:
        result["e0"] = '-'
    if "e1" not in result:
        result["e1"] = '-'
    if "iter" not in result:
        result["iter"] = '-'
    if "pair" not in result:
        result["pair"] = '-'
    if "time" not in result:
        result["time"] = '-'
    if "space" not in result:
        result["space"] = '-'
    return result

def scan_graphsimp(fpath):
    with open(fpath, 'r') as f:
        ori, red, t = f.read().strip().split()
    return { 'e0' : int(ori.strip()), 'e1' : int(red.strip()), 'time' : round(float(t.strip()), 3) }

def print_2a():
    for bench in Taint:
        r = scan_graph(f'mutual-refinement/exp/graphs/taint/{bench}.dot')
        print(f'{bench} & ({r["v"]}, {r["e"]})\\\\')

def print_2b():
    for bench in Valueflow:
        r = scan_graph(f'mutual-refinement/exp/graphs/valueflow/{bench}.dot')
        print(f'{bench} & ({r["v"]}, {r["e"]})\\\\')

def print_3a():
    for bench in Taint:
        rn = scan_mr_naive(f'mutual-refinement/exp/results/taint/naive-{bench}.result')
        rr = scan_mr_refine(f'mutual-refinement/exp/results/taint/refine-{bench}.result')
        print(f'{bench} & {rr["iter"]} & {rn["pair"]} & {rr["pair"]} & {rn["time"]} & {rr["time"]} & {rn["space"]} & {rr["space"]}\\\\')

def print_3b():
    for bench in Valueflow:
        rn = scan_mr_naive(f'mutual-refinement/exp/results/valueflow/naive-{bench}.result')
        rr = scan_mr_refine(f'mutual-refinement/exp/results/valueflow/refine-{bench}.result')
        print(f'{bench} & {rr["iter"]} & {rn["pair"]} & {rr["pair"]} & {rn["time"]} & {rr["time"]} & {rn["space"]} & {rr["space"]}\\\\')

def print_4():
    for bench in Taint:
        r_ori = scan_graph(f'mutual-refinement/exp/graphs/taint/{bench}.dot')
        r_mr = scan_mr_refine(f'mutual-refinement/exp/results/taint/refine-{bench}.result')
        r_lzr = scan_graphsimp(f'graphsimp-results/{bench}.result')
        print(f'{bench} & {r_ori["e"]} & {r_lzr["e1"]} & {r_mr["e1"]} & {r_lzr["time"]} & {r_mr["time"]}\\\\')

def main():
    print('2a')
    print_2a()
    print('2b')
    print_2b()
    print('3a')
    print_3a()
    print('3b')
    print_3b()
    print('4')
    print_4()

if __name__ == "__main__":
    main()

import shutil
import subprocess
import os.path

Taint = [
    "backflash",
    "batterydoc",
    "droidkongfu",
    "fakebanker",
    "fakedaum",
    "faketaobao",
    "jollyserv",
    "loozfon",
    "phospy",
    "roidsec",
    "scipiex",
    "simhosy",
    "skullkey",
    "uranai",
    "zertsecurity"
]

def clean_graph(path):
    with open(path, 'r') as f:
        lines = f.readlines()
    with open(path, 'w') as f:
        for line in lines:
            if '->' in line:
                f.write(line.strip() + '\n')

def count_edges(path):
    with open(path, 'r') as f:
        lines = f.readlines()
    s = set()
    for line in lines:
        s.add(line.strip())
    return len(s)

def main():
    for bench in Taint:
        '''
        print(f'simplifying {bench}')
        shutil.copyfile(f'mutual-refinement/exp/graphs/taint/{bench}.dot', 'interdyck_graph_reduce/current.dot')
        '''
        clean_graph(f'mr/exp/graphs/taint/{bench}.dot')
        '''original = count_edges('interdyck_graph_reduce/current.dot')
        output = subprocess.run(
                './graph_reduce.sh',
                shell = True,
                stdout = subprocess.PIPE,
                stderr = subprocess.STDOUT,
                universal_newlines = True,
                timeout = 14400,
                cwd = 'interdyck_graph_reduce/').stdout
        t = output.strip().split()[-1]
        reduced = count_edges('interdyck_graph_reduce/current.dot')
        with open(f'graphsimp-results/{bench}.result', 'w') as f:
            f.write(f'{original} {reduced} {t}')
        '''

if __name__ == '__main__':
    main()
