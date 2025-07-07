#include <iostream>

#include "event_sim/event_sim.h"
#include "event_sim_ip/saos_core.h"

using namespace std;
using namespace event_sim;
using namespace event_sim::saos;
using namespace event_sim::saos::runtime;


int main(int argc, char *argv[]) {
    const int command_queue_depth        = 8;
    const int command_queue_window_limit = 8;

    const int pe_height = 32;
    const int pe_width  = 32;

    const int   ref_clock_freq          = 1;
    const int   mem_clock_freq          = 1;
    const int   dma_block_size          = 32;
    const int   dma_channel_num         = 2;
    const float dma_load_latency        = 10;
    const float dma_store_latency       = 10;
    const bool  dma_is_ddr              = true;
    const int   dma_request_queue_depth = 256;
    const int   dma_command_queue_depth = 8;

    const int   ifm_slot_num = 8;
    const int   wgt_slot_num = 8;
    const int   ofm_slot_num = 4;
    
    SAOSCore_SimpleDMA *device_p = new SAOSCore_SimpleDMA(
        command_queue_depth,
        command_queue_window_limit,
        pe_height,
        pe_width,
        ref_clock_freq,
        mem_clock_freq,
        dma_block_size,
        dma_channel_num,
        dma_load_latency,
        dma_store_latency,
        dma_is_ddr,
        dma_request_queue_depth,
        dma_command_queue_depth,
        ifm_slot_num,
        wgt_slot_num,
        ofm_slot_num
    );

    device_p->reset_context();
    device_p->set_single_command_cycle_thres(1000); // The simulation result will be undeterministic unless a single command takes less than 1000 cycles
    device_p->set_synchronization_cycle_thres(10000);  // The simulation result will be undeterministic unless the synchronization phase takes less than 10000 cycles

    const int M = 32, N = 32, K = 512;

    gemm_dense_nmk((SAOSCoreBase *)device_p, M, N, K, 1, 1, 4, true, true);

    cout << "simulation terminated at " << device_p->context.get_timestamp() << endl;

    delete device_p;

    return 0;
}