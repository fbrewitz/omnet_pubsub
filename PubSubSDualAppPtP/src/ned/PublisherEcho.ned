
package pubsub.ned;


simple PublisherEchoApp extends inet.applications.udpapp.UdpBasicApp
{
    parameters:
        @class(pub::PublisherEchoApp);
        double publishedDataSet @unit(s);
    	double dataSetWriter @unit(s);
    	double writerGroup @unit(s);
        double processingSendTime @unit(s);
			
}