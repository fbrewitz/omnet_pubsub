package pubsub.simulations;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;


@license(LGPL);

// This is for the whole network and the actual simulation!! Define submodules and such
network PubSubSafetyNetwork
{
    parameters:
        int numNodeA;
        int numNodeB;

        @display("bgb=823,401");
    submodules:
        nodeA[numNodeA]: StandardHost {
            @display("p=120,202");
        }
        nodeB[numNodeB]: StandardHost {
            @display("p=703,202");
        }
        configurator: Ipv4NetworkConfigurator { // to set IP addresses and such, writer more on why this is needed in the report

            @display("p=537,81");
        }
        switch: EthernetSwitch {
           @display("p=379,201");
        }
    connections:
        // difference between switch.ethg[0] and switch.ethg++ - OMNeT++ thinks the link is subscribted, but it is not. Replace with incremental type.
        for j=0..numNodeA-1 {
            nodeA[j].ethg++ <--> Eth100M {  delay = 2.65us;  } <--> switch.ethg++;
        }

        for i=0..numNodeB-1 {
            switch.ethg++ <--> Eth100M {  delay = 2.65us;  } <--> nodeB[i].ethg++;
        }

}


