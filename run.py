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

def scan_graphsimp(fpath):
    with open(fpath, 'r') as f:
        ori, red, t = f.read().strip().split()
    return { 'e0' : int(ori.strip()), 'e1' : int(red.strip()), 'time' : round(float(t.strip()), 3) }

def print_2a(f):
    for bench in Taint:
        r = scan_graph(f'mutual-refinement/exp/graphs/taint/{bench}.dot')
        f.write(f'{bench} & ({r["v"]}, {r["e"]})\\\\\n')

def print_2b(f):
    for bench in Valueflow:
        r = scan_graph(f'mutual-refinement/exp/graphs/valueflow/{bench}.dot')
        f.write(f'{bench} & ({r["v"]}, {r["e"]})\\\\\n')

def print_3a(f):
    for bench in Taint:
        rn = scan_mr_naive(f'mutual-refinement/exp/results/taint/naive-{bench}.result')
        rr = scan_mr_refine(f'mutual-refinement/exp/results/taint/refine-{bench}.result')
        f.write(f'{bench} & {rr["iter"]} & {rn["pair"]} & {rr["pair"]} & {rn["time"]} & {rr["time"]} & {rn["space"]} & {rr["space"]}\\\\\n')

def print_3b(f):
    for bench in Valueflow:
        rn = scan_mr_naive(f'mutual-refinement/exp/results/valueflow/naive-{bench}.result')
        rr = scan_mr_refine(f'mutual-refinement/exp/results/valueflow/refine-{bench}.result')
        f.write(f'{bench} & {rr["iter"]} & {rn["pair"]} & {rr["pair"]} & {rn["time"]} & {rr["time"]} & {rn["space"]} & {rr["space"]}\\\\\n')

def print_4(f):
    for bench in Taint:
        r_ori = scan_graph(f'mutual-refinement/exp/graphs/taint/{bench}.dot')
        r_mr = scan_mr_refine(f'mutual-refinement/exp/results/taint/refine-{bench}.result')
        r_lzr = scan_graphsimp(f'graphsimp-results/{bench}.result')
        f.write(f'{bench} & {r_ori["e"]} & {r_lzr["e1"]} & {r_mr["e1"]} & {r_lzr["time"]} & {r_mr["time"]}\\\\\n')

def log(f):
    print('2a')
    print_2a(f)
    print('2b')
    print_2b(f)
    print('3a')
    print_3a(f)
    print('3b')
    print_3b(f)
    print('4')
    print_4(f)

def run_and_get_output(c, d, s):
    return subprocess.run(
            c,
            cwd = d,
            shell = s,
            stdout = subprocess.PIPE,
            stderr = subprocess.STDOUT,
            universal_newlines = True,
    ).stdout

def main():
    # run valueflow analysis to get the graphs
    run_and_get_output('./analyze.sh', 'vf/', True)
    # run graph simplification to get the graphs
    for bench in Taint:
        shutil.copyfile(f'mr/exp/graphs/taint/{bench}.dot', 'lzr/current.dot')
        run_and_get_output('./graph_reduce.sh', 'lzr/', True)
        shutil.copyfile('lzr/current.dot', f'mr/exp/graphs/simplified-taint/{bench}.dot')
    # run mutual refinement
    run_and_get_output('./run.sh', 'mr/', True)
    # summerize and log results

if __name__ == '__main__':
    main()
