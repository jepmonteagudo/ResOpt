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


#ifndef RUNNER_H
#define RUNNER_H


/**
 * @brief Class that controls the main execution of ResOpt
 *
 */

#include <tr1/memory>

#include <QString>
#include <QFile>
#include <QVector>
#include <QObject>

class QThread;

using std::tr1::dynamic_pointer_cast;
using std::tr1::shared_ptr;


namespace ResOpt
{

class Model;
class ReservoirSimulator;
class ModelReader;
class Optimizer;
class Launcher;
class CaseQueue;
class Case;



/**
 * @brief Main execution class.
 *
 */
class Runner : public QObject
{
    Q_OBJECT
private:
    QVector<Launcher*> m_launchers;
    QVector<bool> m_launcher_running;
    QVector<QThread*> m_threads;

    ModelReader *p_reader;
    Model *p_model;
    ReservoirSimulator *p_simulator;
    Optimizer *p_optimizer;

    QFile *p_summary;
    int m_number_of_runs;
    bool m_up_to_date;

    CaseQueue *p_cases;

    void initializeLaunchers();



public:
    explicit Runner(const QString &driver_file, QObject *parent = 0);
    ~Runner();


    /**
     * @brief Initializes the model and optimizer.
     * @details The Model is first read from the driver file. Then the ReservoirSimulator and Optimizer are initialized.
     *
     */
    void initialize();







    /**
     * @brief Writes the problem definition to the summary file
     * @details A list of all the variables and constraints are printed to the summary file.
     */
    void writeProblemDefToSummary();


    /**
     * @brief Writes the results from the current run of the Model to the summary file
     * @details The variable, constraints and objective values are printed as a new line to the summary file.
     */
    void writeCasesToSummary();

    // set functions

    void setSummaryFile(const QString &f);

    void setOptimizer(Optimizer *o) {p_optimizer = o;}

    // get functions
    Model* model() {return p_model;}

    bool isUpToDate() {return m_up_to_date;}

public slots:
    void onLauncherFinished(Launcher *l);

    /**
     * @brief Starts the Optimizer
     * @details The function first checks if the Model has been initialized. If not initialize() is called. Then the Optimizer is started
     *          through Optimizer::start().
     *
     */
    void run();


    /**
     * @brief Evaluates a list of cases.
     * @details This function is called by an Optimizer, or another class that needs to evaluate the Model.
     *          The cases in the queue is distrubuted among the Launchers for model evaluation. When a Launcher finishes the evaluation,
     *          it emits a signal that is connected to the onLauncherFinished() slot. Further distrubution of cases is take from there. When
     *          calling this function, it should be done within an event loop. The event loop should wait for the casesFinished() signal before
     *          proceeding.
     *
     * @param cases
     */
    void evaluate(CaseQueue *cases);




signals:
    void optimizationFinished();
    void casesFinished();
    void sendCase(Case *c);

};

} // namespace ResOpt

#endif // RUNNER_H

