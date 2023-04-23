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
    result = {
        "v" : None,
        "e" : None
    }
    with open(fpath, "r") as f:
        lines = f.read().strip().splitlines()
        nodes = set()
        edges = set()
        for line in lines:
            line = line.strip()
            if "->" in line:
                n1, n2tail = line.split("->")
                n2, _ = n2tail.split("[")
                nodes.add(n1.strip())
                nodes.add(n2.strip())
                edges.add(line.strip())
        result["v"] = len(nodes)
        result["e"] = len(edges)
    return result

def scan_mr_naive(fpath):
    result = {
        "pair" : None,
        "time" : None,
        "space" : None
    }
    with open(fpath, "r") as f:
        lines = f.read().strip().splitlines()
        for line in lines:
            line = line.strip()
            if line.startswith("Number of Reachable Pairs"):
                result["pair"] = int(line.split()[-1].strip())
            elif line.startswith("Total Time"):
                result["time"] = float(line.split()[-1].strip())
            elif line.startswith("Peak Space"):
                result["space"] = int(line.split()[-1].strip()) * 1000 / 1048576
    return result

def scan_mr_refine(fpath):
    result = {
        "e0" : None,
        "e1" : None,
        "iter" : None,
        "pair" : None,
        "time" : None,
        "space" : None
    }
    with open(fpath, "r") as f:
        lines = f.read().strip().splitlines()
        for line in lines:
            line = line.strip()
            if line.startswith("Edge Set Reduction"):
                result["e0"] = int(line.split()[-3].strip())
                result["e1"] = int(line.split()[-1].strip())
            elif line.startswith("Number of Refinement Iterations"):
                result["iter"] = int(line.split()[-1].strip())
            elif line.startswith("Number of Reachable Pairs"):
                result["pair"] = int(line.split()[-1].strip())
            elif line.startswith("Total Time"):
                result["time"] = float(line.split()[-1].strip())
            elif line.startswith("Peak Space"):
                result["space"] = int(line.split()[-1].strip()) * 1000 / 1048576
    return result

def scan_lzr(fpath):
    result = {
        "time" : None,
        "space" : None
    }
    with open(fpath, 'r') as f:
        lines = f.read().strip().splitlines()
        for line in lines:
            line = line.strip()
            if line.startswith("elapsed time"):
                result['time'] = float(line.split()[-1].strip())
            elif line.startswith("Maximum resident set size (kbytes)"):
                result['space'] = int(line.split()[-1].strip()) * 1000 / 1048576
    return result

def p_id(x):
    if type(x) == int:
        return str(x)
    elif type(x) == float:
        return f'{round(x, 2):.2f}'
    else:
        return '-'

def p_add(a, b):
    if type(a) == int and type(b) == int:
        return str(a + b)
    elif type(a) == float and type(b) == float:
        return f'{round(a + b, 2):.2f}'
    else:
        return '-'

def p_sub(a, b):
    if type(a) == int and type(b) == int:
        return str(a - b)
    elif type(a) == float and type(b) == float:
        return f'{round(a - b, 2):.2f}'
    else:
        return '-'

def p_mul(a, b):
    if type(a) == int and type(b) == int:
        return str(a * b)
    elif type(a) == float and type(b) == float:
        return f'{round(a * b, 2):.2f}'
    else:
        return '-'

def p_div(a, b):
    if type(a) == int and type(b) == int:
        return str(a // b)
    elif type(a) == float and type(b) == float:
        return f'{round(a / b, 2):.2f}'
    else:
        return '-'

def p_max(a, b):
    if type(a) == int and type(b) == int:
        return str(max(a, b))
    elif type(a) == float and type(b) == float:
        return f'{round(max(a, b), 2):.2f}'
    else:
        return '-'

def print_2a(f):
    for bench in Taint:
        r = scan_graph(f'mr/exp/graphs/taint/{bench}.dot')
        f.write('{} & ({}, {})\\\\\n'.format(
            bench, p_id(r['v']), p_id(r['e'])
        ))

def print_2b(f):
    for bench in Valueflow:
        r = scan_graph(f'mr/exp/graphs/valueflow/{bench}.dot')
        f.write('{} & ({}, {})\\\\\n'.format(
            bench, p_id(r['v']), p_id(r['e'])
        ))

def print_3a(f):
    for bench in Taint:
        rn = scan_mr_naive(f'mr/exp/results/taint/naive-{bench}.result')
        rr = scan_mr_refine(f'mr/exp/results/taint/refine-{bench}.result')
        f.write('{} & {} & {} & {} & {} & {} & {} & {}\\\\\n'.format(
            bench, p_id(rr['iter']),
            p_id(rn['pair']), p_id(rr['pair']),
            p_id(rn['time']), p_id(rr['time']),
            p_id(rn['space']), p_id(rr['space'])
        ))

def print_3b(f):
    for bench in Valueflow:
        rn = scan_mr_naive(f'mr/exp/results/valueflow/naive-{bench}.result')
        rr = scan_mr_refine(f'mr/exp/results/valueflow/refine-{bench}.result')
        f.write('{} & {} & {} & {} & {} & {} & {} & {}\\\\\n'.format(
            bench, p_id(rr['iter']),
            p_id(rn['pair']), p_id(rr['pair']),
            p_id(rn['time']), p_id(rr['time']),
            p_id(rn['space']), p_id(rr['space'])
        ))

def print_4(f):
    for bench in Taint:
        ori_g = scan_graph(f'mr/exp/graphs/taint/{bench}.dot')
        lzr_g = scan_graph(f'mr/exp/graphs/simplified-taint/{bench}.dot')
        mr_r = scan_mr_refine(f'mr/exp/results/taint/refine-{bench}.result')
        lzr_r = scan_lzr(f'lzr/resource-logs/{bench}.log')
        lzrmr_r = scan_mr_refine(f'mr/exp/results/simplified-taint/refine-{bench}.result')
        f.write('{} & {} & {} & {} & {} & {} & {} & {} & {}\\\\\n'.format(
            bench,
            p_id(mr_r['pair']), p_id(lzrmr_r['pair']),
            p_id(mr_r['time']), p_add(lzr_r['time'], lzrmr_r['time']),
            p_id(mr_r['space']), p_max(lzr_r['space'], lzrmr_r['space']),
            p_sub(ori_g['e'], lzr_g['e']), p_sub(ori_g['e'], lzrmr_r['e1'])
        ))

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
    with open('report.txt', 'w') as f:
        log(f)

if __name__ == '__main__':
    main()
