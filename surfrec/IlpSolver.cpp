#include "IlpSolver.h"
#include <solver/SolverFactory.h>
#include <util/Logger.h>
#include <util/helpers.hpp>

logger::LogChannel ilpsolverlog("ilpsolverlog", "[IlpSolver] ");

IlpSolver::IlpSolver(std::size_t num_nodes, std::size_t num_edges, int num_levels, int max_gradient) :
	_level_costs(_graph),
	_num_nodes(0),
	_num_edges(0),
	_num_levels(num_levels),
	_max_gradient(max_gradient) {

	_graph.reserveNode(num_nodes);
	_graph.reserveEdge(num_edges);

	SolverFactory factory;
	_solver = std::unique_ptr<LinearSolverBackend>(factory.createLinearSolverBackend());
}

IlpSolver::NodeId
IlpSolver::add_nodes(std::size_t num_nodes) {

	if (num_nodes < 1)
		UTIL_THROW_EXCEPTION(
				UsageError,
				"at least one node has to be added with a call to add_nodes");

	_num_nodes += num_nodes;

	NodeId first = _graph.id(_graph.addNode());
	num_nodes--;

	for (std::size_t i = 0; i < num_nodes; i++)
		_graph.addNode();

	return first;
}

void
IlpSolver::add_edge(NodeId u, NodeId v) {

	_graph.addEdge(_graph.nodeFromId(u), _graph.nodeFromId(v));
	_num_edges++;
}

void
IlpSolver::set_level_costs(NodeId n, const std::vector<double>& costs) {

	_level_costs[_graph.nodeFromId(n)] = costs;
}

double
IlpSolver::min_surface() {

	std::size_t num_vars = _num_nodes*_num_levels;

	LOG_DEBUG(ilpsolverlog) << "creating objective for " << num_vars << " binary variables" << std::endl;
	LinearObjective objective(num_vars);

	LOG_DEBUG(ilpsolverlog) << "setting objective coefficients" << std::endl;
	for (GraphType::NodeIt n(_graph); n != lemon::INVALID; ++n) {

		double sum = 0;

		for (int l = 0; l < _num_levels; l++) {

			std::size_t var_num = _graph.id(n)*_num_levels + l;
			double costs = _level_costs[n][l];
			double accumulated_costs = costs - sum;
			sum += accumulated_costs;

			objective.setCoefficient(var_num, accumulated_costs);
		}
	}

	LinearConstraints constraints;

	// pick at least one constraints
	LOG_DEBUG(ilpsolverlog) << "adding indicator constraints" << std::endl;
	for (GraphType::NodeIt n(_graph); n != lemon::INVALID; ++n) {

		LinearConstraint indicator;

		for (int l = 0; l < _num_levels; l++) {

			std::size_t var_num = _graph.id(n)*_num_levels + l;
			indicator.setCoefficient(var_num,  1.0);
		}

		indicator.setRelation(GreaterEqual);
		indicator.setValue(1.0);
		constraints.add(indicator);
	}

	// column inclusion constraints
	LOG_DEBUG(ilpsolverlog) << "adding column inclusion constraints" << std::endl;
	for (GraphType::NodeIt n(_graph); n != lemon::INVALID; ++n) {

		for (int l = 1; l < _num_levels; l++) {

			std::size_t upper_var_num = _graph.id(n)*_num_levels + l;
			std::size_t lower_var_num = _graph.id(n)*_num_levels + l - 1;

			LinearConstraint inclusion;
			inclusion.setCoefficient(upper_var_num,  1.0);
			inclusion.setCoefficient(lower_var_num, -1.0);
			inclusion.setRelation(LessEqual);
			inclusion.setValue(0.0);

			constraints.add(inclusion);
		}
	}

	// gradient constraints
	LOG_DEBUG(ilpsolverlog) << "adding gradient constraints" << std::endl;
	for (GraphType::EdgeIt e(_graph); e != lemon::INVALID; ++e) {

		GraphType::Node u = _graph.u(e);
		GraphType::Node v = _graph.v(e);

		for (auto& p : { std::make_pair(u, v), std::make_pair(v, u) }) {

			GraphType::Node u_ = p.first;
			GraphType::Node v_ = p.second;

			for (int l = _max_gradient; l < _num_levels; l++) {

				std::size_t upper_var_num = _graph.id(u_)*_num_levels + l;
				std::size_t lower_var_num = _graph.id(v_)*_num_levels + l - _max_gradient;

				LinearConstraint inclusion;
				inclusion.setCoefficient(upper_var_num,  1.0);
				inclusion.setCoefficient(lower_var_num, -1.0);
				inclusion.setRelation(LessEqual);
				inclusion.setValue(0.0);

				constraints.add(inclusion);
			}
		}
	}

	LOG_DEBUG(ilpsolverlog) << "initialize solver" << std::endl;
	_solver->initialize(num_vars, Binary);
	LOG_DEBUG(ilpsolverlog) << "setting objective" << std::endl;
	_solver->setObjective(objective);
	LOG_DEBUG(ilpsolverlog) << "setting setting constraints" << std::endl;
	_solver->setConstraints(constraints);

	LOG_ALL(ilpsolverlog) << objective << std::endl;
	for (auto c : constraints)
		LOG_ALL(ilpsolverlog) << c << std::endl;

	LOG_DEBUG(ilpsolverlog) << "solving" << std::endl;
	std::string message;
	if (!_solver->solve(_solution, message))
		UTIL_THROW_EXCEPTION(
				LinearSolverBackendException,
				"linear program could not be solved: " << message);

	LOG_ALL(ilpsolverlog) << _solution.getVector() << std::endl;

	return _solution.getValue();
}

int
IlpSolver::level(NodeId n) {

	std::size_t lowest_var_num = n*_num_levels;

	if (_solution[lowest_var_num] < 0.5)
		UTIL_THROW_EXCEPTION(
				Exception,
				"no level was selected for node " << n);

	for (int l = 1; l < _num_levels; l++)
		if (_solution[lowest_var_num + l] < 0.5)
			return l-1;

	return _num_levels - 1;
}