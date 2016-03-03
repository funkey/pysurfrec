# make sure surfrec.so is can be found by adjusting your PYTHONPATH

import surfrec
import random
from timeit import timeit

def test():

    num_nodes = 1000
    num_levels = 20
    max_gradient = 1

    #surfrec.setLogLevel(surfrec.LogLevel.All)

    print "Instantiating solver"
    s = surfrec.IlpSolver(num_nodes, num_nodes - 1, num_levels, max_gradient)

    print "Adding nodes"
    first = s.add_nodes(num_nodes)
    nodes = range(first, first + num_nodes)

    print "Adding edges"
    for i in range(num_nodes - 1):
        s.add_edge(nodes[i], nodes[i+1])

    print "Adding level costs"
    for i in range(num_nodes):
        costs = surfrec.ColumnCosts(num_levels)
        for j in range(num_levels):
            costs[j] = random.random();
        s.set_level_costs(nodes[i], costs);

    print "Solving"
    value = s.min_surface()

    print "found levels: " + str([ s.level(n) for n in nodes ])
    print "surface costs : " + str(value)

if __name__ == "__main__":

    random.seed(123)
    print "Runtime (s): " + str(timeit("test()", setup = "from __main__ import test", number = 1))
