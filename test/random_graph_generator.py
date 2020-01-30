#! /usr/bin/env python3

import sys
import os
import random

random.seed(b'l\x1dau\x0edx\xa7\xeb\xb7hn:(\x86z\x18', 2)


output_ranges = [0, *[1]*600, *[2]*300, *[3]*87, *[4]*10, 5, 6]

if __name__ == "__main__":
    if len(sys.argv) != 4 or not sys.argv[1].isdigit() or not sys.argv[2].isdigit():
        print("Usage: <nodes> <edges> <outputname>")
        exit(0)
    filen = sys.argv[3]
    if not filen.endswith(".dot"):
        filen += ".dot"
    nodes = int(sys.argv[1])
    edges = int(sys.argv[2])
    whitespaces = " "*(len(str(nodes))+len(str(edges))+3)
    edges_old = edges
    node_pool = list(range(2, nodes+1))
    random.shuffle(node_pool)

    with open(filen, "w") as f:
        f.write("graph graph_%s {\n" % whitespaces)
        for i in range(1, nodes+1):
            f.write("  {};\n".format(i))
        for i in node_pool:
            if edges == 0:
                break
            amount = min(random.choice(output_ranges), i-1)
            amount = min(amount, edges)
            edges -= amount
            sample = random.sample(range(1, i), k=amount)
            for l in sample:
                f.write("  {}--{};\n".format(l, i))
        f.write("}\n")
        f.seek(12) # first space
        f.write("{}n_{}e".format(nodes, edges_old-edges))
    print("edges:", edges_old-edges)
