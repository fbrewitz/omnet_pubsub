
package pubsub.ned;
@license(LGPL);


simple SubscriberApp extends inet.applications.udpapp.UdpSink
{
    // It is possible to define @default(..) values in NED files. However, a non-default value assigned from NED can not be overwritten later in NED or from ini files,
    // as the value becomes "hardcoded"
    parameters:
        @class(sub::SubscriberApp);
        double subscribedDataSet @unit(s);
        double dataSetReader @unit(s);
        double readerGroup @unit(s);
        double processingRecTime @unit(s);

        @signal[latency](type = omnetpp::simtime_t);
		@statistic[endToEndLatency](title="endToEndLatency";source=latency; record=vector,histogram,count; unit=us; interpolationmode=none);

}