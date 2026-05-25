#pragma once

#include "inet/applications/udpapp/UdpSink.h"
#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3AddressResolver.h"


namespace sub {

class SubscriberApp : public inet::UdpSink
{
protected:
    enum SelfMsgKinds { START = 1, STOP, DECODE };
    //parameters here
    double decodingExecTime;
    double subscribedDataSet;
    double dataSetReader;
    double readerGroup;
    double processingRecTime;
    omnetpp::simtime_t publishTime;
    omnetpp::simtime_t round;
    omnetpp::simtime_t delay;
    omnetpp::simtime_t lastPublishTime = 0;

private:
    omnetpp::simsignal_t roundSignal;
    omnetpp::simsignal_t delaySignal;

protected:
    // override methods here
    int numInitStages() const override { return UdpSink::numInitStages(); }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessageWhenUp(inet::cMessage *msg) override;
    virtual void setSocketOptions() override;
    virtual void processStart() override;
    virtual void processPacket(inet::Packet *pk) override;
    void processDecoding();

};

}
