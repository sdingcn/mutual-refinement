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

def arith_mean(lst):
    n = len(lst)
    x = 0
    for v in lst:
        x += v
    return round(x / n, 3)

def geo_mean(lst):
    n = len(lst)
    x = 1
    for v in lst:
        x *= v
    return round(x ** (1 / n), 3)

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
    return { "V" : len(nodes), "E" : len(edges) }

def scan_result(fpath):
    result = {}
    with open(fpath, "r") as f:
        content = f.read()
        lines = list(map(lambda l : l.strip(), content.splitlines()))
        for line in lines:
            if line.startswith("Number of Refinement Iterations"):
                result["rounds"] = int(line.split()[-1].strip())
            elif line.startswith("Number of Reachable Pairs"):
                result["precision"] = int(line.split()[-1].strip())
            elif line.startswith("Total Time"):
                result["time"] = float(line.split()[-1].strip())
            elif line.startswith("Peak Space"):
                result["space"] = float(line.split()[-1].strip())
    return result

def collect_data(name, graph_path, naive_result_path, refine_result_path):
    g = scan_graph(graph_path)
    nr = scan_result(naive_result_path)
    rr = scan_result(refine_result_path)
    ok = ("time" in nr) and ("time" in rr)
    return {
        "V"  : g["V"],
        "E"  : g["E"],
        "R"  : rr["rounds"] if ok else "-",
        "PI" : round(nr["precision"] / rr["precision"], 3) if ok else "-",
        "TI" : round(rr["time"] / nr["time"], 3) if ok else "-",
        "SI" : round(rr["space"] / nr["space"], 3) if ok else "-"
    }

def print_table(analysis, names, graphs_dir, results_dir):
    print("*** {} ***".format(analysis))
    print("Benchmark & $(|V|, |E|)$ & Rounds & Precision Improvement & Time Increase & Space Increase\\\\")
    Vs = []
    Es = []
    Rs = []
    PIs = []
    TIs = []
    SIs = []
    for name in names:
        graph_path = os.path.join(graphs_dir, "{}.dot".format(name))
        naive_result_path = os.path.join(results_dir, "naive-{}.result".format(name))
        refine_result_path = os.path.join(results_dir, "refine-{}.result".format(name))
        data = collect_data(name, graph_path, naive_result_path, refine_result_path)
        print("{} & ({}, {}) & {} & {}x & {}x & {}x\\\\".format(name, data["V"], data["E"], data["R"], data["PI"], data["TI"], data["SI"]))
        if data["V"] != "-": Vs.append(data["V"])
        if data["E"] != "-": Es.append(data["E"])
        if data["R"] != "-": Rs.append(data["R"])
        if data["PI"] != "-": PIs.append(data["PI"])
        if data["TI"] != "-": TIs.append(data["TI"])
        if data["SI"] != "-": SIs.append(data["SI"])
    print("Average & ({}, {}) & {} & {}x & {}x & {}x\\\\".format(
        arith_mean(Vs),
        arith_mean(Es),
        arith_mean(Rs),
        geo_mean(PIs),
        geo_mean(TIs),
        geo_mean(SIs)
    ))

def main():

    taint_graphs_dir = "exp/graphs/taint"
    valueflow_graphs_dir = "exp/graphs/valueflow"
    taint_results_dir = "exp/results/taint"
    valueflow_results_dir = "exp/results/valueflow"

    print_table("Taint Analysis", Taint, taint_graphs_dir, taint_results_dir)
    print_table("Value-Flow Analysis", Valueflow, valueflow_graphs_dir, valueflow_results_dir)

if __name__ == "__main__":
    main()
