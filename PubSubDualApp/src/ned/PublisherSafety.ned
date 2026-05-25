

package pubsub.ned;


simple PublisherSafety extends inet.applications.udpapp.UdpBasicApp
{
    parameters:
        @class(pub::PublisherSafety);
        double publishedDataSet @unit(s);
    	double dataSetWriter @unit(s);
    	double writerGroup @unit(s);
        double processingSendTime @unit(s);

        @signal[delta](type = omnetpp::simtime_t);
		@statistic[delta](title="delta";source=delta; record=vector,histogram,count; unit=ns; interpolationmode=none);
			
}