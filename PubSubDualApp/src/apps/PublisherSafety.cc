#include "PublisherSafety.h"
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

Define_Module(PublisherSafety);


// To initialize my own parameters
void PublisherSafety::initialize(int stage)
{
    // To be able to initialize my own PubSub parameters
    UdpBasicApp::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        //publisherId = par("publisherId");
        //dataSetWriterId = par("dataSetWriterId");
        //publisherInterval = par("publisherInterval");

        deltaSignal = registerSignal("delta");
        //seqNr;
    }
}


// To add my own statistics
void PublisherSafety::finish()
{

    UdpBasicApp::finish();
}


void PublisherSafety::sendPacket()
{
    // Send pkt to Sub node A
    std::ostringstream str;
    str << packetName << "-" << numSent;
    Packet *packet = new Packet(str.str().c_str());
    if (dontFragment)
        packet->addTag<FragmentationReq>()->setDontFragment(true);
    const auto& payload = makeShared<ApplicationPacket>();
    payload->setChunkLength(B(par("messageLength")));
    payload->setSequenceNumber(seqNr);
    payload->addTag<PublisherTimeTag>()->setPublisherTime(publishTime); // so that the Subscriber in node A can measure RTT, reuse the same time
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime()); // perhaps edit this to use this for RTT instead, use another tag for delay in Pub B
    packet->insertAtBack(payload);
    L3Address destAddr = chooseDestAddr();
    emit(packetSentSignal, packet);
    socket.sendTo(packet, destAddr, destPort);
    numSent++;
}


// Override the function from the base class, keep almost all logic but remove the WriterGroup and Starts since it only sends upon "arrival" from the Sub
void PublisherSafety::processStart()
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
void PublisherSafety::processSend()
{
    //startTimePublish = simTime();
    //EV_INFO << "In processWriterGroup: startTimePublish: " << startTimePublish.inUnit(SIMTIME_US) << "us" << endl;

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


// Add to this later in order to add other timing characteristics as well
void PublisherSafety::processEncoding()
{
    // -> From processStart -> to processSend, this is in between!!
    publishedDataSet = par("publishedDataSet");
    dataSetWriter = par("dataSetWriter");
    writerGroup = par("writerGroup");
    processingSendTime = par("processingSendTime");

    lastEncodingExecTime = publishedDataSet + dataSetWriter + writerGroup + processingSendTime;
    EV_INFO << "In processWriterGroup: " << lastEncodingExecTime << "us" << endl;
    selfMsg->setKind(SEND);
    scheduleClockEventAfter(lastEncodingExecTime, selfMsg);
}


void PublisherSafety::processPacket(Packet *pk)
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

     auto tagCreation = payload->findTag<CreationTimeTag>();
     if (tagCreation != nullptr) {
         creationTime = tagCreation->getCreationTime();
         EV_INFO << "creationTime = " << creationTime << " s\n";
      }
      else {
         EV_WARN << "No CreationTimeTag found in payload!\n";
      }

     auto SeqNrPublisher = pk->peekAtFront<ApplicationPacket>();
     if (SeqNrPublisher != nullptr) {
         seqNr = SeqNrPublisher->getSequenceNumber();
         EV_INFO << "Sequence number in Pub node B, rec from Sub: " << seqNr << endl;
     }
     else {
         EV_WARN << "No sequenceNumber found payload!\n";
     }

    // get delta (time between this pub and the sub)
    simtime_t end = simTime();
    simtime_t diff = end - creationTime;
    delta = diff.inUnit(SIMTIME_NS);

    EV_INFO << "end time: " << end << " ns" << endl;
    EV_INFO << "Emitting delta signal: " << delta << " ns" << endl;
    emit(deltaSignal, delta);

    // Just emit the delta signal and do nothing, this will have its own sendInterval!! It does not depend on the SUbscriber
}


void PublisherSafety::setSocketOptions()
{
    UdpBasicApp::setSocketOptions();

    // Explicitly join the multicast group
    //L3Address multicastAddr = L3AddressResolver().resolve("239.1.1.1");
    //socket.joinMulticastGroup(multicastAddr);
}

void PublisherSafety::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    processPacket(packet);
}

// Only if I want to change the logic of self messages etc
void PublisherSafety::handleMessageWhenUp(cMessage *msg)
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






