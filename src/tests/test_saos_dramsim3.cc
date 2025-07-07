#include <iostream>

#include "event_sim/event_sim.h"
#include "event_sim_ip/saos_core.h"

using namespace std;
using namespace event_sim;
using namespace event_sim::saos;
using namespace event_sim::saos::runtime;


int main(int argc, char *argv[]) {
    const string dramsim_config_dir = std::getenv("DRAMSIM3_CONFIG_DIR");
    const string dramsim_default_out_dir = std::getenv("DRAMSIM3_DEFAULT_OUT_DIR");

    const int command_queue_depth        = 8;
    const int command_queue_window_limit = 8;

    const int pe_height = 32;
    const int pe_width  = 32;

    const string dma_config_file = dramsim_config_dir + "/HBM2_8Gb_x128.ini";
    const string dma_output_dir  = dramsim_default_out_dir;
    const long unsigned int dma_command_queue_depth = 32;
    const long unsigned int dma_cacheline_size = 128;

    const int   ifm_slot_num = 8;
    const int   wgt_slot_num = 8;
    const int   ofm_slot_num = 4;
    
    SAOSCore_DRAMSim3 *device_p = new SAOSCore_DRAMSim3(
        command_queue_depth,
        command_queue_window_limit,
        pe_height,
        pe_width,
        dma_config_file,
        dma_output_dir,
        dma_command_queue_depth,
        dma_cacheline_size,
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