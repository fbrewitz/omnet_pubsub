#pragma once

#include "inet/applications/udpapp/UdpSink.h"
//#include "inet/applications/udpapp/UdpBasicApp.h"
#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3AddressResolver.h"


namespace sub {

class Subscriber : public inet::UdpSink
{
protected:
    enum SelfMsgKinds { START = 1, STOP, DECODE };
    //parameters here
    //int publisherId;
    //int dataSetWriterId; // If the value is 0 (null), the parameter shall be ignored and all received DataSetMessages pass the DataSetWriterId filter
    //double messageReceiveTimeout; // only necessary if I use the "corresponding" parameters in publisher?

    double decodingExecTime;
    double subscribedDataSet;
    double dataSetReader;
    double readerGroup;
    double processingRecTime;
    omnetpp::simtime_t publishTime;
    omnetpp::simtime_t round;
    omnetpp::simtime_t delay;
    omnetpp::simtime_t lastPublishTime = 0;

    int expectedSeqNr;
    int actualSeqNr = -1;

private:
    omnetpp::simsignal_t roundSignal;
    omnetpp::simsignal_t delaySignal;

protected:
    // override methods here
    int numInitStages() const override { return UdpSink::numInitStages(); }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessageWhenUp(inet::cMessage *msg) override;
    //virtual void socketDataArrived(inet::UdpSocket *socket, inet::Packet *packet) override;
    virtual void setSocketOptions() override;
    virtual void processStart() override;
    virtual void processPacket(inet::Packet *pk) override;
    void processDecoding();

};

}
