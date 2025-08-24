#include <iostream>
#include "neuromta/modules/saos_module.h"
#include "neuromta/modules/dma_module.h"
#include "neuromta/common/common.h"

using namespace std;
using namespace neuromta;


class SAOSCoreBase: public CoreO3 {
public:
    int ifm_slot_num;
    int wgt_slot_num;
    int ofm_slot_num;
    int ifm_slot_offset;
    int wgt_slot_offset;
    int ofm_slot_offset;

    SAOSCoreBase(
        const int command_queue_depth,
        const int command_queue_window_limit,

        const int ifm_slot_num,
        const int wgt_slot_num,
        const int ofm_slot_num
    );

    slot_id_t get_ifm_slot_id(const int tile_id) const;
    slot_id_t get_wgt_slot_id(const int tile_id) const;
    slot_id_t get_ofm_slot_id(const int tile_id) const;

    void load(const cmd_arg_t addr, const cmd_arg_t size, const slot_id_t slot_id);
    void store(const cmd_arg_t addr, const cmd_arg_t size, const slot_id_t slot_id);
    void preload(const slot_id_t slot_id);
    void execute(const cmd_arg_t seq_len, const slot_id_t ifm_slot_id, const slot_id_t wgt_slot_id);
    void flush(const slot_id_t slot_id);

    virtual int get_pe_height() const = 0;
    virtual int get_pe_width()  const = 0;
};

class SAOSCore_DRAMSim3: public SAOSCoreBase {
private:
    SystolicArrayOS   *gemm_core_p;
    DRAMSim3DMAModule *dma_engine_p;

public:
    SAOSCore_DRAMSim3(
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
    );

    virtual ~SAOSCore_DRAMSim3();

    virtual int get_pe_height() const override;
    virtual int get_pe_width()  const override;
};

void gemm_dense_nmk(
    SAOSCoreBase *device_p,

    int M,
    int N,
    int K,

    int ifm_wordsize,
    int wgt_wordsize,
    int ofm_wordsize,

    const bool use_bias=true,
    const bool verbose=true
);


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
    this->gemm_core_p = new SystolicArrayOS(
        pe_height,
        pe_width
    );

    this->dma_engine_p = new DRAMSim3DMAModule(
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


int main(int argc, char *argv[]) {
    if (!std::getenv("DRAMSIM3_CONFIG_DIR")) {
        cerr << "[ERROR] The environment variable \'DRAMSIM3_CONFIG_DIR\' is undefined. You may need to execute \'env.sh\' to properly set environment variables to use externalsmodules including DRAMSim3 and Booksim2." << endl;
        throw NeuroMTARuntimeException("Runtime", "environment exception");
    }

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

    device_p->reset();
    device_p->set_single_command_cycle_thres(1000); // The simulation result will be undeterministic unless a single command takes less than 1000 cycles
    device_p->set_synchronization_cycle_thres(10000);  // The simulation result will be undeterministic unless the synchronization phase takes less than 10000 cycles

    const int M = 32, N = 32, K = 512;

    gemm_dense_nmk((SAOSCoreBase *)device_p, M, N, K, 1, 1, 4, true, true);

    cout << "simulation terminated at " << device_p->context_p->get_timestamp() << endl;
    
    delete device_p;

    return 0;
}