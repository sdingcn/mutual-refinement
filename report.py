import os
import os.path
import sys
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

def scan_lzr():
    with open('lzr_time.txt', 'r') as f:
        lines = f.readlines()
    result = {}
    for line in lines:
        bench, time = line.strip().split()
        result[bench] = float(time)
    return result

def print_2a(f):
    for bench in Taint:
        r = scan_graph(f'mr/exp/graphs/taint/{bench}.dot')
        f.write(f'{bench} & ({r["v"]}, {r["e"]})\\\\\n')

def print_2b(f):
    for bench in Valueflow:
        r = scan_graph(f'mr/exp/graphs/valueflow/{bench}.dot')
        f.write(f'{bench} & ({r["v"]}, {r["e"]})\\\\\n')

def print_3a(f):
    for bench in Taint:
        rn = scan_mr_naive(f'mr/exp/results/taint/naive-{bench}.result')
        rr = scan_mr_refine(f'mr/exp/results/taint/refine-{bench}.result')
        f.write(f'{bench} & {rr["iter"]} & {rn["pair"]} & {rr["pair"]} & {rn["time"]} & {rr["time"]} & {rn["space"]} & {rr["space"]}\\\\\n')

def print_3b(f):
    for bench in Valueflow:
        rn = scan_mr_naive(f'mr/exp/results/valueflow/naive-{bench}.result')
        rr = scan_mr_refine(f'mr/exp/results/valueflow/refine-{bench}.result')
        f.write(f'{bench} & {rr["iter"]} & {rn["pair"]} & {rr["pair"]} & {rn["time"]} & {rr["time"]} & {rn["space"]} & {rr["space"]}\\\\\n')

def print_4(f):
    lzr_time_map = scan_lzr()
    for bench in Taint:
        mr_pair = scan_mr_refine(f'mr/exp/results/taint/refine-{bench}.result')['pair']
        lzr_mr_pair = scan_mr_refine(f'mr/exp/results/simplified-taint/refine-{bench}.result')['pair']
        mr_time = scan_mr_refine(f'mr/exp/results/taint/refine-{bench}.result')['time']
        lzr_mr_time = lzr_time_map[bench] + scan_mr_refine(f'mr/exp/results/simplified-taint/refine-{bench}.result')['time']
        ori_size = scan_graph(f'mr/exp/graphs/taint/{bench}.dot')['e']
        lzr_size = scan_graph(f'mr/exp/graphs/simplified-taint/{bench}.dot')['e']
        lzr_mr_size = scan_mr_refine(f'mr/exp/results/simplified-taint/refine-{bench}.result')['e1']
        f.write(f'{bench} & {mr_pair} & {lzr_mr_pair} & {mr_time} & {lzr_mr_time} & {ori_size - lzr_size} & {ori_size - lzr_mr_size}\\\\\n')

def log(f):
    f.write('=== 2a ===\n')
    print_2a(f)
    f.write('=== 2b ===\n')
    print_2b(f)
    f.write('=== 3a ===\n')
    print_3a(f)
    f.write('=== 3b ===\n')
    print_3b(f)
    f.write('=== 4 ===\n')
    print_4(f)

def main():
    log('report.txt')

if __name__ == '__main__':
    main()
