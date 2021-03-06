/*
 * This file is part of the ResOpt project.
 *
 * Copyright (C) 2011-2013 Aleksander O. Juell <aleksander.juell@ntnu.no>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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


#ifndef MASTEROPTIMIZER_H
#define MASTEROPTIMIZER_H

#include <QObject>

namespace ResOpt
{

class MasterRunner;
class Case;

class MasterOptimizer : public QObject
{
    Q_OBJECT

private:
    MasterRunner *p_master_runner;

    Case* generateBaseCase();

public:
    MasterOptimizer(MasterRunner *mr);

    void initialize();

    void start();

};


} // namespace ResOpt

#endif // MASTEROPTIMIZER_H
