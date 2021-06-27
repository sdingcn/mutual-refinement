import sys
import subprocess

def run_and_get_output(commands):
    return subprocess.run(commands, stdout = subprocess.PIPE, stderr = subprocess.STDOUT, universal_newlines = True).stdout

def main(dotfile_path):
    reachable_pairs = run_and_get_output(['./LinConjReach', '1', '1', '1'])
    r_pairs = []
    m = 0
    n = 0
    for line in reachable_pairs.splitlines():
        if 'ALLPAIRS' in line:
            m += 1
            garbage, v1, v2 = line.split()
            output = run_and_get_output(['./LinConjReach', '2', str(v1), str(v2)])
            with open('tmp', 'w') as g:
                for l in output.splitlines():
                    if 'INFO' in l:
                        g.write(l.split()[1] + '\n')
            res = run_and_get_output(['../boolean-program-reachability/main', 'pa-ssss', dotfile_path, 'tmp', str(v1), str(v2)])
            print(res)
            if res.strip() == 'possibly reachable':
                n += 1
    print(m, n)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('Usage: python3 {} <dotfile-path>'.format(sys.argv[0]))
    else:
        main(sys.argv[1])

