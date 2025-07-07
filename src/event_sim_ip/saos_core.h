#ifndef __EVENT_SIM_SAOS_DEVICE_H
#define __EVENT_SIM_SAOS_DEVICE_H

#include <stdexcept>

#include "event_sim.h"
#include "saos.h"
#include "dma_engine.h"


namespace event_sim {
namespace saos {

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

class SAOSCore_SimpleDMA: public SAOSCoreBase {
private:
    GEMMCoreSAOS *gemm_core_p;
    DMAEngine    *dma_engine_p;

public:
    SAOSCore_SimpleDMA(
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
    );

    virtual ~SAOSCore_SimpleDMA();

    virtual int get_pe_height() const override;
    virtual int get_pe_width()  const override;
};

class SAOSCore_DRAMSim3: public SAOSCoreBase {
private:
    GEMMCoreSAOS        *gemm_core_p;
    DRAMSim3DMAEngine   *dma_engine_p;

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

namespace runtime {

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

}  // end of namespace: runtime
}  // end of namespace: saos
}  // end of namespace: event_sim

#endif