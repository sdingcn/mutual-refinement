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
