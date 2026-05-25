#include "Subscriber.h"
#include "PublisherTimeTag_m.h"
using namespace inet;
using namespace omnetpp;

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/common/Simsignals.h"

namespace sub {

Define_Module(SubscriberApp);


void SubscriberApp::initialize(int stage)
{
    UdpSink::initialize(stage); // to not skip the UdpSink setup for ports etc

    if (stage == INITSTAGE_LOCAL)
    {
        latencySignal = registerSignal("latency");
    }
}


void SubscriberApp::finish()
{
    UdpSink::finish();
}


void SubscriberApp::processStart()
{
    socket.setOutputGate(gate("socketOut"));
    socket.bind(localPort);
    setSocketOptions();

    if (stopTime >= SIMTIME_ZERO) {
        selfMsg->setKind(STOP);
        scheduleAt(stopTime, selfMsg);
    }
}



void SubscriberApp::processPacket(Packet *pk)
{
    EV_INFO << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
    emit(packetReceivedSignal, pk);

        const auto& payload = pk->peekData();
        auto tag = payload->findTag<PublisherTimeTag>();
        if (tag != nullptr) {
             publishTime = tag->getPublisherTime();
             EV_INFO << "publishTime = " << publishTime << " s\n";
        }
        else {
            EV_WARN << "No PublisherTimeTag found in payload!\n";
       }

    delete pk;

    subscribedDataSet = par("subscribedDataSet");
    dataSetReader = par("dataSetReader");
    readerGroup = par("readerGroup");
    processingRecTime = par("processingRecTime");
    decodingExecTime = subscribedDataSet + dataSetReader + readerGroup + processingRecTime;

    selfMsg->setKind(DECODE);
    scheduleAt(simTime() + decodingExecTime, selfMsg);
}


void SubscriberApp::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    processPacket(packet);
}



void SubscriberApp::processDecoding()
{
    simtime_t end = simTime();
    simtime_t diff = end - publishTime;
    latency = diff.inUnit(SIMTIME_US);

    numReceived++;

    emit(latencySignal, latency);
}




void SubscriberApp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
            case START:
                processStart();
                break;

            case DECODE:
                processDecoding();
                break;

            case STOP:
                processStop();
                break;

            default:
                throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
        }
    }
    else if (msg->arrivedOn("socketIn"))
        socket.processMessage(msg);
    else
        throw cRuntimeError("Unknown incoming gate: '%s'", msg->getArrivalGate()->getFullName());
}

}

