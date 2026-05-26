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



namespace pub {

Define_Module(PublisherEchoApp);


// To initialize my own parameters
void PublisherEchoApp::initialize(int stage)
{
    // To be able to initialize my own PubSub parameters
    UdpBasicApp::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
    }
}


// To add my own statistics
void PublisherEchoApp::finish()
{

    UdpBasicApp::finish();
}


void PublisherEchoApp::sendPacket()
{
    // Send pkt to Sub node A
    std::ostringstream str;
    str << packetName << "-" << numSent;
    Packet *packet = new Packet(str.str().c_str());
    if (dontFragment)
        packet->addTag<FragmentationReq>()->setDontFragment(true);
    const auto& payload = makeShared<ApplicationPacket>();
    payload->setChunkLength(B(par("messageLength")));
    payload->setSequenceNumber(numSent);
    payload->addTag<PublisherTimeTag>()->setPublisherTime(publishTime); // so that the Subscriber in node A can measure RTT, reuse the same time
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime()); 
    packet->insertAtBack(payload);
    L3Address destAddr = chooseDestAddr();
    emit(packetSentSignal, packet);
    socket.sendTo(packet, destAddr, destPort);
    numSent++;
}


void PublisherEchoApp::processStart()
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

    if (!destAddresses.empty()) {
        selfMsg->setKind(ENCODE);
        processEncoding();
     }

    else {
        if (stopTime >= CLOCKTIME_ZERO) {
            selfMsg->setKind(STOP);
            scheduleClockEventAt(stopTime, selfMsg);
        }
    }
}


// To add my own logic for timing parameters
void PublisherEchoApp::processSend()
{
    sendPacket();
    clocktime_t interval = par("sendInterval");
    if (stopTime < CLOCKTIME_ZERO || getClockTime() + interval < stopTime) {
        clocktime_t d = interval - lastEncodingExecTime;
        selfMsg->setKind(ENCODE);
        scheduleClockEventAfter(d, selfMsg);
    }
    else {
        selfMsg->setKind(STOP);
         scheduleClockEventAt(stopTime, selfMsg);
    }
}


void PublisherEchoApp::processEncoding()
{
    publishedDataSet = par("publishedDataSet");
    dataSetWriter = par("dataSetWriter");
    writerGroup = par("writerGroup");
    processingSendTime = par("processingSendTime");

    lastEncodingExecTime = publishedDataSet + dataSetWriter + writerGroup + processingSendTime;
    EV_INFO << "In processWriterGroup: " << lastEncodingExecTime << "us" << endl;
    selfMsg->setKind(SEND);
    scheduleClockEventAfter(lastEncodingExecTime, selfMsg);
}


void PublisherEchoApp::processPacket(Packet *pk)
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
}


void PublisherEchoApp::setSocketOptions()
{
    UdpBasicApp::setSocketOptions();
}

void PublisherEchoApp::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    processPacket(packet);
}


void PublisherEchoApp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
            case START:
                processStart();
                break;

            case ENCODE:
                processEncoding();
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
        socket.processMessage(msg); // If we receive packet
}

}






