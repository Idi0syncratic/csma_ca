/* Header sensor node
 *
 * Author: Leonardo Bonati
 * Created on Feb. 2018
 *
 */

#include <omnetpp.h>

using namespace omnetpp;

namespace csma_ca {

class SensorNodeCSMACA : public cSimpleModule
{
  private:
    cMessage *backoff_timer_expired;
    cMessage *set_channel_busy;
    cMessage *set_channel_free;
    cMessage *send_message;
    cMessage *decrease_concurrent_tx_counter;
    int nb;
    int be;
    int macMinBE;
    int macMaxBE;
    int macMaxCSMABackoffs;
    double D_bp;
    int T;
    double D_p;
    int pkt_to_send;
    int tot_pkt;
    double T_cca;
    double pkt_creation_time;
    double p_tx;
    double p_rx;

  public:
    SensorNodeCSMACA();
     virtual ~SensorNodeCSMACA();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
    bool perform_cca();
    void set_channel_state(bool state);
    double generate_backoff_time();
    void decrease_and_repeat();
};

Define_Module(SensorNodeCSMACA);

}; // namespace
