package pubsub.simulations;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;


@license(LGPL);

// This is for the whole network and the actual simulation!! Define submodules and such
network PubSubSafetyNetworkPtP
{
    parameters:

        @display("bgb=823,401");
    submodules:
        nodeA: StandardHost {
            @display("p=120,202");
        }
        nodeB: StandardHost {
            @display("p=703,202");
        }
        configurator: Ipv4NetworkConfigurator { // to set IP addresses and such, writer more on why this is needed in the report

            @display("p=537,81");
        }

    connections:
		// Point-to-Point
		nodeA.ethg++ <--> Eth100M {
            	delay = 5.3us;
        } <--> nodeB.ethg++;



}


