/* Sink
 *
 * Author: Leonardo Bonati
 * Created on Feb. 2018
 *
 */

#include <omnetpp.h>
#include <math.h>

#include "sink.h"

using namespace omnetpp;

namespace csma_ca {

void SinkNodeCSMACA::initialize()
{
    received_pkts = 0;
    collided = 0;
}

void SinkNodeCSMACA::handleMessage(cMessage *msg)
{
    cModule* c = getModuleByPath("SourceSink");

    // check if collision happened
    if (((int) c->par("concurrent_tx")) > 1)
    {
        EV << "Collision" << endl;

        // increase collided packets counter
        collided++;
    }
    else    // successful transmission
    {
        EV << "Packet successfully received" << endl;

        // increase successful packets counter
        received_pkts++;
    }

    cancelAndDelete(msg);
}

void SinkNodeCSMACA::finish()
{
    cModule* c = getModuleByPath("SourceSink");

    double tot_p = received_pkts + collided + ((double) c->par("dropped_pkts"));

    // print statistics
    EV << "Average delivery ratio: " << ((double) received_pkts) / tot_p * 100. << "%" << endl;
    EV << "Average latency: " << ((double) c->par("latency"))  / ((double) received_pkts) * 1000. << "ms" << endl;
    EV << "Average energy consumption: " << ((double) c->par("energy")) / ((double) received_pkts) * 1000. << "mJ" << endl;
}

}; // namespace


