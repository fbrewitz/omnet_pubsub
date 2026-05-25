#include "SubscriberSafety.h"
#include "PublisherTimeTag_m.h"
#include "PublisherSafety.h"
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

// this is the new subscriber, to measure end-to-end latency in node B, and sends a packet to Pub in the same node, also includes timestamps so we can measure the time between
// Sub B and Pub B. Since it also sends a packet, in inherits from UdpBasicApp instead of UdpSink

namespace sub {

Define_Module(SubscriberSafety); // Is this necessary?? Documentation says so

// Override methods here
void SubscriberSafety::initialize(int stage)
{
    UdpBasicApp::initialize(stage); // to not skip the UdpSink setup for ports etc

    if (stage == INITSTAGE_LOCAL)
    {
        delaySignal = registerSignal("delay");

        //dataSetWriterId = par("dataSetWriterId");
        //messageReceiveTimeout = par("messageReceiveTimeout");
        //readerGroupExecTime = par("readerGroupExecTime");
        //seqNr;
    }
}


void SubscriberSafety::finish()
{
    UdpBasicApp::finish();
}


void SubscriberSafety::processStart()
{
    socket.setOutputGate(gate("socketOut"));
    const char *localAddress = par("localAddress");
    socket.bind(*localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);
    setSocketOptions();

    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;

    while ((token = tokenizer.nextToken()) != nullptr) {
        destAddressStr.push_back(token);
        L3Address result;
        L3AddressResolver().tryResolve(token, result);
        if (result.isUnspecified())
              EV_ERROR << "cannot resolve destination address: " << token << endl;
        destAddresses.push_back(result);
    }

    if (stopTime >= CLOCKTIME_ZERO) {
        selfMsg->setKind(STOP);
        scheduleClockEventAt(stopTime, selfMsg);
    }
}

// This is the same as in UdpSink
void SubscriberSafety::processPacket(Packet *pk)
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

        auto SeqNrPublisher = pk->peekAtFront<ApplicationPacket>();
        if (SeqNrPublisher != nullptr) {
            seqNr = SeqNrPublisher->getSequenceNumber();
            EV_INFO << "Sequence number in Sub B: " << seqNr << endl;
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



void SubscriberSafety::processDecoding()
{
    simtime_t end = simTime();
    simtime_t diff = end - publishTime;
    delay = diff.inUnit(SIMTIME_US);
    EV_INFO << "In processReaderGroup: publishTime" << publishTime.inUnit(SIMTIME_US) << "us" << endl;
    EV_INFO << "In processReaderGroup: endTime" << end.inUnit(SIMTIME_US) << "us" << endl;
    EV_INFO << "In processReaderGroup: diff in time" << diff.inUnit(SIMTIME_US) << "us" << endl;

    EV_INFO << "In processReaderGroup for " << decodingExecTime << "us" << endl;
    numReceived++;

    emit(delaySignal, delay);

    selfMsg->setKind(SEND);
    scheduleAt(simTime(), selfMsg);

    /*std::ostringstream str;
    str << packetName << "-" << numSent;
    Packet *pk = new Packet(str.str().c_str());
    if (dontFragment)
        pk->addTag<FragmentationReq>()->setDontFragment(true);
    const auto& payload = makeShared<ApplicationPacket>();
    payload->setChunkLength(B(par("messageLength")));
    payload->setSequenceNumber(numSent);
    payload->addTag<PublisherTimeTag>()->setPublisherTime(publishTime); // to measure RTT later in Sub node A
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime()); // to be used for delta in Pub B!!
    pk->insertAtBack(payload);
    L3Address destAddr = chooseDestAddr();

    // get the containing node
    cModule *node = getContainingNode(this);

    // Find the publisher module
    cModule *publisherModule = node->getSubmodule("app", 1);
    if (!publisherModule) {
        EV_ERROR << "Publisher module not found at index 1!\n";
        return;
    }

    // Cast to the expected type
    pub::PublisherSafety *publisher = dynamic_cast<pub::PublisherSafety *>(publisherModule);
    if (!publisher) {
        EV_ERROR << "Failed to cast to PublisherSafety type!\n";
        return;
    }

    // Use the interface
    publisher->processPacket(pk);*/
}


void SubscriberSafety::processSend()
{
    //startTimeSub = simTime();
    //EV_INFO << "In processWriterGroup: startTimePublish: " << startTimePublish.inUnit(SIMTIME_US) << "us" << endl;

    sendPacket();
    /*clocktime_t interval = par("sendInterval");
    if (stopTime < CLOCKTIME_ZERO || getClockTime() + interval < stopTime) {
        clocktime_t d = interval - lastWriterGroupExec;
        selfMsg->setKind(WG);
        scheduleClockEventAfter(d, selfMsg);
    }
    else {
        selfMsg->setKind(STOP);
        scheduleClockEventAt(stopTime, selfMsg);
    }*/
}

// Is it ok to create a new one to "copy" it to the Pub in B? They will have the same seqNum, and we can use the creationTime to measure the delta
void SubscriberSafety::sendPacket()
{
    std::ostringstream str;
    str << packetName << "-" << numSent;
    Packet *packet = new Packet(str.str().c_str());
    if (dontFragment)
        packet->addTag<FragmentationReq>()->setDontFragment(true);
    const auto& payload = makeShared<ApplicationPacket>();
    payload->setChunkLength(B(par("messageLength")));
    payload->setSequenceNumber(seqNr);
    payload->addTag<PublisherTimeTag>()->setPublisherTime(publishTime); // to measure RTT later in Sub node A
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime()); // to be used for delta in Pub B!!
    packet->insertAtBack(payload);
    L3Address destAddr = chooseDestAddr();
    emit(packetSentSignal, packet);
    socket.sendTo(packet, destAddr, destPort);
    numSent++;

    // get the containing node
      /*  cModule *node = getContainingNode(this);

        // Find the publisher module
        cModule *publisherModule = node->getSubmodule("app", 1);
        if (!publisherModule) {
            EV_ERROR << "Publisher module not found at index 1!\n";
            return;
        }

        // Cast to the expected type
        pub::PublisherSafety *publisher = dynamic_cast<pub::PublisherSafety *>(publisherModule);
        if (!publisher) {
            EV_ERROR << "Failed to cast to PublisherSafety type!\n";
            return;
        }

        // Use the interface
        publisher->processPacket(packet);*/
}

void SubscriberSafety::setSocketOptions()
{
    UdpBasicApp::setSocketOptions();

    // Explicitly join the multicast group
    L3Address multicastAddr = L3AddressResolver().resolve("239.1.1.1");
    socket.joinMulticastGroup(multicastAddr);
}

void SubscriberSafety::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    processPacket(packet);
}



void SubscriberSafety::handleMessageWhenUp(cMessage *msg)
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

            case SEND:
                processSend();
                break;

            case STOP:
                processStop();
                break;

            default:
                throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
         }
    }
    else
        socket.processMessage(msg);
}

}
