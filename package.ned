package networkproject;

@license(LGPL);
//
// TODO documentation
//
network Network
{
       
    @display("bgb=425,329");
    submodules:
        coordinator: Coordinator {
            @display("p=206,123;i=block/cogwheel");
        }
        Node0: Node {
            @display("p=62,193;i=device/switch");
        }
        Node1: Node {
            @display("p=348,181;i=device/switch");
        }
    connections:
        coordinator.out0 --> Node0.coordin;
        coordinator.out1 --> Node1.coordin;
        Node1.out --> Node0.in;
        Node0.out --> Node1.in;
}

