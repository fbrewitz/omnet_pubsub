
package pubsub.ned;
@license(LGPL);


simple PublisherApp extends inet.applications.udpapp.UdpBasicApp
{
    parameters:
        @class(pub::PublisherApp);
        double publishedDataSet @unit(s);
    	double dataSetWriter @unit(s);
    	double writerGroup @unit(s);
        double processingSendTime @unit(s);
			
		// See Subscriber.ned for info on how to declare statistics and signals
}
