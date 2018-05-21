/* Header sink
 *
 * Author: Leonardo Bonati
 * Created on Feb. 2018
 *
 */

#include <omnetpp.h>

using namespace omnetpp;

namespace csma_ca {

class SinkNodeCSMACA : public cSimpleModule
{
  private:
    int received_pkts;
    int collided;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

  public:
    bool get_channel_state();
    void change_channel_state(bool state);
};

Define_Module(SinkNodeCSMACA);

}; // namespace
