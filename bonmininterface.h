#ifndef BONMININTERFACE_H
#define BONMININTERFACE_H

#include "BonTMINLP.hpp"
#include <QVector>
#include <tr1/memory>

class Runner;
class RealVariable;
class BinaryVariable;
class Constraint;

using namespace Ipopt;
using namespace Bonmin;
using std::tr1::shared_ptr;

class BonminInterface : public TMINLP
{
private:
    Runner *p_runner;

    QVector<shared_ptr<RealVariable> > m_vars_real;
    QVector<shared_ptr<BinaryVariable> > m_vars_binary;
    QVector<shared_ptr<Constraint> > m_cons;

    QVector<double> m_grad_f;   // calculated values for df/dx
    QVector<double> m_jac_g;    // calculated values for dc/dx
    QVector<double> m_x_real;   // the variable values where the gradents were calculated
    QVector<int> m_x_binary;

    double m_pertubation;


    /**
     * @brief Checks if the variable values in x are the same as in the Modelc
     *
     * @param x
     * @return bool
     */
    bool newVariableValues(Index n, const Number *x);
    double getPerturbationReal(shared_ptr<RealVariable> var);
    int getPerturbationBinary(shared_ptr<BinaryVariable> var);
    void calculateGradients(Index n, const Number *x);
    bool gradientsAreUpdated(Index n, const Number *x);

public:
    BonminInterface(Runner *r);

    // overloaded functions specific to a TMINLP


     /**
      * @brief Pass the type of the variables (INTEGER, BINARY, CONTINUOUS) to the optimizer.
      *
      * @param n size of var_types (has to be equal to the number of variables in the problem)
      * @param var_types types of the variables (has to be filled by function).
      * @return bool
      */
     virtual bool get_variables_types(Index n, VariableType* var_types);


    /**
     * @brief Pass info about linear and nonlinear variables.
     *
     * @param n
     * @param var_types
     * @return bool
     */
    virtual bool get_variables_linearity(Index n, Ipopt::TNLP::LinearityType* var_types);


    /**
     * @brief Pass the type of the constraints (LINEAR, NON_LINEAR) to the optimizer.
     *
     * @param m size of const_types (has to be equal to the number of constraints in the problem)
     * @param const_types types of the constraints (has to be filled by function).
     * @return bool
     */
    virtual bool get_constraints_linearity(Index m, Ipopt::TNLP::LinearityType* const_types);


    /**
     * @brief Method to pass the main dimensions of the problem to Ipopt.
     *
     * @param n number of variables in problem.
     * @param m number of constraints.
     * @param nnz_jac_g number of nonzeroes in Jacobian of constraints system.
     * @param nnz_h_lag number of nonzeroes in Hessian of the Lagrangean.
     * @param index_style indicate wether arrays are numbered from 0 (C-style) or from 1 (Fortran).
     * @return bool
     */
    virtual bool get_nlp_info(Index& n, Index&m, Index& nnz_jac_g,
                               Index& nnz_h_lag, TNLP::IndexStyleEnum& index_style);


    /**
     * @brief Method to pass the bounds on variables and constraints to Ipopt.
     *
     * @param n size of x_l and x_u (has to be equal to the number of variables in the problem)
     * @param x_l lower bounds on variables (function should fill it).
     * @param x_u upper bounds on the variables (function should fill it).
     * @param m size of g_l and g_u (has to be equal to the number of constraints in the problem).
     * @param g_l lower bounds of the constraints (function should fill it).
     * @param g_u upper bounds of the constraints (function should fill it).
     * @return bool
     */
    virtual bool get_bounds_info(Index n, Number* x_l, Number* x_u,
                                  Index m, Number* g_l, Number* g_u);


    /**
     * @brief Method to to pass the starting point for optimization to Ipopt.
     *
     * @param n
     * @param init_x do we initialize primals?
     * @param x pass starting primal points (function should fill it if init_x is 1).
     * @param init_z
     * @param z_L
     * @param z_U
     * @param m size of lambda (has to be equal to the number of constraints in the problem).
     * @param init_lambda do we initialize duals of constraints?
     * @param lambda lower bounds of the constraints (function should fill it).
     * @return bool
     */
    virtual bool get_starting_point(Index n, bool init_x, Number* x,
                                     bool init_z, Number* z_L, Number* z_U,
                                     Index m, bool init_lambda,
                                     Number* lambda);


    /**
     * @brief Method which compute the value of the objective function at point x.
     *
     * @param n size of array x (has to be the number of variables in the problem).
     * @param x point where to evaluate.
     * @param new_x Is this the first time we evaluate functions at this point? (in the present context we don't care).
     * @param obj_value value of objective in x (has to be computed by the function).
     * @return bool
     */
    virtual bool eval_f(Index n, const Number* x, bool new_x, Number& obj_value);


    /**
     * @brief Method which compute the gradient of the objective at a point x.
     *
     * @param n size of array x (has to be the number of variables in the problem).
     * @param x point where to evaluate.
     * @param new_x Is this the first time we evaluate functions at this point?
     * @param grad_f gradient of objective taken in x (function has to fill it).
     * @return bool
     */
    virtual bool eval_grad_f(Index n, const Number* x, bool new_x, Number* grad_f);


    /**
     * @brief Method which compute the value of the functions defining the constraints at a point x.
     *
     * @param n size of array x (has to be the number of variables in the problem).
     * @param x point where to evaluate.
     * @param new_x Is this the first time we evaluate functions at this point?
     * @param m size of array g (has to be equal to the number of constraints in the problem)
     * @param g values of the constraints (function has to fill it).
     * @return bool
     */
    virtual bool eval_g(Index n, const Number* x, bool new_x, Index m, Number* g);


    /**
     * @brief Method to compute the Jacobian of the functions defining the constraints.
     * @details If the parameter values==NULL fill the arrays iCol and jRow which store the position of
     *          the non-zero element of the Jacobian.
     *          If the paramenter values!=NULL fill values with the non-zero elements of the Jacobian.
     *
     * @param n size of array x (has to be the number of variables in the problem).
     * @param x point where to evaluate.
     * @param new_x Is this the first time we evaluate functions at this point?
     * @param m size of array g (has to be equal to the number of constraints in the problem)
     * @param nele_jac
     * @param iRow
     * @param jCol
     * @param values values of the jacobian(function has to fill it).
     * @return bool
     */
    virtual bool eval_jac_g(Index n, const Number* x, bool new_x,
                             Index m, Index nele_jac, Index* iRow, Index *jCol,
                             Number* values);



    /**
     * @brief Method to compute the Hessian
     * @details This method should probably not be used, instead using Newton approximations.
     *
     * @param n
     * @param x
     * @param new_x
     * @param obj_factor
     * @param m
     * @param lambda
     * @param new_lambda
     * @param nele_hess
     * @param iRow
     * @param jCol
     * @param values
     * @return bool
     */
    virtual bool eval_h(Index n, const Number* x, bool new_x,
                         Number obj_factor, Index m, const Number* lambda,
                         bool new_lambda, Index nele_hess, Index* iRow,
                         Index* jCol, Number* values);


     /**
      * @brief Method called by Ipopt at the end of optimization
      *
      * @param status
      * @param n
      * @param x
      * @param obj_value
      */
     virtual void finalize_solution(TMINLP::SolverReturn status,
                                    Index n, const Number* x, Number obj_value);



    virtual const SosInfo * sosConstraints() const{return NULL;}

    virtual const BranchingInfo* branchingInfo() const{return NULL;}



};

#endif // BONMININTERFACE_H
