#include "Publisher.h"
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

Define_Module(PublisherApp);


// To initialize my own parameters
void PublisherApp::initialize(int stage)
{
    // To be able to initialize my own PubSub parameters
    UdpBasicApp::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
    }
}


// To add my own statistics
void PublisherApp::finish()
{

    UdpBasicApp::finish();
}


void PublisherApp::sendPacket()
{
    std::ostringstream str;
    str << packetName << "-" << numSent;
    Packet *packet = new Packet(str.str().c_str());
    if (dontFragment)
        packet->addTag<FragmentationReq>()->setDontFragment(true);
    const auto& payload = makeShared<ApplicationPacket>();
    payload->setChunkLength(B(par("messageLength")));
    payload->setSequenceNumber(numSent);
    payload->addTag<PublisherTimeTag>()->setPublisherTime(startTimePublish);
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);
    L3Address destAddr = chooseDestAddr();
    emit(packetSentSignal, packet);
    socket.sendTo(packet, destAddr, destPort);
    numSent++;

}


void PublisherApp::processStart()
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

    // This is edited in my version
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


void PublisherApp::processSend()
{
    startTimePublish = simTime();
    EV_INFO << "In processWriterGroup: startTimePublish: " << startTimePublish.inUnit(SIMTIME_US) << "us" << endl;

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


void PublisherApp::processEncoding()
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



// See header file for adding enums
void PublisherApp::handleMessageWhenUp(cMessage *msg)
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
        socket.processMessage(msg);
}

}






