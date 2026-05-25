
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

			
}
