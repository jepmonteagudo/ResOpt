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


#ifndef GPRSSIMULATOR_H
#define GPRSSIMULATOR_H

#include <QStringList>
#include "reservoirsimulator.h"

class QTextStream;


namespace ResOpt
{

class Well;
class WellControl;


/**
 * @brief Interface for GPRS from Stanford.
 * @details This interface takes care of generating input files in the GPRS format, launching GPRS, and extracting output. GPRS must be installed (and in the path) on the computer.
 *
 */
class GprsSimulator : public ReservoirSimulator
{
private:
    bool generateMainInputFile(Model *m);
    bool generateWellInputFile(Model *m);
    void printWellControl(QTextStream *out, WellControl *wc);

    bool generateControlInputFile(Model *m);

    bool readWellOutput(Well *w, QString file_name);

    QStringList processLine(const QString &line);

public:
    GprsSimulator();

    GprsSimulator(const GprsSimulator &g);

    virtual ~GprsSimulator();

    // virtual functions

    virtual ReservoirSimulator* clone() const {return new GprsSimulator(*this);}

    virtual QString description() const {return QString("SIMULATOR GPRS\n\n");}

    virtual bool generateInputFiles(Model *m);
    virtual bool launchSimulator();
    virtual bool readOutput(Model *m);


};

} // namespace ResOpt

#endif // GPRSSIMULATOR_H
