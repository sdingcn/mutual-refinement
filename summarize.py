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

def get_result(fpath):
    with open(fpath, "r") as f:
        content = f.read()
        if "Resource Consumption" in content:
            n, _, t, s = content.splitlines()
            n = int(n.strip())
            t = float(t.split()[-1].strip())
            s = int(s.split()[-1].strip())
            return (n, t, s)
        else:
            return None

def main():
   taintpath = "exp/results/taint"
   valueflowpath = "exp/results/valueflow"
   print("*** Taint Analysis ***")
   print("Benchmark".ljust(30) + "Naive".ljust(30) + "Refine".ljust(30))
   for taint in T:
       c1 = taint.ljust(30)
       c2 = str(get_result(os.path.join(taintpath, "naive-{}.result".format(taint)))).ljust(30)
       c3 = str(get_result(os.path.join(taintpath, "refine-{}.result".format(taint)))).ljust(30)
       print(c1 + c2 + c3)
   print("*** Value-Flow Analysis ***")
   print("Benchmark".ljust(30) + "Naive".ljust(30) + "Refine".ljust(30))
   for valueflow in V:
       c1 = valueflow.ljust(30)
       c2 = str(get_result(os.path.join(valueflowpath, "naive-{}.result".format(valueflow)))).ljust(30)
       c3 = str(get_result(os.path.join(valueflowpath, "refine-{}.result".format(valueflow)))).ljust(30)
       print(c1 + c2 + c3)

if __name__ == "__main__":
    main()
