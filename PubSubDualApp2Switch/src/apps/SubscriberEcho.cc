#include "SubscriberEcho.h"

#include "PublisherEcho.h"
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

// this is the new subscriber, to measure end-to-end latency in node B, and sends a packet to Pub in the same node, also includes timestamps so we can measure the time between
// Sub B and Pub B. Since it also sends a packet, in inherits from UdpBasicApp instead of UdpSink

namespace sub {

Define_Module(SubscriberEchoApp);

void SubscriberEchoApp::initialize(int stage)
{
    UdpBasicApp::initialize(stage); // to not skip the UdpSink setup for ports etc

    if (stage == INITSTAGE_LOCAL)
    {
        latencySignal = registerSignal("latency");
    }
}


void SubscriberEchoApp::finish()
{
    UdpBasicApp::finish();
}


void SubscriberEchoApp::processStart()
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
void SubscriberEchoApp::processPacket(Packet *pk)
{
    EV_INFO << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
    emit(packetReceivedSignal, pk);
    numReceived++;

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



void SubscriberEchoApp::processDecoding()
{
    simtime_t end = simTime();
    simtime_t diff = end - publishTime;
    latency = diff.inUnit(SIMTIME_US);

    emit(latencySignal, latency);

    selfMsg->setKind(SEND);
    scheduleAt(simTime(), selfMsg);
}


void SubscriberEchoApp::processSend()
{
    sendPacket();
}


void SubscriberEchoApp::sendPacket()
{
    std::ostringstream str;
    str << packetName << "-" << numSent;
    Packet *packet = new Packet(str.str().c_str());
    if (dontFragment)
        packet->addTag<FragmentationReq>()->setDontFragment(true);
    const auto& payload = makeShared<ApplicationPacket>();
    payload->setChunkLength(B(par("messageLength")));
    payload->setSequenceNumber(numSent);
    payload->addTag<PublisherTimeTag>()->setPublisherTime(publishTime); // to measure RTT later in Sub node A
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);
    L3Address destAddr = chooseDestAddr();
    emit(packetSentSignal, packet);
    socket.sendTo(packet, destAddr, destPort);
    numSent++;
}

void SubscriberEchoApp::setSocketOptions()
{
    UdpBasicApp::setSocketOptions();

    // Explicitly join the multicast group
    L3Address multicastAddr = L3AddressResolver().resolve("239.1.1.1");
    socket.joinMulticastGroup(multicastAddr);
}

void SubscriberEchoApp::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    processPacket(packet);
}



void SubscriberEchoApp::handleMessageWhenUp(cMessage *msg)
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
