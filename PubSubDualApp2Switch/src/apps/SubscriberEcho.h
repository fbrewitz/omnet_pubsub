#pragma once

#include "inet/applications/udpapp/UdpBasicApp.h"
#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3AddressResolver.h"


namespace sub {

class SubscriberEchoApp : public inet::UdpBasicApp
{
protected:
    enum SelfMsgKinds { START = 1, STOP, DECODE, SEND };

    double decodingExecTime;
    double subscribedDataSet;
    double dataSetReader;
    double readerGroup;
    double processingRecTime;
    omnetpp::simtime_t publishTime;
    omnetpp::simtime_t latency;

private:
    omnetpp::simsignal_t latencySignal;

protected:
    int numInitStages() const override { return UdpBasicApp::numInitStages(); }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessageWhenUp(inet::cMessage *msg) override;
    virtual void processStart() override;
    virtual void processSend() override;
    virtual void processPacket(inet::Packet *pk) override;
    virtual void setSocketOptions() override;
    virtual void socketDataArrived(inet::UdpSocket *socket, inet::Packet *packet) override;
    virtual void sendPacket() override;
    void processDecoding();

};

}
