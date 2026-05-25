

package pubsub.ned;

simple SubscriberEchoApp extends inet.applications.udpapp.UdpBasicApp
{
    // It is possible to define @default(..) values in NED files. However, a non-default value assigned from NED can not be overwritten later in NED or from ini files,
    // as the value becomes "hardcoded"
    parameters:
        @class(sub::SubscriberEchoApp);
        double subscribedDataSet @unit(s);
        double dataSetReader @unit(s);
        double readerGroup @unit(s);
        double processingRecTime @unit(s);

        @signal[latency](type = omnetpp::simtime_t);
		@statistic[endToEndLatency](title="endToEndLatency";source=latency; record=vector,histogram,count; unit=us; interpolationmode=none);

}