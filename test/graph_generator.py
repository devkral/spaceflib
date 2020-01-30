#! /usr/bin/env python3

import sys, os
path = os.path.join(os.path.dirname(__file__), "thirdparty", "laudahn", "graph_server")
sys.path.insert(0, path)
import fake_graphs_r
import graph2r

graphs = {
    "default": [
        ('02a', 70, True),
        ('02b', 80, False),
        ('star', [('02a', 50, False), ('02b', 69, True), ('02a', 78, False), ('02b', 88, True)]),
        ('clique', [('02a', 45, False), ('02a', 68, True), ('02a', 77, True)]),
        ('bic', [('02a', 75, True), ('02a', 67, True)], [('02b', 30, True), ('02a', 79, False)]),
    ],
    "star": [(
        "star",
        [
            ('02a', 70, True),
            ('02b', 80, False),
            ('star', [('02a', 50, False), ('02b', 69, True), ('02a', 78, False), ('02b', 88, True)]),
            ('clique', [('02a', 45, False), ('02a', 68, True), ('02a', 77, True)]),
            ('bic', [('02a', 75, True), ('02a', 67, True)], [('02b', 30, True), ('02a', 79, False)]),
        ],
    )],
    "4000": [(
        "star",
        [
            ('02a', 2000, True),
            ('02b', 2000, False),
        ]
    )],
    "8000": [(
        "star",
        [
            (
                "bic",
                [
                    ('02a', 2000, True),
                    ('02b', 2000, False),
                ],
                [('02a', 2000, False)],
            ),
            ('02b', 2000, True),
        ],
    )],
    "160000": [(
        "clique",
        [
            ('02a', 19980, True),
            ('02b', 20001, False),
            (
                "bic",
                [
                    ('02a', 20, True),
                    ('02b', 20, False),
                ],
                [('02a', 10000, False)]
            ),
            ('02a', 10000, False),
            ('02b', 20000, True),
            (
                "star",
                [
                    ('02a', 20001, True),
                    ('02b', 20000, False),
                    ('02a', 20000, False),
                    ('02b', 20000, True),
                ]
            ),
        ]
    )],
    "320000": [(
        "star",
        [
            ('02a', 40001, True),
            ('02b', 40000, False),
            ('02a', 40000, False),
            ('02b', 40000, True),
            (
                "clique",
                [
                    ('02a', 40000, True),
                    ('02b', 40000, False),
                    ('02a', 40000, False),
                    ('02b', 40000, True),
                ],
            ),
        ],
    )],
}

if __name__ == "__main__":
    if len(sys.argv) != 3 or sys.argv[1] not in graphs:
        print("Usage: <graphname> <outputname>")
        print("Graphs:", *graphs.keys())
        exit(0)
    graph = graphs[sys.argv[1]]
    if isinstance(graph, (list, tuple)):
        graph = fake_graphs_r.CombinedFakeGraph(graph)
    if not fake_graphs_r.checkIfInvalid(graph, True):
        filen = sys.argv[2]
        if not filen.endswith(".dot"):
            filen += ".dot"
        dotStr = graph2r.toDotString(graph)
        with open(filen, "w") as f:
            f.write(dotStr)
