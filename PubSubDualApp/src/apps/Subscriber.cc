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

// this is the "usual" subscriber, to measure the RTT in node a. It does not send packets, only receive

namespace sub {

Define_Module(Subscriber);

// Override methods here
void Subscriber::initialize(int stage)
{
    UdpSink::initialize(stage); // to not skip the UdpSink setup for ports etc

    if (stage == INITSTAGE_LOCAL)
    {
        delaySignal = registerSignal("delay");
        roundSignal = registerSignal("round");
        EV_INFO << "Register RTT signal with ID: " << roundSignal << endl;
        EV_INFO << "Statistic path: " << getFullPath() << ".roundTripTime" << endl;

        //dataSetWriterId = par("dataSetWriterId");
        //messageReceiveTimeout = par("messageReceiveTimeout");
        //readerGroupExecTime = par("readerGroupExecTime");
        expectedSeqNr = 100;
        //actualSeqNr;
    }
}


void Subscriber::finish()
{

    UdpSink::finish();
}


void Subscriber::processStart()
{
    EV_INFO << "Binding local port to socket in Subscriber: " << localPort<< endl;
    EV_INFO << "Multicast group for SUbscriber: " << multicastGroup << endl;
    socket.setOutputGate(gate("socketOut"));
    socket.bind(localPort);
    setSocketOptions();


    if (stopTime >= SIMTIME_ZERO) {
        selfMsg->setKind(STOP);
        scheduleAt(stopTime, selfMsg);
    }
}

// This is the same as in UdpSink
void Subscriber::processPacket(Packet *pk)
{
    EV_INFO << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
    emit(packetReceivedSignal, pk);
    //numReceived++;

    const auto& payload = pk->peekData();
    auto tag = payload->findTag<PublisherTimeTag>();
    if (tag != nullptr) {
        publishTime = tag->getPublisherTime();
        EV_INFO << "publishTime = " << publishTime << " s\n";
     }
     else {
         EV_WARN << "No PublisherTimeTag found in payload!\n";
     }

    auto SeqNr = pk->peekAtFront<ApplicationPacket>();
    if (SeqNr != nullptr) {
        actualSeqNr = SeqNr->getSequenceNumber();
        EV_INFO << "Sequence number: " << actualSeqNr << endl;
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


void Subscriber::processDecoding()
{
    simtime_t end = simTime();
    simtime_t diff = end - publishTime;
    delay = diff.inUnit(SIMTIME_US);
    round = diff.inUnit(SIMTIME_US);
    EV_INFO << "RTT - In processReaderGroup: publishTime" << publishTime.inUnit(SIMTIME_US) << "us" << endl;
    EV_INFO << "RTT - In processReaderGroup: endTime" << end.inUnit(SIMTIME_US) << "us" << endl;
    EV_INFO << "RTT - In processReaderGroup: diff" << diff.inUnit(SIMTIME_US) << "us" << endl;

    EV_INFO << "In processReaderGroup for " << decodingExecTime << "us" << endl;
    numReceived++;

    if (publishTime == 0 || publishTime == lastPublishTime) {
        EV_INFO << "Node B did not send a new packet with a new PublishTime" << endl;
    }

    else {
        emit(delaySignal, delay);
        emit(roundSignal, round);
        EV_INFO << "Emitting RTT signal: " << round << " us" << endl;
     }

     lastPublishTime = publishTime;


    /*if (actualSeqNr == expectedSeqNr) {
        emit(delaySignal, delay);
        EV_INFO << "Emitting RTT signal: " << round << " us" << endl;
        emit(roundSignal, round);
        expectedSeqNr++;
    }
    else {
        EV_INFO << "SeqNr does not match" << endl;
        EV_INFO << "actualSeqNr: " << actualSeqNr << endl;
        EV_INFO << "expectedSeqNr: " << expectedSeqNr << endl;
    }*/
}

void Subscriber::setSocketOptions()
{
    UdpSink::setSocketOptions();

    // Explicitly join the multicast group
    //L3Address multicastAddr = L3AddressResolver().resolve("239.1.1.1");
    //socket.joinMulticastGroup(multicastAddr);
}


void Subscriber::handleMessageWhenUp(cMessage *msg)
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

