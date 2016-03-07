#ifndef PYSURFREC_SURFREC_ILP_SOLVER_GRAPH_H__
#define PYSURFREC_SURFREC_ILP_SOLVER_GRAPH_H__

#include <lemon/list_graph.h>
#include <solver/SolverFactory.h>
#include <util/helpers.hpp>

/**
 * An ILP solver for the surface reconstruction problem. Formulates the 
 * objective and constraints on the surface directly as an ILP.
 */
class IlpSolver {

public:

	typedef std::size_t NodeId;

	struct Parameters {

		Parameters() : enforce_zero_minimum(false), num_neighbors(-1), num_threads(0) {}

		/**
		 * If set to true, the ILP ensures that every minimum has a value of 
		 * zero (as it should be for distance maps).
		 */
		bool enforce_zero_minimum;

		/**
		 * The number of neighbors for a regular node inside the grid. Needs to 
		 * be set if enforce_zero_minimum to properly handle nodes at the 
		 * boundary.
		 */
		int num_neighbors;

		/**
		 * The number of threads to use for inference.
		 */
		int num_threads;

		/**
		 * Enable verbose output of the ILP solver.
		 */
		bool verbose;
	};

	/**
	 * Create a new IlpSolver solver for the given estimated number 
	 * of nodes and edges (of the original graph). More nodes and edges can be 
	 * added later, but might require re-allocation.
	 *
	 * @param num_nodes
	 *              The estimated number of nodes.
	 * @param num_edges
	 *              The estimated number of edges.
	 * @param num_levels
	 *              The number of levels in each column, i.e., the maximal 
	 *              height of the surface.
	 * @param max_gradient
	 *              The maximal gradient that the surface can have between two 
	 *              neighboring nodes.
	 */
	IlpSolver(std::size_t num_nodes, std::size_t num_edges, int num_levels, int max_gradient);

	/**
	 * Add n nodes to the graph, return the index to the first one.
	 */
	NodeId add_nodes(std::size_t num_nodes);

	/**
	 * Set the costs for passing the surface through the different levels of a 
	 * column.
	 */
	void set_level_costs(NodeId n, const std::vector<double>& costs);

	/**
	 * Add a neighborhood edge between nodes u and v.
	 */
	void add_edge(NodeId u, NodeId v);

	/**
	 * Find the cost-minimal surface.
	 */
	double min_surface(const Parameters& parameters);

	/**
	 * Return the level where the found surface passes through the column of 
	 * node n.
	 */
	int level(NodeId n);

	/**
	 * Dump the ILP into a text file.
	 */
	void dump_ilp(std::string filename) { _solver->dumpProblem(filename); }

private:

	typedef lemon::ListGraph GraphType;

	GraphType _graph;
	GraphType::NodeMap<std::vector<double>> _level_costs;

	std::size_t _num_nodes;
	std::size_t _num_edges;
	int _num_levels;
	int _max_gradient;

	std::unique_ptr<LinearSolverBackend> _solver;

	Solution _solution;
};

#endif // PYSURFREC_SURFREC_ILP_SOLVER_GRAPH_H__
