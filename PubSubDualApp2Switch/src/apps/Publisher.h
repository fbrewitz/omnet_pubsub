#pragma once

#include "inet/applications/udpapp/UdpBasicApp.h"
#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace pub {

class PublisherApp : public inet::UdpBasicApp
{
protected:
    enum SelfMsgKinds { START = 1, SEND, STOP, ENCODE };

    double lastEncodingExecTime;
    double publishedDataSet;
    double dataSetWriter;
    double writerGroup;
    double processingSendTime;
    omnetpp::simtime_t startTimePublish;


protected:
    int numInitStages() const override { return UdpBasicApp::numInitStages(); }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void sendPacket() override;
    virtual void handleMessageWhenUp(inet::cMessage *msg) override;
    virtual void processStart() override;
    virtual void processSend() override;
    void processEncoding();
};
}
