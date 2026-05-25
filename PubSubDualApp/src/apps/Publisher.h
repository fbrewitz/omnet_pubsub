#pragma once

#include "inet/applications/udpapp/UdpBasicApp.h"
#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace pub {

class Publisher : public inet::UdpBasicApp
{
protected:
    enum SelfMsgKinds { START = 1, SEND, STOP, ENCODE };

    //parameters here
    //int publisherId;
    //int dataSetWriterId;
    //double publisherInterval;
    //int writerGroupId; // include this??
    //double writerGroupExecTime;
    //int messageLength;

    double lastEncodingExecTime;
    double publishedDataSet;
    double dataSetWriter;
    double writerGroup;
    double processingSendTime;

    int seqNr;

    omnetpp::simtime_t startTimePublish;
    //omnetpp::simtime_t publishTime;
    //omnetpp::simtime_t creationTime;
    //omnetpp::simtime_t subTime;
    //omnetpp::simtime_t delta;

private:
    //omnetpp::simsignal_t deltaSignal;

protected:
    // override methods here
    int numInitStages() const override { return UdpBasicApp::numInitStages(); }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void sendPacket() override;
    //virtual void processPacket(inet::Packet *pk) override;
    virtual void handleMessageWhenUp(inet::cMessage *msg) override;
    virtual void processStart() override;
    virtual void processSend() override;
    //virtual void setSocketOptions() override;
    //virtual void socketDataArrived(inet::UdpSocket *socket, inet::Packet *packet) override;
    void processEncoding();

};
}
