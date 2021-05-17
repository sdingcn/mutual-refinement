import subprocess

def run_and_get_output(commands: list) -> str:
    return subprocess.run(commands, stdout = subprocess.PIPE, stderr = subprocess.STDOUT, universal_newlines = True).stdout

def get_time(s: str) -> float:
    return float(s.split()[-1])

def main():
    benchmarks = ['lcl-exp/taint/normal/fakebanker.dot',
            'lcl-exp/taint/normal/faketaobao.dot',
            'lcl-exp/taint/normal/jollyserv.dot',
            'lcl-exp/taint/normal/loozfon.dot',
            'lcl-exp/taint/normal/uranai.dot'
            ]
    for fpath in benchmarks:
        t = []
        for i in range(10):
            t.append(get_time(run_and_get_output(['./main', fpath])))
        print('Benchmark: {} Time: {}'.format(fpath, sum(t)/len(t)))

if __name__ == '__main__':
    main()
