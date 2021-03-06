#ifndef SCIP_SOLVER_H__
#define SCIP_SOLVER_H__

#ifdef HAVE_SCIP

#include <string>

#include <scip/scip.h>

#include "LinearConstraints.h"
#include "QuadraticObjective.h"
#include "QuadraticSolverBackend.h"
#include "Sense.h"
#include "Solution.h"

/**
 * Scip interface to solve the following (integer) quadratic program:
 *
 * min  <a,x> + xQx
 * s.t. Ax  == b
 *      Cx  <= d
 *      optionally: x_i \in {0,1} for all i
 *
 * Where (A,b) describes all linear equality constraints, (C,d) all linear
 * inequality constraints and x is the solution vector. a is a real-valued
 * vector denoting the coefficients of the objective and Q a PSD matrix giving
 * the quadratic coefficients of the objective.
 */
class ScipBackend : public QuadraticSolverBackend {

public:

	ScipBackend();

	virtual ~ScipBackend();

	///////////////////////////////////
	// solver backend implementation //
	///////////////////////////////////

	void initialize(
			unsigned int numVariables,
			VariableType variableType);

	void initialize(
			unsigned int                                numVariables,
			VariableType                                defaultVariableType,
			const std::map<unsigned int, VariableType>& specialVariableTypes);

	void setObjective(const LinearObjective& objective);

	void setObjective(const QuadraticObjective& objective);

	void setConstraints(const LinearConstraints& constraints);

	void addConstraint(const LinearConstraint& constraint);

	bool solve(Solution& solution, std::string& message, const LinearSolverBackend::Parameters& parameters = LinearSolverBackend::Parameters());

private:

	//////////////
	// internal //
	//////////////

	/**
	 * Enable solver output.
	 */
	void setVerbose(bool verbose);

	void freeVariables();

	void freeConstraints();

	SCIP_VARTYPE scipVarType(VariableType type, double& lb, double& ub);

	// size of a and x
	unsigned int _numVariables;

	SCIP* _scip;

	std::vector<SCIP_VAR*> _variables;

	std::vector<SCIP_CONS*> _constraints;
};

#endif // HAVE_SCIP

#endif // SCIP_SOLVER_H__



