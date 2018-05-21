// Simulation NED file
//
// Author: Leonardo Bonati
// Created on Feb. 2018
//

package csma_ca;

// Sensor node
simple SensorNodeCSMACA
{
    parameters:
        int T_value @unit(s) = default(5.0s);
        double T_cca_value @unit(s) = default(0.000128s);
        double D_bp_value @unit(s) = default(0.00032s);
        int macMinBE_value @prompt("macMinBE") = default(3);
        int macMaxBE_value @prompt("macMaxBE") = default(4);
        int macMaxCSMABackoffs_value @prompt("macMaxCSMABackoffs") = default(2);
        double D_p_value @unit(s) = default(0.004256s);
        int pkt_to_send_value = default(1000);
        double p_tx_value @unit(W) = default(0.0495W);
        double p_rx_value @unit(W) = default(0.0564W);
        @display("i=block/source");
    gates:
        output out;
}

// Sink
simple SinkNodeCSMACA
{
    parameters:
        @display("i=block/sink");
    gates:
        input in[];
}

// Network
network SourceSink
{
    parameters:
        int n @prompt("Number of nodes") = default(5);
        bool channel_free = default(true);
        int concurrent_tx = default(0);
        double latency = default(0);
        int dropped_pkts = default(0);
        int tx_pkts = default(0);
        double energy = default(0);
    submodules:
        source[n]: SensorNodeCSMACA {
        }
        sink: SinkNodeCSMACA {
        }
    connections:
        for i=0..n-1 {
            source[i].out --> sink.in++;
        }
}
