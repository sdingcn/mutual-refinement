import sys

if __name__ == '__main__':
    if len(sys.argv) != 2:
        sys.exit('Usage: python3 {} <graph-file>'.format(sys.argv[0]))
    with open(sys.argv[1], 'r') as f:
        lines = f.readlines()
    cleaned_lines = []
    for line in lines:
        if '->' in line:
            cleaned_lines.append(line)
    with open(sys.argv[1], 'w') as f:
        for line in cleaned_lines:
            f.write(line)
