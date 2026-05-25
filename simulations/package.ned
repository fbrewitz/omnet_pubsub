package pubsub.simulations;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetLink;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


@license(LGPL);

// This is for the whole network and the actual simulation! Define submodules and such, this is a compound module and the actual simulation model
network PubSubNetwork
{
    parameters:
        int numSubscribers;
        int numPublishers;

        @display("bgb=823,401");
    submodules:
        publisher[numPublishers]: StandardHost {
            @display("p=120,202");
        }
        subscriber[numSubscribers]: StandardHost {
            @display("p=703,202");
        }
        configurator: Ipv4NetworkConfigurator { // to set IP addresses and such

            @display("p=537,81");
        }
        switch: EthernetSwitch {
            @display("p=379,201");
        }
    connections:
        for j=0..numPublishers-1 {
            publisher[j].ethg++ <--> Eth100M{ delay = 2.65us; } <--> switch.ethg++;
        }

        for i=0..numSubscribers-1 {
            switch.ethg++ <--> Eth100M { delay = 2.65us; } <-->  subscriber[i].ethg++;
        }
}


