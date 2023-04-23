import sys

def check(cond):
    if not cond:
        sys.exit('Abruption.')

def strip_list(lst):
    return [item.strip() for item in lst]

def map_list(fun, lst):
    return [fun(item) for item in lst]

def zip_list(lst1, lst2):
    check(len(lst1) == len(lst2))
    n = len(lst1)
    return [(lst1[i], lst2[i]) for i in range(n)]

def get_col(table, index):
    numbers = []
    for row in table:
        try:
            numbers.append(float(row[index]))
        except ValueError:
            pass
    return numbers

def arith_mean(lst):
    check(len(lst) >= 1)
    n = len(lst)
    return sum(lst) / n

def geo_mean(lst):
    check(len(lst) >= 1)
    n = len(lst)
    v = 1
    for i in range(n):
        v *= lst[i]
    return v ** (1 / n)

def main():
    A, B, C, D, E = [], [], [], [], []
    cur = A
    with open('report.txt', 'r') as f:
        lines = f.readlines()
        for line in lines:
            line = line.strip()
            if line == '=== 2a ===':
                cur = A
            elif line == '=== 2b ===':
                cur = B
            elif line == '=== 3a ===':
                cur = C
            elif line == '=== 3b ===':
                cur = D
            elif line == '=== 4 ===':
                cur = E
            else:
                row = strip_list(line[:-2].split('&'))
                if '-' not in row:
                    cur.append(row)
    print(geo_mean(map_list(lambda bm: bm[0] / bm[1], zip_list(get_col(C, 2), get_col(C, 3)))) - 1)
    print(geo_mean(map_list(lambda bm: bm[0] / bm[1], zip_list(get_col(D, 2), get_col(D, 3)))) - 1)
    print(arith_mean(get_col(C, 1)))
    print(geo_mean(map_list(lambda t: t[1] / t[0], zip_list(get_col(C, 4), get_col(C, 5)))))
    print(geo_mean(map_list(lambda s: s[1] / s[0], zip_list(get_col(C, 6), get_col(C, 7)))))
    print(arith_mean(get_col(D, 1)))
    print(geo_mean(map_list(lambda t: t[1] / t[0], zip_list(get_col(D, 4), get_col(D, 5)))))
    print(geo_mean(map_list(lambda s: s[1] / s[0], zip_list(get_col(D, 6), get_col(D, 7)))))
    print(geo_mean(map_list(lambda ml: ml[0] / ml[1], zip_list(get_col(E, 1), get_col(E, 2)))) - 1)
    print(geo_mean(map_list(lambda ml: ml[1] / ml[0], zip_list(get_col(E, 7), get_col(E, 8)))) - 1)

if __name__ == '__main__':
    main()
