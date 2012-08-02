#ifndef SEPARATOR_H
#define SEPARATOR_H

#include "pipe.h"

#include <tr1/memory>

using std::tr1::shared_ptr;

namespace ResOpt
{

class Cost;
class IntVariable;
class RealVariable;
class PipeConnection;


/**
 * @brief A separator that can remove phases
 * @details The separator can be fed by any number of pipes, and feeds one upstream pipe. The total rates going through the separator is calculated from the feed pipes.
 *          An installation time is set for the separator. From the installation time and on, any of the phases flowing through the separator can be removed.
 *
 */
class Separator : public Pipe
{
private:

    Cost *p_cost;                               // the cost associated with installing the separator
    shared_ptr<IntVariable> p_install_time;     // the installation time of the separator
    shared_ptr<RealVariable> p_remove_fraction; // the fraction of water that should be removed from the separator
    shared_ptr<RealVariable> p_remove_capacity; // the maximum removal capacity of water
    PipeConnection *p_outlet_connection;        // the outlet pipe that this separator feeds




public:
    Separator();
    Separator(const Separator &s);

    virtual ~Separator();

    // virtual functions

    virtual Pipe* clone() {return new Separator(*this);}

    virtual void calculateInletPressure();

    // set functions
    void setOutletConnection(PipeConnection *c) {p_outlet_connection = c;}
    void setCost(Cost *c) {p_cost = c;}
    void setInstallTime(shared_ptr<IntVariable> t) {p_install_time = t;}
    void setRemoveFraction(shared_ptr<RealVariable> f) {p_remove_fraction = f;}
    void setRemoveCapacity(shared_ptr<RealVariable> c) {p_remove_capacity = c;}

    // get functions
    PipeConnection* outletConnection() {return p_outlet_connection;}
    Cost* cost() {return p_cost;}
    shared_ptr<IntVariable> installTime() {return p_install_time;}
    shared_ptr<RealVariable> removeFraction() {return p_remove_fraction;}
    shared_ptr<RealVariable> removeCapacity() {return p_remove_capacity;}
};

} // namespace ResOpt

#endif // SEPARATOR_H