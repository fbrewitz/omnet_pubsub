package pubsub.simulations;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;
import inet.queueing.contract.ITrafficConditioner;


@license(LGPL);

// This is for the whole network and the actual simulation!! Define submodules and such
network PubSubSafetyNetwork2Switch
{
    parameters:

        @display("bgb=1116,543");
    submodules:
        nodeA: StandardHost {
            @display("p=175,236");
        }
        nodeB: StandardHost {
            @display("p=923,236");
        }
        configurator: Ipv4NetworkConfigurator { // to set IP addresses and such, writer more on why this is needed in the report

            @display("p=76,78");
        }
        switch1: EthernetSwitch {
            @display("p=434,235");
        }
        switch2: EthernetSwitch {
            @display("p=670,235");
        }
        nodeC: StandardHost {
            @display("p=434,376");
        }
        nodeD: StandardHost {
            @display("p=669,377");
        }

    connections:
        switch1.ethg++ <--> Eth100M <--> switch2.ethg++;
        nodeA.ethg++ <--> Eth100M {  delay = 2.65us; } <--> switch1.ethg++;
        switch2.ethg++ <--> Eth100M {  delay = 2.65us; } <--> nodeB.ethg++;
        switch1.ethg++ <--> Eth100M <--> nodeC.ethg++;
        switch2.ethg++ <--> Eth100M <--> nodeD.ethg++;

}


