#ifndef INFERENCE_LINEAR_SOLVER_BACKEND_H__
#define INFERENCE_LINEAR_SOLVER_BACKEND_H__

#include <util/exceptions.h>
#include "LinearObjective.h"
#include "LinearConstraints.h"
#include "Solution.h"
#include "VariableType.h"

class LinearSolverBackend {

public:

	struct Parameters {

		Parameters() :
				mipGap(0.0001),
				mipFocus(0),
				numThreads(0),
				verbose(false) {}

		// The relative optimality gap.
		double mipGap;

		// The MIP focus: 0 = balanced, 1 = feasible solutions, 2 = optimal 
		// solution, 3 = bound (not supported by all solver backends)
		unsigned int mipFocus;

		// The number of threads to be used. The default (0) leaves the decision 
		// to the solver.
		unsigned int numThreads;

		// Show the verbose output.
		bool verbose;
    };

	virtual ~LinearSolverBackend() {}

	/**
	 * Initialise the linear solver for the given type of variables.
	 *
	 * @param numVariables The number of variables in the problem.
	 * @param variableType The type of the variables (Continuous, Integer,
	 *                     Binary).
	 */
	virtual void initialize(
			unsigned int numVariables,
			VariableType variableType) = 0;

	/**
	 * Initialise the linear solver for the given type of variables.
	 *
	 * @param numVariables
	 *             The number of variables in the problem.
	 * 
	 * @param defaultVariableType
	 *             The default type of the variables (Continuous, Integer, 
	 *             Binary).
	 *
	 * @param specialVariableTypes
	 *             A map of variable numbers to variable types to override the 
	 *             default.
	 */
	virtual void initialize(
			unsigned int                                numVariables,
			VariableType                                defaultVariableType,
			const std::map<unsigned int, VariableType>& specialVariableTypes) = 0;

	/**
	 * Set the objective.
	 *
	 * @param objective A linear objective.
	 */
	virtual void setObjective(const LinearObjective& objective) = 0;

	/**
	 * Set the linear (in)equality constraints.
	 *
	 * @param constraints A set of linear constraints.
	 */
	virtual void setConstraints(const LinearConstraints& constraints) = 0;

	/**
	 * Add a single constraint.
	 *
	 * @param constraint A linear constraints.
	 */
	virtual void addConstraint(const LinearConstraint& constraint) = 0;

	/**
	 * Solve the problem.
	 *
	 * @param solution A solution object to write the solution to.
	 * @param value The optimal value of the objective.
	 * @param message A status message from the solver.
	 * @return true, if the optimal value was found.
	 */
	virtual bool solve(Solution& solution, std::string& message, const Parameters& parameters = Parameters()) = 0;

	virtual void dumpProblem(std::string filename) { UTIL_THROW_EXCEPTION(NotYetImplemented, "this solver does not supporting dumping"); }
};

class LinearSolverBackendException : public Exception {};

#endif // INFERENCE_LINEAR_SOLVER_BACKEND_H__

