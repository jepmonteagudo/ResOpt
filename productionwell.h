/*
 * This file is part of the ResOpt project.
 *
 * Copyright (C) 2011-2012 Aleksander O. Juell <aleksander.juell@ntnu.no>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */


#ifndef PRODUCTIONWELL_H
#define PRODUCTIONWELL_H

#include "well.h"

#include <tr1/memory>

using std::tr1::shared_ptr;


namespace ResOpt
{

class IntVariable;
class BinaryVariable;
class PipeConnection;
class Constraint;
class Pipe;


/**
 * @brief Representation of a production well.
 *
 */
class ProductionWell : public Well
{
private:
    QVector<shared_ptr<Constraint> > m_bhp_constraints;        // vector of bhp constraint<s

    QVector<PipeConnection*> m_pipe_connections;               // vector of all pipes that could be connected to the well
    shared_ptr<Constraint> p_connection_constraint;            // constraint that makes sure that the sum of flow to pipes = 1

    QVector<WellControl*> m_gaslift_schedule;           // the gas lift schedule of the well



public:
    ProductionWell();
    ProductionWell(const ProductionWell &w);

    virtual ~ProductionWell();

    virtual Well* clone() const {return new ProductionWell(*this);}

    // overloaded functions
    void setName(const QString &n);

    /**
     * @brief Checks if the inlet pressure of the connected pipe is at any time higher than the wells BHP
     * @details If the inlet pressure of the pipe is higher than the producing BHP, this is not a feasible solution,
     *          and the constraint is set out of bounds.
     *
     */

    void setupConstraints();

    void updateBhpConstraint();

    void updatePipeConnectionConstraint();


    /**
     * @brief Finds the fraction of the rates from this Well that flows through the pipe p
     *
     * @param p
     * @param ok false if the Pipe is not connected to this Well
     * @return double
     */
    double flowFraction(Pipe *p, bool *ok = 0);

    // virtual functions

    virtual void setAutomaticType() {setType(Well::PROD);}

    virtual QString description() const;

    // add functions

    void addPipeConnection(PipeConnection *c) {m_pipe_connections.push_back(c);}

    void addGasLiftControl(WellControl *c) {m_gaslift_schedule.push_back(c);}


    // set functions

    //get functions


    /**
     * @brief Returns the Constraint on the BHP
     *
     * @return Constraint
     */
    shared_ptr<Constraint> bhpConstraint(int i) {return m_bhp_constraints.at(i);}
    int numberOfBhpConstraints() const {return m_bhp_constraints.size();}

    shared_ptr<Constraint> pipeConnectionConstraint() {return p_connection_constraint;}

    int numberOfPipeConnections() const {return m_pipe_connections.size();}
    PipeConnection* pipeConnection(int i) {return m_pipe_connections.at(i);}

    int numberOfGasLiftControls() const {return m_gaslift_schedule.size();}
    WellControl* gasLiftControl(int i) {return m_gaslift_schedule.at(i);}

    bool hasGasLift() const {return (m_gaslift_schedule.size() == numberOfControls());}



};

} // namespace ResOpt

#endif // PRODUCTIONWELL_H
