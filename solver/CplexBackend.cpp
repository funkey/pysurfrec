#include <config.h>

#ifdef HAVE_CPLEX

#include <string>
#include <vector>

#include <ilcplex/ilocplex.h>

#include "LinearConstraints.h"
#include "QuadraticObjective.h"
#include "QuadraticSolverBackend.h"
#include "Sense.h"
#include "Solution.h"
#include "CplexBackend.h"
#include <util/Logger.h>

logger::LogChannel cplexlog("cplexlog", "[Cplex] ");

CplexBackend::CplexBackend() :
    model_(env_),
    x_(env_),
    c_(env_),
    obj_(env_),
    sol_(env_),
    firstRun_(true)
{
    LOG_DEBUG(cplexlog) << "constructing cplex solver" << std::endl;
}

CplexBackend::~CplexBackend() {
    LOG_DEBUG(cplexlog) << "destructing cplex solver..." << std::endl;
}

void
CplexBackend::initialize(
        unsigned int numVariables,
        VariableType variableType) {

    initialize(numVariables, variableType, std::map<unsigned int, VariableType>());
}

void
CplexBackend::initialize(
        unsigned int                                numVariables,
        VariableType                                defaultVariableType,
        const std::map<unsigned int, VariableType>& specialVariableTypes) {

    _numVariables = numVariables;

    // delete previous variables
    x_.clear();

    // add new variables to the model
    if (defaultVariableType == Binary) {
        LOG_USER(cplexlog) << "creating " << _numVariables << " binary variables" << std::endl;
        x_.add(IloNumVarArray(env_, _numVariables, 0, 1, ILOBOOL));
    } else if (defaultVariableType == Continuous) {
        LOG_USER(cplexlog) << "creating " << _numVariables << " continuous variables" << std::endl;
        x_.add(IloNumVarArray(env_, _numVariables, -IloInfinity, IloInfinity));
    } else if (defaultVariableType == Integer) {
        x_.add(IloNumVarArray(env_, _numVariables, -IloInfinity, IloInfinity, ILOINT));
    }

    // TODO: port me!
//    // handle special variable types
//    typedef std::map<unsigned int, VariableType>::const_iterator VarTypeIt;
//    for (VarTypeIt i = specialVariableTypes.begin(); i != specialVariableTypes.end(); i++) {

//        unsigned int v = i->first;
//        VariableType type = i->second;

//        char t = (type == Binary ? 'B' : (type == Integer ? 'I' : 'C'));
//        _variables[v].set(GRB_CharAttr_VType, t);
//    }
    LOG_USER(cplexlog) << "creating " << _numVariables << " ceofficients" << std::endl;
}

void
CplexBackend::setObjective(const LinearObjective& objective) {

    setObjective((QuadraticObjective)objective);
}

void
CplexBackend::setObjective(const QuadraticObjective& objective) {
    try {


       if(!firstRun_){
        model_.remove(obj_);
       }

  
        // set sense of objective
        if (objective.getSense() == Minimize)
            obj_ = IloMinimize(env_);
        else
            obj_ = IloMaximize(env_);
        

        // set the constant value of the objective
        obj_.setConstant(objective.getConstant());

        LOG_DEBUG(cplexlog) << "setting linear coefficients" << std::endl;

        for(size_t i = 0; i < _numVariables; i++)
        {
			if (objective.getCoefficients()[i] == std::numeric_limits<double>::infinity())
				obj_.setLinearCoef(x_[i], CPX_INFBOUND);
			else
				obj_.setLinearCoef(x_[i], objective.getCoefficients()[i]);
        }

        // set the quadratic coefficients for all pairs of variables
        LOG_DEBUG(cplexlog) << "setting quadratic coefficients" << std::endl;

        typedef std::map<std::pair<unsigned int, unsigned int>, double>::const_iterator QuadCoefIt;
        for (QuadCoefIt i = objective.getQuadraticCoefficients().begin(); i != objective.getQuadraticCoefficients().end(); i++) {

            const std::pair<unsigned int, unsigned int>& variables = i->first;
            float value = i->second;

            if (value != 0)
                obj_.setQuadCoef(x_[variables.first], x_[variables.second], value);
        }
        if(true || firstRun_){
            model_.add(obj_);
            firstRun_ = false;
        }


    } catch (IloCplex::Exception e) {

        LOG_ERROR(cplexlog) << "CPLEX error: " << e.getMessage() << std::endl;
    }
}

void
CplexBackend::setConstraints(const LinearConstraints& constraints) {

    // remove previous constraints
    for (ConstraintVector::iterator constraint = _constraints.begin(); constraint != _constraints.end(); constraint++)
        model_.remove(*constraint);
    _constraints.clear();

    // allocate memory for new constraints
    _constraints.reserve(constraints.size());

    try {
        LOG_USER(cplexlog) << "setting " << constraints.size() << " constraints" << std::endl;

        IloExtractableArray cplex_constraints(env_);
        for (LinearConstraints::const_iterator constraint = constraints.begin(); constraint != constraints.end(); constraint++) {
            IloRange linearConstraint = createConstraint(*constraint);
            _constraints.push_back(linearConstraint);
            cplex_constraints.add(linearConstraint);
        }

        // add all constraints as batch to the model
        model_.add(cplex_constraints);

    } catch (IloCplex::Exception e) {

        LOG_ERROR(cplexlog) << "error: " << e.getMessage() << std::endl;
    }
}

void
CplexBackend::addConstraint(const LinearConstraint& constraint) {

    try {
        LOG_ALL(cplexlog) << "adding a constraint" << std::endl;

        // add to the model
        _constraints.push_back(model_.add(createConstraint(constraint)));

    } catch (IloCplex::Exception e) {

        LOG_ERROR(cplexlog) << "error: " << e.getMessage() << std::endl;
    }
}

IloRange
CplexBackend::createConstraint(const LinearConstraint& constraint) {


    // create the lhs expression
    IloExpr linearExpr(env_);

    // set the coefficients
    typedef std::map<unsigned int, double>::const_iterator CoefIt;
    for (CoefIt pair = constraint.getCoefficients().begin(); pair != constraint.getCoefficients().end(); pair++)
    {
        linearExpr.setLinearCoef(x_[pair->first], pair->second);
    }



    switch(constraint.getRelation())
    {
        case LessEqual:
            return IloRange(env_, linearExpr, constraint.getValue());
            break;
        case GreaterEqual:
            return IloRange(env_, constraint.getValue(), linearExpr);
            break;
        default:
        //case Equal:
            return IloRange(env_,  constraint.getValue(), linearExpr, constraint.getValue());
            break;
    }
}

bool
CplexBackend::solve(Solution& x,/* double& value, */ std::string& msg, const LinearSolverBackend::Parameters& paramter) {

    try {
        cplex_ = IloCplex(model_);
        setVerbose(parameter.verbose);

        setMIPGap(parameter.mipGap);

        if (parameter.mipFocus <= 3)
            setMIPFocus(parameter.mipFocus);
        else
            LOG_ERROR(cplexlog) << "Invalid value for MIP focus!" << std::endl;

        setNumThreads(parameter.numThreads);

        if(!cplex_.solve()) {
           LOG_USER(cplexlog) << "failed to optimize. " << cplex_.getStatus() << std::endl;
           msg = "Optimal solution *NOT* found";
           return false;
        }
        else
            msg = "Optimal solution found";

        // extract solution
        cplex_.getValues(sol_, x_);
        x.resize(_numVariables);
        for (unsigned int i = 0; i < _numVariables; i++)
            x[i] = sol_[i];

        // get current value of the objective
        const double value = cplex_.getObjValue();
        x.setValue(value);
        model_.remove(obj_);
        //cplex_.clearModel();

    } catch (IloCplex::Exception& e) {

        LOG_ERROR(cplexlog) << "error: " << e.getMessage() << std::endl;

        msg = e.getMessage();

        return false;
    }

    return true;
}

void
CplexBackend::setMIPGap(double gap) {
     cplex_.setParam(IloCplex::EpGap, gap);
}

A
void
CplexBackend::setMIPFocus(unsigned int focus) {
    /*
     * GUROBI and CPLEX have the same meaning for the values of the MIPFocus and MIPEmphasis parameter:
     *
     * GUROBI docs:
     * If you are more interested in finding feasible solutions quickly, you can select MIPFocus=1.
     * If you believe the solver is having no trouble finding good quality solutions,
     * and wish to focus more attention on proving optimality, select MIPFocus=2.
     * If the best objective bound is moving very slowly (or not at all), you may want to try MIPFocus=3
     * to focus on the bound.
     *
     * CPLEX params:
     * switch(focus) {
        case MIP_EMPHASIS_BALANCED:
            cplex_.setParam(IloCplex::MIPEmphasis, 0);
            break;
        case  MIP_EMPHASIS_FEASIBILITY:
            cplex_.setParam(IloCplex::MIPEmphasis, 1);
            break;
        case MIP_EMPHASIS_OPTIMALITY:
            cplex_.setParam(IloCplex::MIPEmphasis, 2);
            break;
        case MIP_EMPHASIS_BESTBOUND:
            cplex_.setParam(IloCplex::MIPEmphasis, 3);
            break;
        }
     */

    cplex_.setParam(IloCplex::MIPEmphasis, focus);
}

void
CplexBackend::setNumThreads(unsigned int numThreads) {
    cplex_.setParam(IloCplex::Threads, numThreads);
}

void
CplexBackend::setVerbose(bool verbose) {

    // setup GRB environment
    if (verbose)
    {
        cplex_.setParam(IloCplex::MIPDisplay, 1);
        cplex_.setParam(IloCplex::SimDisplay, 1);
        cplex_.setParam(IloCplex::SiftDisplay, 1);
    }
    else
    {
        cplex_.setParam(IloCplex::MIPDisplay, 0);
        cplex_.setParam(IloCplex::SimDisplay, 0);
        cplex_.setParam(IloCplex::SiftDisplay, 0);
    }
}





#endif // HAVE_CPLEX
