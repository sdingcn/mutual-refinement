import os
import os.path
import sys

T = [
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

V = [
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

def geo_mean(lst):
    n = len(lst)
    x = 1
    for v in lst:
        x *= v
    return round(x ** (1 / n), 3)

def get_result(fpath):
    result = {}
    with open(fpath, "r") as f:
        content = f.read()
        lines = list(map(lambda l : l.strip(), content.splitlines()))
        for line in lines:
            if line.startswith("Number of Refinement Iterations"):
                result["iter"] = int(line.split()[-1].strip())
            elif line.startswith("Number of Reachable Pairs"):
                result["reach"] = int(line.split()[-1].strip())
            elif line.startswith("Total Time"):
                result["time"] = float(line.split()[-1].strip())
            elif line.startswith("Peak Space"):
                result["space"] = float(line.split()[-1].strip())
    return result

def main(option):
    
    taint_path = "exp/results/taint"
    taint_results = {}
    for taint in T:
        taint_results[taint] = {
            "naive": get_result(os.path.join(taint_path, "naive-{}.result".format(taint))),
            "refine": get_result(os.path.join(taint_path, "refine-{}.result".format(taint)))
        }
    
    valueflow_path = "exp/results/valueflow"
    valueflow_results = {}
    for valueflow in V:
        valueflow_results[valueflow] = {
            "naive": get_result(os.path.join(valueflow_path, "naive-{}.result".format(valueflow))),
            "refine": get_result(os.path.join(valueflow_path, "refine-{}.result".format(valueflow)))
        }
    
    if option == "prettyprint":
        
        print("*** Taint Analysis ***")
        for taint in T:
            print(taint)
            for k, v in taint_results[taint].items():
                print("\t", k, ":", v)
        
        print("*** Value-Flow Analysis ***")
        for valueflow in V:
            print(valueflow)
            for k, v in valueflow_results[valueflow].items():
                print("\t", k, ":", v)
    
    elif option == "latextable":
        
        print("*** Taint Analysis ***")
        print("Benchmark & Number of Reachable Pairs Ratio & Time Ratio & Space Ratio\\\\")
        reaches = []
        times = []
        spaces = []
        for taint in T:
            if len(taint_results[taint]["naive"]) != 0 and len(taint_results[taint]["refine"]) != 0:
                reach = round(taint_results[taint]["refine"]["reach"] / taint_results[taint]["naive"]["reach"], 3)
                time = round(taint_results[taint]["refine"]["time"] / taint_results[taint]["naive"]["time"], 3)
                space = round(taint_results[taint]["refine"]["space"] / taint_results[taint]["naive"]["space"], 3)
                print("{} & {} & {} & {}\\\\".format(taint, reach, time, space))
                reaches.append(reach)
                times.append(time)
                spaces.append(space)
        print("Mean & {} & {} & {}\\\\".format(geo_mean(reaches), geo_mean(times), geo_mean(spaces)))
        
        print("*** Value-Flow Analysis ***")
        print("Benchmark & Number of Reachable Pairs Ratio & Time Ratio & Space Ratio")
        reaches = []
        times = []
        spaces = []
        for valueflow in V:
            if len(valueflow_results[valueflow]["naive"]) != 0 and len(valueflow_results[valueflow]["refine"]) != 0:
                reach = round(valueflow_results[valueflow]["refine"]["reach"] / valueflow_results[valueflow]["naive"]["reach"], 3)
                time = round(valueflow_results[valueflow]["refine"]["time"] / valueflow_results[valueflow]["naive"]["time"], 3)
                space = round(valueflow_results[valueflow]["refine"]["space"] / valueflow_results[valueflow]["naive"]["space"], 3)
                print("{} & {} & {} & {}\\\\".format(valueflow, reach, time, space))
                reaches.append(reach)
                times.append(time)
                spaces.append(space)
        print("Mean & {} & {} & {}\\\\".format(geo_mean(reaches), geo_mean(times), geo_mean(spaces)))

if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit("Usage: python3 {} <prettyprint/latextable>".format(sys.argv[0]))
    main(sys.argv[1])
