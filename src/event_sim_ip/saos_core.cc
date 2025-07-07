#include "saos_core.h"

namespace event_sim {
namespace saos {

SAOSCoreBase::SAOSCoreBase(
    const int command_queue_depth, 
    const int command_queue_window_limit,

    const int ifm_slot_num,
    const int wgt_slot_num,
    const int ofm_slot_num
): CoreO3(command_queue_depth, command_queue_window_limit) {
    this->ifm_slot_num      = ifm_slot_num;
    this->wgt_slot_num      = wgt_slot_num;
    this->ofm_slot_num      = ofm_slot_num;
    this->ifm_slot_offset   = 0;
    this->wgt_slot_offset   = this->ifm_slot_offset + this->ifm_slot_num;
    this->ofm_slot_offset   = this->wgt_slot_offset + this->wgt_slot_num;
}

slot_id_t SAOSCoreBase::get_ifm_slot_id(const int tile_id) const {
    return (slot_id_t)(this->ifm_slot_offset + tile_id);
}

slot_id_t SAOSCoreBase::get_wgt_slot_id(const int tile_id) const {
    return (slot_id_t)(this->wgt_slot_offset + tile_id);
}

slot_id_t SAOSCoreBase::get_ofm_slot_id(const int tile_id) const {
    return (slot_id_t)(this->ofm_slot_offset + tile_id);
}

void SAOSCoreBase::load(const cmd_arg_t addr, const cmd_arg_t size, const slot_id_t slot_id) {
    Command command = Command("load");
    command.args.push_back(addr);
    command.args.push_back(size);
    command.dst_dept = slot_id;
    command.module_dept = this->get_module_id("dma_engine");

    this->submit_command("dma_engine", command);
}

void SAOSCoreBase::store(const cmd_arg_t addr, const cmd_arg_t size, const slot_id_t slot_id) {
    Command command = Command("store");
    command.args.push_back(addr);
    command.args.push_back(size);
    command.src_dept.push_back(slot_id);
    command.module_dept = this->get_module_id("dma_engine");
    
    this->submit_command("dma_engine", command);
}

void SAOSCoreBase::preload(const slot_id_t slot_id) {
    Command command = Command("preload");
    command.src_dept.push_back(slot_id);
    command.module_dept = this->get_module_id("gemm_core");
    
    this->submit_command("gemm_core", command);
}

void SAOSCoreBase::execute(const cmd_arg_t seq_len, const slot_id_t ifm_slot_id, const slot_id_t wgt_slot_id) {
    Command command = Command("execute");
    command.args.push_back(seq_len);
    command.src_dept.push_back(ifm_slot_id);
    command.src_dept.push_back(wgt_slot_id);
    command.module_dept = this->get_module_id("gemm_core");
    
    this->submit_command("gemm_core", command);
}

void SAOSCoreBase::flush(const slot_id_t slot_id) {
    Command command = Command("flush");
    command.dst_dept = slot_id;
    command.module_dept = this->get_module_id("gemm_core");
    
    this->submit_command("gemm_core", command);
}


SAOSCore_SimpleDMA::SAOSCore_SimpleDMA(
    const int command_queue_depth,
    const int command_queue_window_limit,

    const int pe_height,
    const int pe_width,

    const int   ref_clock_freq,
    const int   mem_clock_freq,
    const int   dma_block_size,
    const int   dma_channel_num,
    const float dma_load_latency,
    const float dma_store_latency,
    const bool  dma_is_ddr,
    const int   dma_request_queue_depth,
    const int   dma_command_queue_depth,

    const int ifm_slot_num,
    const int wgt_slot_num,
    const int ofm_slot_num
): SAOSCoreBase (
    command_queue_depth,
    command_queue_window_limit,
    ifm_slot_num,
    wgt_slot_num,
    ofm_slot_num
) {
    this->gemm_core_p = new GEMMCoreSAOS(
        pe_height,
        pe_width
    );

    this->dma_engine_p = new DMAEngine(
        ref_clock_freq,
        mem_clock_freq,
        dma_block_size,
        dma_channel_num,
        dma_load_latency,
        dma_store_latency,
        dma_is_ddr,
        dma_request_queue_depth,
        dma_command_queue_depth
    );

    this->register_module("gemm_core",  (Module *)this->gemm_core_p );
    this->register_module("dma_engine", (Module *)this->dma_engine_p);
}

SAOSCore_SimpleDMA::~SAOSCore_SimpleDMA() {
    delete gemm_core_p;
    delete dma_engine_p;
}

int SAOSCore_SimpleDMA::get_pe_height() const{
    return this->gemm_core_p->pe_height;
}

int SAOSCore_SimpleDMA::get_pe_width() const{
    return this->gemm_core_p->pe_width;
}


SAOSCore_DRAMSim3::SAOSCore_DRAMSim3(
    const int command_queue_depth,
    const int command_queue_window_limit,

    const int pe_height,
    const int pe_width,

    const std::string& dma_config_file, 
    const std::string& dma_output_dir, 
    const long unsigned int dma_command_queue_depth,
    const long unsigned int dma_cacheline_size,

    const int ifm_slot_num,
    const int wgt_slot_num,
    const int ofm_slot_num
): SAOSCoreBase (
    command_queue_depth,
    command_queue_window_limit,
    ifm_slot_num,
    wgt_slot_num,
    ofm_slot_num
) {
    this->gemm_core_p = new GEMMCoreSAOS(
        pe_height,
        pe_width
    );

    this->dma_engine_p = new DRAMSim3DMAEngine(
        dma_config_file,
        dma_output_dir,
        dma_command_queue_depth,
        dma_cacheline_size
    );

    this->register_module("gemm_core",  (Module *)this->gemm_core_p );
    this->register_module("dma_engine", (Module *)this->dma_engine_p);
}

SAOSCore_DRAMSim3::~SAOSCore_DRAMSim3() {
    delete gemm_core_p;
    delete dma_engine_p;
}

int SAOSCore_DRAMSim3::get_pe_height() const{
    return this->gemm_core_p->pe_height;
}

int SAOSCore_DRAMSim3::get_pe_width() const{
    return this->gemm_core_p->pe_width;
}


namespace runtime {

void gemm_dense_nmk(
    SAOSCoreBase *device_p,

    int M,
    int N,
    int K,

    int ifm_wordsize,
    int wgt_wordsize,
    int ofm_wordsize,

    const bool use_bias,
    const bool verbose
) {
    Debugger *debugger_p;

    if (verbose) {
        debugger_p = new Debugger();
        device_p->register_debugger(debugger_p);
    }
    
    const int m_tile = device_p->get_pe_height();
    const int n_tile = device_p->get_pe_width();
    const int k_tile = device_p->get_pe_width();

    const int m_pad = (M % m_tile) ? (m_tile - (M % m_tile)) : 0;
    const int n_pad = (N % m_tile) ? (n_tile - (N % n_tile)) : 0;
    const int k_pad = (K % k_tile) ? (k_tile - (K % k_tile)) : 0;

    M += m_pad;
    N += n_pad;
    K += k_pad;

    const int m_tile_num = M / m_tile;
    const int n_tile_num = N / n_tile;
    const int k_tile_num = K / k_tile;

    const int k_tile_group_size     = std::min<int>(k_tile_num, std::min<int>(device_p->ifm_slot_num, device_p->wgt_slot_num));
    const int k_tile_group_num      = k_tile_num / k_tile_group_size;
    
    const int mn_tile_group_size    = std::min<int>(m_tile_num * n_tile_num, device_p->ofm_slot_num);

    if (verbose) {
        std::cout << "=== Runtime Summary ===" << std::endl;
        std::cout << "M: " << M << std::endl;
        std::cout << "N: " << N << std::endl;
        std::cout << "K: " << K << std::endl << std::endl;

        std::cout << "M tile: " << m_tile << std::endl;
        std::cout << "N tile: " << n_tile << std::endl;
        std::cout << "K tile: " << k_tile << std::endl << std::endl;

        std::cout << "# of M tiles: " << m_tile_num << std::endl;
        std::cout << "# of N tiles: " << n_tile_num << std::endl;
        std::cout << "# of K tiles: " << k_tile_num << std::endl << std::endl;

        std::cout << "K tile group size:  " << k_tile_group_size    << std::endl;
        std::cout << "# of K tile groups: " << k_tile_group_num     << std::endl;
        std::cout << "=== End Summary ===" << std::endl << std::endl;

    }

    for (int k_tile_group_idx = 0; k_tile_group_idx < k_tile_group_num; k_tile_group_idx++) {
        int k_tile_group_offset_max_limit = std::min<int>(k_tile_group_size, k_tile_num - (k_tile_group_idx * k_tile_group_size));

        for (int n_tile_idx = 0; n_tile_idx < n_tile_num; n_tile_idx++) {
            for (int m_tile_idx = 0; m_tile_idx < m_tile_num; m_tile_idx++) {
                int mn_tile_idx = n_tile_idx * m_tile_num + m_tile_idx;
                int mn_tile_group_offset = mn_tile_idx % mn_tile_group_size;

                int psum_id = device_p->get_ofm_slot_id(mn_tile_group_offset);
                int ofm_id  = device_p->get_ofm_slot_id(mn_tile_group_offset);
                
                if (k_tile_group_idx) {
                    device_p->store(0x00, m_tile*n_tile*ofm_wordsize, ofm_id);
                }

                for (int k_tile_group_offset = 0; k_tile_group_offset < k_tile_group_offset_max_limit; k_tile_group_offset++) {
                    // int k_tile_idx = k_tile_group_idx * k_tile_group_size + k_tile_group_offset;

                    int ifm_id = device_p->get_ifm_slot_id(k_tile_group_offset);
                    int wgt_id = device_p->get_wgt_slot_id(k_tile_group_offset);

                    bool bias_preload_required = use_bias && (k_tile_group_idx == 0) & (k_tile_group_offset == 0);
                    bool psum_preload_required = k_tile_group_offset == 0;
                    bool preload_required = bias_preload_required || psum_preload_required;
                    bool flush_required = k_tile_group_offset == (k_tile_group_offset_max_limit-1);

                    if (preload_required) {
                        device_p->load(0x00, m_tile*n_tile*ofm_wordsize, psum_id);
                        device_p->preload(psum_id);
                    }

                    device_p->load(0x00, n_tile*k_tile*wgt_wordsize, wgt_id);
                    device_p->load(0x00, m_tile*k_tile*ifm_wordsize, ifm_id);
                    device_p->execute(k_tile, ifm_id, wgt_id);

                    if (flush_required) {
                        device_p->flush(ofm_id);
                    }
                }

                if (k_tile_group_idx == (k_tile_group_num-1)) {
                    device_p->store(0x00, m_tile*n_tile*ofm_wordsize, ofm_id);
                }
            }
        }
    }

    device_p->synchronize();

    if (verbose) {
        delete debugger_p;
    }
}

}  // end of namespace: runtime
}  // end of namespace: saos
}  // end of namespace: event_sim