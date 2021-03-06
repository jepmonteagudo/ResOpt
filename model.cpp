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


#include "model.h"

#include <iostream>
#include "reservoir.h"
#include "well.h"
#include "capacity.h"
#include "intvariable.h"
#include "realvariable.h"
#include "binaryvariable.h"
#include "constraint.h"
#include "objective.h"
#include "cost.h"
#include "midpipe.h"
#include "endpipe.h"
#include "separator.h"
#include "pressurebooster.h"
#include "stream.h"
#include "productionwell.h"
#include "pipeconnection.h"
#include "userconstraint.h"
#include "logger.h"

using std::cout;
using std::endl;


namespace ResOpt
{



Model::Model()
    : p_reservoir(0),
      p_obj(0),
      m_up_to_date(false)
{
    p_logger = new Logger(Logger::DELEGATE);

}

//-----------------------------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------------------------
Model::Model(const Model &m)
{
    // logger
    p_logger = new Logger(Logger::DELEGATE);

    // setting the up to date status
    m_up_to_date = false;

    // copying driver path
    m_driver_path = m.m_driver_path;

    // copying the reservoir
    p_reservoir = new Reservoir(*m.reservoir());

    // copying the wells
    for(int i = 0; i < m.numberOfWells(); i++)
    {
        m_wells.push_back(m.well(i)->clone());
    }

    // copying the pipes
    for(int i = 0; i < m.numberOfPipes(); i++)
    {
        m_pipes.push_back(m.pipe(i)->clone());
    }

    // copying the capacity constraints
    for(int i = 0; i < m.numberOfCapacities(); i++)
    {
        m_capacities.push_back(new Capacity(*m.m_capacities.at(i)));
    }

    // copying the objective
    p_obj = m.p_obj->clone();

    // copying the master schedule
    m_master_schedule = m.m_master_schedule;

    // copying the user defined constraints
    for(int i = 0; i < m.m_user_constraints.size(); ++i)
    {
        m_user_constraints.push_back(new UserConstraint(*m.m_user_constraints.at(i), this));
    }


}


Model::~Model()
{
    delete p_logger;

    if(p_reservoir != 0) delete p_reservoir;

    for(int i = 0; i < m_wells.size(); i++) delete m_wells.at(i);
    for(int i = 0; i < m_pipes.size(); i++) delete m_pipes.at(i);
    for(int i = 0; i < m_capacities.size(); i++) delete m_capacities.at(i);
    for(int i = 0; i < m_user_constraints.size(); ++i) delete m_user_constraints.at(i);
}


//-----------------------------------------------------------------------------------------------
// Validates the Model
//-----------------------------------------------------------------------------------------------
bool Model::validate()
{
    bool ok = true;

    // first checking that the reservoir is defined
    if(p_reservoir == 0)
    {
        cout << endl << "###  Model Validation Error  ###" << endl
             << "No RESERVOIR defined..." << endl << endl;

        ok = false;
    }

    // checking that at least one well is defined
    if(numberOfWells() == 0)
    {
        cout << endl << "###  Model Validation Error  ###" << endl
             << "No WELL defined..." << endl << endl;
        ok = false;
    }

    // checking that at least one pipe is defined
    if(numberOfPipes() == 0)
    {
        cout << endl << "###  Model Validation Error  ###" << endl
             << "No PIPE defined..." << endl << endl;
        ok = false;
    }

    // checking that the master schedule at least containts one entry
    if(numberOfMasterScheduleTimes() < 1)
    {
        cout << endl << "###  Model Validation Error  ###" << endl
             << "MASTERSCHEDULE does not containt any entries..." << endl << endl;
        ok = false;
    }

    // checking that the master schedule corresponds to all well schedules
    for(int i = 0; i < numberOfWells(); ++i)
    {
        // first checking that the schedules have the same size
        if(well(i)->numberOfControls() != numberOfMasterScheduleTimes())
        {
            cout << endl << "###  Model Validation Error  ###" << endl
                 << "Well: " << well(i)->name().toLatin1().constData() << endl
                 << "Does not have the same number of SHEDULE entries as the MASTERSCHEDULE..." << endl << endl;
            ok = false;
            break;
        }
        else
        {
            // checking that each time entry in the well is the same as in the master schedule
            for(int j = 0; j < numberOfMasterScheduleTimes(); ++j)
            {
                if(well(i)->control(j)->endTime() != masterScheduleTime(j))
                {

                    cout << endl << "###  Model Validation Error  ###" << endl
                         << "Well: " << well(i)->name().toLatin1().constData() << endl
                         << "SHEDULE entry: " << well(i)->control(j)->endTime() << endl
                         << "Is not found in the MASTERSCHEDULE..." << endl << endl;
                    ok = false;
                    break;


                }
            } // master schedule entries
        }

    } // well

    // checking that the master schedule corresponds to the gas lift schedule for the production wells that have gas lift
    for(int i = 0; i < numberOfWells(); ++i)
    {
        // checking if this is a production well
        ProductionWell *prod_well = dynamic_cast<ProductionWell*>(well(i));

        if(prod_well != 0)
        {
            // checking if this production well has gas lift
            if(prod_well->hasGasLift())
            {
                // first checking that the schedules have the same size
                if(prod_well->numberOfGasLiftControls() != numberOfMasterScheduleTimes())
                {
                    cout << endl << "###  Model Validation Error  ###" << endl
                         << "Well: " << prod_well->name().toLatin1().constData() << endl
                         << "Does not have the same number of GASLIFT entries as the MASTERSCHEDULE..." << endl << endl;
                    ok = false;
                    break;
                }
                else
                {
                    // checking that each time entry in the well is the same as in the master schedule
                    for(int j = 0; j < numberOfMasterScheduleTimes(); ++j)
                    {
                        if(prod_well->gasLiftControl(j)->endTime() != masterScheduleTime(j))
                        {

                            cout << endl << "###  Model Validation Error  ###" << endl
                                 << "Well: " << prod_well->name().toLatin1().constData() << endl
                                 << "GASLIFT entry: " << prod_well->gasLiftControl(j)->endTime() << endl
                                 << "Is not found in the MASTERSCHEDULE..." << endl << endl;
                            ok = false;
                            break;


                        }
                    } // master schedule entries
                }

            } // has gas lift
        } // production well
    }   // well





    return ok;
}


//-----------------------------------------------------------------------------------------------
// Connects wells and pipes to the outlets
//-----------------------------------------------------------------------------------------------
bool Model::resolvePipeRouting()
{
    cout << "Resolving the pipe routing..." << endl;

    bool ok = true;

    // first cleaning up the current feeds connected to the pipes
    for(int k = 0; k < m_pipes.size(); ++k) m_pipes.at(k)->cleanFeedConnections();


    // connecting the wells
    for(int i = 0; i < m_wells.size(); ++i)     // looping through each well
    {

        // checking if this is a production well

        ProductionWell *prod_well = dynamic_cast<ProductionWell*>(well(i));

        if(prod_well != 0)     // skipping injection wells
        {

            // looping through the pipe connections for the well
            for(int k = 0; k < prod_well->numberOfPipeConnections(); ++k)
            {
                bool connection_ok;
                int pipe_num = prod_well->pipeConnection(k)->pipeNumber();

                for(int j = 0; j < m_pipes.size(); j++)     // looping through the pipes to find the correct one
                {
                    if(m_pipes.at(j)->number() == pipe_num)     // found the correct pipe
                    {
                        m_pipes.at(j)->addFeedWell(prod_well);              // adding the well as a feed to the pipe
                        prod_well->pipeConnection(k)->setPipe(m_pipes.at(j));   // setting the pipe as outlet pipe for the pipe connection

                        connection_ok = true;
                        break;
                    }
                } // pipe for pipe connection k

                // checking if the well - pipe connection was ok
                if(!connection_ok)
                {
                    cout << endl << "###  Runtime Error  ###" << endl
                        << "Well to Pipe connection could not be established..." << endl
                        << "PIPE: " << pipe_num << endl
                        << "WELL: " << prod_well->name().toLatin1().constData() << endl;

                    exit(1);

                }


            } // pipe connections for well i

            // checking that the well is at least connected to one pipe
            if(prod_well->numberOfPipeConnections() == 0)
            {
                cout << endl << "###  Runtime Error  ###" << endl
                     << "Well " << prod_well->name().toLatin1().constData() << endl
                     << "Is not connected to any pipe..." << endl << endl;
                exit(1);

            }

        }

    }

    // connecting the pipes
    for(int i = 0; i < m_pipes.size(); i++)     // looping through the pipes
    {
        // only the MidPipe, Separator, and PressureBooster types should be checked
        MidPipe *p_mid = dynamic_cast<MidPipe*>(m_pipes.at(i));
        Separator *p_sep = dynamic_cast<Separator*>(m_pipes.at(i));
        PressureBooster *p_boost = dynamic_cast<PressureBooster*>(m_pipes.at(i));

        if(p_mid != 0)  // this is a MidPipe
        {
            // looping through the outlet connections for the pipe
            for(int k = 0; k < p_mid->numberOfOutletConnections(); k++)
            {
                bool pipe_ok = false;

                int pipe_num = p_mid->outletConnection(k)->pipeNumber();

                // checking if it is connected to it self
                if(pipe_num == p_mid->number())
                {
                    cout << endl << "###  Runtime Error  ###" << endl
                         << "Pipe " << pipe_num << " is connected to itself!" << endl
                         << "OUTLETPIPE Connection: " << k << endl << endl;

                    exit(1);
                }

                // looping through the pipes to find the correct one
                for(int j = 0; j < numberOfPipes(); j++)
                {
                    if(pipe_num == pipe(j)->number())
                    {
                        p_mid->outletConnection(k)->setPipe(pipe(j));
                        pipe(j)->addFeedPipe(p_mid);

                        pipe_ok = true;
                        break;
                    }

                }

                // checking if the pipe - pipe connection was ok
                if(!pipe_ok)
                {
                    cout << endl << "###  Runtime Error  ###" << endl
                         << "Pipe to Pipe connection could not be established..." << endl
                         << "UPSTREAM PIPE: " << pipe_num << endl
                         << "DOWNSTREAM PIPE: " << pipe(i)->number() << endl;

                    exit(1);

                }

            } // connection k

            // checking that the pipe has at least one outlet connection
            if(p_mid->numberOfOutletConnections() == 0)
            {
                cout << endl << "###  Runtime Error  ###" << endl
                     << "Pipe " << p_mid->number() << endl
                     << "Is not connected to any upstream pipe..." << endl << endl;
                exit(1);

            }

        } // midpipe
        else if(p_sep != 0) // this is a Separator
        {

            bool pipe_ok = false;

            int pipe_num = p_sep->outletConnection()->pipeNumber();

            // checking if it is connected to it self
            if(pipe_num == p_sep->number())
            {
                cout << endl << "###  Runtime Error  ###" << endl
                     << "Separator " << pipe_num << " is connected to itself!" << endl << endl;

                exit(1);
            }

            // looping through the pipes to find the correct one
            for(int j = 0; j < numberOfPipes(); j++)
            {
                if(pipe_num == pipe(j)->number())
                {
                    p_sep->outletConnection()->setPipe(pipe(j));
                    pipe(j)->addFeedPipe(p_sep);

                    pipe_ok = true;
                    break;
                }

            }

            // checking if the pipe - pipe connection was ok
            if(!pipe_ok)
            {
                cout << endl << "###  Runtime Error  ###" << endl
                     << "Separator to Pipe connection could not be established..." << endl
                     << "SEPARATOR:       " << pipe_num << endl
                     << "DOWNSTREAM PIPE: " << pipe(i)->number() << endl;

                exit(1);

            }
        } // separator

        else if(p_boost != 0) // this is a PressureBooster
        {

            bool pipe_ok = false;

            int pipe_num = p_boost->outletConnection()->pipeNumber();

            // checking if it is connected to it self
            if(pipe_num == p_boost->number())
            {
                cout << endl << "###  Runtime Error  ###" << endl
                     << "Booster " << pipe_num << " is connected to itself!" << endl << endl;

                exit(1);
            }

            // looping through the pipes to find the correct one
            for(int j = 0; j < numberOfPipes(); ++j)
            {
                if(pipe_num == pipe(j)->number())
                {
                    p_boost->outletConnection()->setPipe(pipe(j));
                    pipe(j)->addFeedPipe(p_boost);

                    pipe_ok = true;
                    break;
                }

            }

            // checking if the pipe - pipe connection was ok
            if(!pipe_ok)
            {
                cout << endl << "###  Runtime Error  ###" << endl
                     << "Booster to Pipe connection could not be established..." << endl
                     << "BOOSTER:         " << pipe_num << endl
                     << "DOWNSTREAM PIPE: " << pipe(i)->number() << endl;

                exit(1);

            }
        } // booster

    }   // pipe i


    return ok;
}


//-----------------------------------------------------------------------------------------------
// Calculates the pressures in all the pipes
//-----------------------------------------------------------------------------------------------
bool Model::calculatePipePressures()
{
    bool ok = true;

    // finding the endnodes
    QVector<EndPipe*> end_pipes;

    for(int i = 0; i < numberOfPipes(); i++)
    {
        EndPipe *p = dynamic_cast<EndPipe*>(pipe(i));   // trying to cast as EndPipe
        if(p != 0) end_pipes.push_back(p);              // adding if cast was ok
    }

    // if no endpipes were found, return error
    if(end_pipes.size() == 0)
    {
        cout << endl << "### Warning ###" << endl
             << "From: Model" << endl
             << "No end nodes found in the pipe system..." << endl
             << "At least one PIPE must have an OUTLETPRESSURE defined..." << endl << endl;

        ok = false;
    }
    else    // found end pipes, getting on with the calculations...
    {
        // looping through the end nodes
        for(int i = 0; i < end_pipes.size(); i++)
        {
            // finding the connected wells for this branch
            //end_pipes.at(i)->findConnectedWells();

            // calculating the inlet pressures for the end pipes, and the branch of pipes connected
            end_pipes.at(i)->calculateBranchInletPressures();
        }

    }


    return ok;

}

//-----------------------------------------------------------------------------------------------
// Connects capacities to the pipes
//-----------------------------------------------------------------------------------------------
bool Model::resolveCapacityConnections()
{
    cout << "Resolving capacity - pipe connections..." << endl;
    bool ok = true;

    for(int i = 0; i < m_capacities.size(); i++)        // looping through all separators
    {
        Capacity *s = m_capacities.at(i);

        for(int j = 0; j < s->numberOfFeedPipeNumbers(); j++)   // looping through all the pipes specified in the driver file
        {
            int pipe_num = s->feedPipeNumber(j);
            bool pipe_ok = false;

            for(int k = 0; k < m_pipes.size(); k++)     // looping through pipes to find the correct one
            {
                if(m_pipes.at(k)->number() == pipe_num) // this is the correct pipe
                {
                    s->addFeedPipe(m_pipes.at(k));

                    pipe_ok = true;
                    break;
                }
            }

            // checking if the pipe number was found
            if(!pipe_ok)
            {
                cout << endl << "###  Runtime Error  ###" << endl
                     << "Capacity to Pipe connection could not be established..." << endl
                     << "PIPE:     " << pipe_num << endl
                     << "CAPACITY: " << s->name().toLatin1().constData() << endl;

                exit(1);

            }

        }
    }

    return ok;
}

//-----------------------------------------------------------------------------------------------
// returns the well with the given id
//-----------------------------------------------------------------------------------------------
Well* Model::wellById(int comp_id)
{
    for(int i = 0; i < numberOfWells(); ++i)
    {
        if(well(i)->id() == comp_id) return well(i);
    }

    return 0;
}

//-----------------------------------------------------------------------------------------------
// returns the well with the given name
//-----------------------------------------------------------------------------------------------
Well* Model::wellByName(const QString &name)
{
    for(int i = 0; i < numberOfWells(); ++i)
    {
        if(well(i)->name().compare(name) == 0) return well(i);
    }

    return 0;
}

//-----------------------------------------------------------------------------------------------
// Updates the capacity constraints
//-----------------------------------------------------------------------------------------------
bool Model::updateCapacityConstraints()
{
    bool ok = true;

   // cout << "Updating the capacity constraints..." << endl;

    for(int i = 0; i < numberOfCapacities(); i++)
    {
        capacity(i)->updateConstraints();
    }

    return ok;
}

//-----------------------------------------------------------------------------------------------
// Updates the Well BHP and connection constraints
//-----------------------------------------------------------------------------------------------
bool Model::updateWellConstaints()
{
    bool ok = true;

   // cout << "Updating the well BHP and connection constraints" << endl;

    for(int i = 0; i < numberOfWells(); i++)
    {
        // checking if this is a production well
        ProductionWell *prod_well = dynamic_cast<ProductionWell*>(well(i));

        if(prod_well != 0)
        {
            prod_well->updateBhpConstraint();
            prod_well->updatePipeConnectionConstraint();
        }
    }

    return ok;
}

//-----------------------------------------------------------------------------------------------
// Updates the Pipe connection constraints
//-----------------------------------------------------------------------------------------------
bool Model::updatePipeConstraints()
{
    bool ok = true;

    for(int i = 0; i < numberOfPipes(); ++i)
    {
        // checking if this is a mid pipe
        MidPipe *p_mid = dynamic_cast<MidPipe*>(pipe(i));
        if(p_mid != 0)
        {
            p_mid->updateOutletConnectionConstraint();
        }
    }

    return ok;
}

//-----------------------------------------------------------------------------------------------
// Updates the Booster capacity constraints
//-----------------------------------------------------------------------------------------------
bool Model::updateBoosterConstraints()
{
    bool ok = true;

    for(int i = 0; i < numberOfPipes(); ++i)
    {
        // checking if this is a booster
        PressureBooster *p_boost = dynamic_cast<PressureBooster*>(pipe(i));
        if(p_boost != 0)
        {
            p_boost->updateCapacityConstraints();
        }
    }

    return ok;
}

//-----------------------------------------------------------------------------------------------
// Updates the user defined constraints
//-----------------------------------------------------------------------------------------------
bool Model::updateUserDefinedConstraints()
{
    bool ok = true;

    for(int i = 0; i < numberOfUserDefinedConstraints(); ++i)
    {
        if(!userDefinedConstraint(i)->update()) ok = false;
    }

    return ok;
}

//-----------------------------------------------------------------------------------------------
// Updates the constraints that are common for all model types
//-----------------------------------------------------------------------------------------------
bool Model::updateCommonConstraints()
{
    bool ok = true;

    if(!updateCapacityConstraints()) ok = false;
    if(!updateWellConstaints()) ok = false;
    if(!updatePipeConstraints()) ok = false;
    if(!updateBoosterConstraints()) ok = false;
    if(!updateUserDefinedConstraints()) ok = false;

    return ok;
}


//-----------------------------------------------------------------------------------------------
// Reads all the pipe pressure drop files
//-----------------------------------------------------------------------------------------------
void Model::readPipeFiles()
{
    for(int i = 0; i < numberOfPipes(); i++)
    {
        // this should not be done for separators or boosters

        Separator *sep = dynamic_cast<Separator*>(pipe(i));
        PressureBooster *boost = dynamic_cast<PressureBooster*>(pipe(i));

        if(sep == 0 && boost == 0) pipe(i)->readInputFile();
    }
}






//-----------------------------------------------------------------------------------------------
// Updates the value of the objective
//-----------------------------------------------------------------------------------------------
void Model::updateObjectiveValue()
{
    QVector<Stream*> field_rates;

    // finding the end pipes
    QVector<EndPipe*> p_end_pipes;
    for(int i = 0; i < numberOfPipes(); ++i)
    {
        EndPipe *p = dynamic_cast<EndPipe*>(pipe(i));
        if(p != 0) p_end_pipes.push_back(p);
    }

    // adding together the streams from all the end pipes
    for(int i = 0; i < masterSchedule().size(); ++i)
    {
        Stream *s = new Stream();

        // looping through the end pipes
        for(int j = 0; j < p_end_pipes.size(); ++j)
        {
            *s += *p_end_pipes.at(j)->stream(i);

            //cout << "Rates used for objective value:" <<endl;
            //s->printToCout();
        }

        field_rates.push_back(s);
    }


    // then collecting all the costs
    QVector<Cost*> costs;

    // collecting the cost of the separators:
    for(int i = 0; i < numberOfPipes(); ++i)
    {
        Separator *p_sep = dynamic_cast<Separator*>(pipe(i));
        if(p_sep != 0)
        {
            // updating the remove fraction and capacity in the cost according to the variable values in the separator
            p_sep->cost()->setFraction(p_sep->removeFraction()->value());
            p_sep->cost()->setCapacity(p_sep->removeCapacity()->value());

            // updating the time of the cost according to the variable
            int time_cost = p_sep->installTime()->value();


            if(time_cost <= 0) p_sep->cost()->setTime(0.0);
            else if(time_cost >= m_master_schedule.size()) p_sep->cost()->setTime(m_master_schedule.at(m_master_schedule.size()-1) + 1);
            else p_sep->cost()->setTime(m_master_schedule.at(time_cost-1));

            // adding the cost to the vector
            costs.push_back(p_sep->cost());

            //cout << "Cost for separator #" << p_sep->number() << " = " << p_sep->cost()->value() << endl;
            //cout << "Install time   = " << p_sep->cost()->time() << endl << endl;
        }
    }

    // collecting the cost of the boosters:
    for(int i = 0; i < numberOfPipes(); ++i)
    {
        PressureBooster *p_boost = dynamic_cast<PressureBooster*>(pipe(i));
        if(p_boost != 0)
        {
            // updating the pressure and capacity in the cost according to the variable values in the booster
            //p_boost->cost()->setFraction(p_boost->pressureVariable()->value());
            //p_boost->cost()->setCapacity(p_boost->capacityVariable()->value());
            p_boost->cost()->setFraction(p_boost->pressureVariable()->value());
            p_boost->cost()->setCapacity(p_boost->capacityVariable()->value());

            // updating the time of the cost according to the variable
            int time_cost = p_boost->installTime()->value();


            if(time_cost <= 0) p_boost->cost()->setTime(0.0);
            else if(time_cost >= m_master_schedule.size()) p_boost->cost()->setTime(m_master_schedule.at(m_master_schedule.size()-1) + 1);
            else p_boost->cost()->setTime(m_master_schedule.at(time_cost-1));

            // adding the cost to the vector
            costs.push_back(p_boost->cost());


        }
    }


    // collecting the installation cost of the wells:
    for(int i = 0; i < numberOfWells(); ++i)
    {

        Well *w = well(i);

        if(w->hasCost())
        {

            if(w->hasInstallTime())
            {
                // updating the time of the cost according to the variable
                int time_cost = w->installTime()->value();


                if(time_cost <= 0) w->cost()->setTime(0.0);
                else if(time_cost >= m_master_schedule.size()) w->cost()->setTime(m_master_schedule.at(m_master_schedule.size()-1) + 1);
                else w->cost()->setTime(m_master_schedule.at(time_cost));


            }
            else
            {
                w->cost()->setTime(0.0);
            }

            // adding the cost to the vector
            costs.push_back(w->cost());
        }

    }


    QVector<Cost*> costs_sorted = sortCosts(costs);


    //qSort(costs.begin(), costs.end());  // sorting the costs wrt. time



    // calculating the new objective value
    objective()->calculateValue(field_rates, costs_sorted);

   // cout << "Objective value = " << objective()->value() << endl;

    // deleting the generated streams
    for(int i = 0; i < field_rates.size(); i++) delete field_rates.at(i);
}


//-----------------------------------------------------------------------------------------------
// Sorts the costs wrt. time
//-----------------------------------------------------------------------------------------------
QVector<Cost*> Model::sortCosts(QVector<Cost *> c)
{
    QVector<Cost*> result;

    // adding the first element to the results
    if(c.size() > 0) result.push_back(c.at(0));

    // looping through the unsorted costs
    for(int i = 1; i < c.size(); ++i)
    {
        Cost *current_unsorted = c.at(i);

        // finding the correct place in the results:
        bool inserted = false;
        for(int j = 0; j < result.size(); ++j)
        {
            // checking if the current_unsorted < result
            if(current_unsorted->time() <= result.at(j)->time())
            {
                result.insert(j, current_unsorted);
                inserted = true;
                break;
            }
        }
        if(!inserted) result.push_back(current_unsorted);
    }

    return result;

}

//-----------------------------------------------------------------------------------------------
// Assignment operator
//-----------------------------------------------------------------------------------------------
Model& Model::operator =(const Model &rhs)
{
    bool ok = this->numberOfWells() == rhs.numberOfWells() &&
            this->numberOfPipes() == rhs.numberOfPipes() &&
            this->numberOfRealVariables() == rhs.numberOfRealVariables() &&
            this->numberOfBinaryVariables() == rhs.numberOfBinaryVariables() &&
            this->numberOfIntegerVariables() == rhs.numberOfIntegerVariables() &&
            this->numberOfConstraints() == rhs.numberOfConstraints();

    if(this != &rhs && ok)
    {
        // updating wells
        for(int i = 0; i < numberOfWells(); ++i)
        {
            // looping through the streams
            for(int j = 0; j < well(i)->numberOfStreams(); ++j)
            {
                *well(i)->stream(j) = *rhs.well(i)->stream(j);
                //well(i)->stream(j)->setPressure(rhs.well(i)->stream(j)->pressure(true));
            }

        }

        // updating pipes
        for(int i = 0; i < numberOfPipes(); ++i)
        {
            // looping through the streams
            for(int j = 0; j < pipe(i)->numberOfStreams(); ++j)
            {
                *pipe(i)->stream(j) = *rhs.pipe(i)->stream(j);
                //pipe(i)->stream(j)->setPressure(rhs.pipe(i)->stream(j)->pressure(true));
            }

        }

        // updating real variable values
        for(int i = 0; i < numberOfRealVariables(); ++i) realVariables().at(i)->setValue(rhs.realVariableValue(i));

        // updating binary variable values
        for(int i = 0; i < numberOfBinaryVariables(); ++i) binaryVariables().at(i)->setValue(rhs.binaryVariableValue(i));

        // updating integer variable values
        for(int i = 0; i < numberOfIntegerVariables(); ++i) integerVariables().at(i)->setValue(rhs.integerVariableValue(i));

        // updating constraint values
        for(int i = 0; i < numberOfConstraints(); ++i) constraints().at(i)->setValue(rhs.constraintValue(i));




    }

    return *this;
}


} // namespace ResOpt
