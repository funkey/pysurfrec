# -*- encoding:utf-8 -*-

"""
maxflow
=======

This module computes the max-flow/min-cut of a graph by solving a linear
program. The interface is the same as the ``maxflow`` package found at
https://github.com/pmneila/PyMaxflow.

Example (from PyMaxflow):

>>> import maxflow
>>> g = maxflow.Graph[int](2, 2)
>>> g.add_nodes(2)
0
>>> g.add_edge(0, 1, 1, 2)
>>> g.add_tedge(0, 2, 5)
>>> g.add_tedge(1, 9, 4)
>>> g.maxflow()
8
>>> g.get_segments()
array([ True, False], dtype=bool)
"""

import numpy as np
from . import _maxflow

class GraphInt:

	def __init__(self, node_num_max, edge_num_max):

		self.__graph = _maxflow.GraphInt(node_num_max, edge_num_max)

	def add_nodes(self, n):

		first = self.__graph.add_nodes(n)
		return np.arange(first, first + n)

	def add_edge(self, u, v, cap, rev_cap):

		self.__graph.add_edge(u, v, cap, rev_cap)

	def add_tedge(self, n, cap_source, cap_sink):

		self.__graph.add_tedge(n, cap_source, cap_sink)

	def maxflow(self):

		return self.__graph.maxflow()

	def get_segment(self, n):

		return self.__graph.get_segment(n)

	def dump_lp(self, filename):

		return self.__graph.dump_lp(filename)

class GraphFloat:

	def __init__(self, node_num_max, edge_num_max):

		self.__graph = _maxflow.GraphFloat(node_num_max, edge_num_max)

	def add_nodes(self, n):

		first = self.__graph.add_nodes(n)
		return np.arange(first, first + n)

	def add_edge(self, u, v, cap, rev_cap):

		self.__graph.add_edge(u, v, cap, rev_cap)

	def add_tedge(self, n, cap_source, cap_sink):

		self.__graph.add_tedge(n, cap_source, cap_sink)

	def maxflow(self):

		return self.__graph.maxflow()

	def get_segment(self, n):

		return self.__graph.get_segment(n)

	def dump_lp(self, filename):

		return self.__graph.dump_lp(filename)

Graph = {int: GraphInt, float: GraphFloat}

__all__ = ['Graph', "GraphInt", "GraphFloat", "np", "_maxflow"]
