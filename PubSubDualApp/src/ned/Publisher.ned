
package pubsub.ned;
@license(LGPL);


simple Publisher extends inet.applications.udpapp.UdpBasicApp
{
    parameters:
        @class(pub::Publisher);
        double publishedDataSet @unit(s);
    	double dataSetWriter @unit(s);
    	double writerGroup @unit(s);
        double processingSendTime @unit(s);			
}
