/* Sensor node
 *
 * Author: Leonardo Bonati
 * Created on Feb. 2018
 *
 */

#include <omnetpp.h>
#include <math.h>

#include "source.h"

using namespace omnetpp;

namespace csma_ca {

SensorNodeCSMACA::SensorNodeCSMACA()
{
    // initialize pointers to nullptr
    backoff_timer_expired = nullptr;
    set_channel_busy = nullptr;
    set_channel_free = nullptr;
    send_message = nullptr;
    decrease_concurrent_tx_counter = nullptr;
}

SensorNodeCSMACA::~SensorNodeCSMACA()
{
    // delete pointers
    if (backoff_timer_expired != nullptr) {
        cancelAndDelete (backoff_timer_expired);
    }

    if (set_channel_busy != nullptr) {
        cancelAndDelete (set_channel_busy);
    }

    if (set_channel_free != nullptr) {
        cancelAndDelete (set_channel_free);
    }

    if (send_message != nullptr) {
        cancelAndDelete (send_message);
    }

    if (decrease_concurrent_tx_counter != nullptr) {
        cancelAndDelete (decrease_concurrent_tx_counter);
    }
}

void SensorNodeCSMACA::initialize()
{
    // initialize algorithm parameters
    D_bp = par("D_bp_value");
    D_p = par("D_p_value");
    macMinBE = par("macMinBE_value");
    macMaxBE = par("macMaxBE_value");
    macMaxCSMABackoffs = par("macMaxCSMABackoffs_value");
    T = par("T_value");
    T_cca = par("T_cca_value");
    pkt_to_send = par("pkt_to_send_value");
    tot_pkt = pkt_to_send;

    pkt_creation_time = 0;

    p_tx = par("p_tx_value");
    p_rx = par("p_rx_value");

    // initialize algorithm variables
    nb = 0;
    be = macMinBE;

    if (pkt_to_send > 0)
    {
        pkt_creation_time = simTime().dbl();

        // wait for backoff time to expire and perform CCA
        if (backoff_timer_expired != nullptr) {
            cancelAndDelete (backoff_timer_expired);
        }

        backoff_timer_expired = new cMessage("backoff_timer_expired");
        scheduleAt(simTime() + generate_backoff_time(), backoff_timer_expired);
    }
}

void SensorNodeCSMACA::handleMessage(cMessage *msg)
{
    if (msg==backoff_timer_expired)
    {
        EV << "Backoff timer expired" << endl;

        // check if channel is free
        if (perform_cca()){

            EV << "Channel is free" << endl;

            // change channel state to busy
            if (set_channel_busy != nullptr) {
                cancelAndDelete (set_channel_busy);
            }

            set_channel_busy = new cMessage("set_channel_busy");
            scheduleAt(simTime() + D_bp - 0.000001, set_channel_busy);
        }
        else  // channel is busy
        {
            EV << "Channel is busy" << endl;

            // update variables
            nb++;
            be++;
            if (be > macMaxBE)
            {
                be = macMaxBE;
            }

            if (nb <= macMaxCSMABackoffs){

                EV << "Retry" << endl;

                // retry
                if (backoff_timer_expired != nullptr) {
                    cancelAndDelete (backoff_timer_expired);
                }

                backoff_timer_expired = new cMessage("backoff_timer_expired");
                scheduleAt(simTime() + D_bp + generate_backoff_time(), backoff_timer_expired);
            }
            else
            {
                EV << "Drop packet" << endl;

                // drop packet
                cModule* c = getModuleByPath("SourceSink");
                c->par("dropped_pkts") = ((int) c->par("dropped_pkts")) + 1;

                // decrease counter and repeat process if there are still packets to send
                decrease_and_repeat();
            }
        }
    }
    else if (msg==set_channel_busy)
    {
        EV << "Set channel busy" << endl;

        // set channel state to busy
        set_channel_state(false);

        // send actual message - add 1 microsecond to compensate and start
        //  at the beginning of the slot
        if (send_message != nullptr) {
            cancelAndDelete (send_message);
        }

        send_message = new cMessage("send_message");
        scheduleAt(simTime() + 0.000001, send_message);
    }
    else if (msg==set_channel_free)
    {
        EV << "Set channel free" << endl;

        // set channel state to free
        set_channel_state(true);

        cModule* c = getModuleByPath("SourceSink");

        if (((int) c->par("concurrent_tx")) <= 1)
        {
            // compute packet latency
            double tmp_lat = 0;
            tmp_lat = simTime().dbl() - pkt_creation_time;
            cModule* c = getModuleByPath("SourceSink");
            c->par("latency") = ((double) c->par("latency")) + tmp_lat;
        }

        // decrease counter of transmissions in progress
        if (decrease_concurrent_tx_counter != nullptr) {
            cancelAndDelete (decrease_concurrent_tx_counter);
        }

        decrease_concurrent_tx_counter = new cMessage("decrease_concurrent_tx_counter");
        scheduleAt(simTime() + 0.000001, decrease_concurrent_tx_counter);
    }
    else if (msg==send_message)
    {
        EV << "Send message" << endl;

        cModule* c = getModuleByPath("SourceSink");

        // update energy consumption
        c->par("energy") = ((double) c->par("energy")) + p_tx * ((double) D_p);

        // increase counters
        c->par("concurrent_tx") = ((int) c->par("concurrent_tx")) + 1;
        c->par("tx_pkts") = ((int) c->par("tx_pkts")) + 1;

        // send actual packet to the sink
        cMessage *data_pkt = new cMessage("data_pkt");
        send(data_pkt, "out");

        // set channel state to free
        if (set_channel_free != nullptr) {
            cancelAndDelete (set_channel_free);
        }

        set_channel_free = new cMessage("set_channel_free");
        scheduleAt(simTime() + D_p, set_channel_free);
    }
    else if (msg==decrease_concurrent_tx_counter)
    {
        cModule* c = getModuleByPath("SourceSink");
        c->par("concurrent_tx") = ((int) c->par("concurrent_tx")) - 1;

        // decrease counter and repeat process if there are still packets to send
        decrease_and_repeat();
    }

}

// returns true if channel free, else false
bool SensorNodeCSMACA::perform_cca()
{
    EV << "Perform CCA" << endl;

    cModule* c = getModuleByPath("SourceSink");

    // update energy consumption
    c->par("energy") = ((double) c->par("energy")) + p_rx * ((double) T_cca);

    return ((bool) c->par("channel_free"));
}

// set channel_free: true=free, false=busy
void SensorNodeCSMACA::set_channel_state(bool state)
{
    cModule* c = getModuleByPath("SourceSink");
    c->par("channel_free") = state;
}

// obtain new backoff time
double SensorNodeCSMACA::generate_backoff_time(){

    int rv_int = intuniform(0, pow(2, be));
    double tmp = ((double) rv_int) * D_bp;

    return tmp;
}

// decrease packet counter and repeat process if there are still packets to send
void SensorNodeCSMACA::decrease_and_repeat()
{
    // reset variables
     nb = 0;
     be = macMinBE;

     // decrease counter
     pkt_to_send--;

    // repeat process
    if (pkt_to_send > 0){

        if (backoff_timer_expired != nullptr) {
            cancelAndDelete (backoff_timer_expired);
        }

        // schedule at next "T" instant
        backoff_timer_expired = new cMessage("backoff_timer_expired");
        scheduleAt((tot_pkt - pkt_to_send) * T + generate_backoff_time(), backoff_timer_expired);

        pkt_creation_time = (tot_pkt - pkt_to_send) * T;
    }
}

void SensorNodeCSMACA::finish()
{

}

}; // namespace
