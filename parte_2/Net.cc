#ifndef NET
#define NET

#include <string.h>
#include <omnetpp.h>
#include <packet_m.h>
#include <start_m.h>
#include <stdlib.h>
#include <time.h>

using namespace omnetpp;

class Net: public cSimpleModule {
private:
    cOutVector hopCounts;
    cMessage *sendInicialization;
    simtime_t serviceTime;
    bool direction;

public:
    Net();
    virtual ~Net();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Net);

#endif /* NET */

Net::Net() {
}

Net::~Net() {
}

void Net::initialize() {
    hopCounts.setName("hopCounts");
    if (this->getParentModule()->getIndex() == 5){
       sendInicialization = new cMessage("sendEvent");
       scheduleAt(par("interArrivalTime"), sendInicialization);
    }
    direction = true;
}

void Net::finish() {
}

void Net::handleMessage(cMessage *msg) {
    // All msg (events) on net are packets
    if (msg == sendInicialization){
       delete(msg);
       // if msg is a sendInicialization, create the Starter
       Start *srt = new Start("start",5);
       srt->setByteLength(par("packetByteSize"));
       srt->setDirection(true);
       send(srt,"toLnk$o", 0);
    }else if (msg->getKind() == 5){
        int index = this->getParentModule()->getIndex();
        Start *srt = (Start *)msg;
        if (msg->getKind() != index){
            if(index >= 1 && index <= 4){
                srt->setDirection(true);
            }else{
                srt->setDirection(false);
            }
            direction = srt->getDirection();
            send(srt, "toLnk$o", 0);
        } else {
            delete(msg);
        }
    } else{
        Packet *pkt = (Packet *) msg;
        // If this node is the final destination, send to App
        if (pkt->getDestination() == this->getParentModule()->getIndex()) {
            send(msg, "toApp$o");
            hopCounts.record(pkt->getHopCount());
        } else {
            // If not, forward the packet to some else... to who?
            // If my packet is coming from "app", we set the direction
            if (msg->arrivedOn("toApp$i")){
                pkt->setDirection(direction);
            }

            // Depending of the packets direction, send the packet left or rigth
            if (pkt->getDirection()){
                send(pkt, "toLnk$o", 1);
            } else {
                send(pkt, "toLnk$o", 0);
            }
            pkt->setHopCount(pkt->getHopCount() + 1);
        }
    }
}
