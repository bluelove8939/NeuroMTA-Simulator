#ifndef __EVENT_SIM_DMA_ENGINE_H
#define __EVENT_SIM_DMA_ENGINE_H

#include <queue>
#include <map>

#include "event_sim.h"

namespace event_sim {

class DMAEngine: public Module {
protected:
    const int   _ref_clock_freq;
    const int   _mem_clock_freq;
    const int   _dma_block_size;
    const int   _channel_num;
    const bool  _is_ddr;

    const long unsigned int _request_queue_depth;
    const long unsigned int _command_queue_depth;

    int _dma_load_cycles;
    int _dma_store_cycles;

    long unsigned int _concurrent_request_cnt;
    std::queue<Command *> _suspended_commands;
    std::queue<int>       _suspended_requests;
    std::queue<int>       _suspended_cycles;

public:
    DMAEngine(
        const int   ref_clock_freq,
        const int   mem_clock_freq,
        const int   dma_block_size,
        const int   channel_num,
        const float dma_load_latency,
        const float dma_store_latency,
        const bool  is_ddr,
        const int   request_queue_depth,
        const int   command_queue_depth
    );

    virtual bool issue_command(Command *command_p)  override;
    virtual void tick_clock()                       override;
};

class HBM2DMAEngine: protected DMAEngine {
public:
    HBM2DMAEngine(
        const int ref_clock_freq,
        const int request_queue_depth=400,
        const int command_queue_depth=8
    );
};

class DRAMSim3DMAEngine: public Module {
    protected:
        void *_memory_system_p;  // TODO: this pointer is originally 'dramsim3::MemorySystem *' but declared as 'void *' to eliminate build dependency of DRAMSim3
    
        std::queue<Command*>    _suspended_commands;        // commands to be issued by the DMA engine (pop when all the associated requests are issued by the DMA engine (memory system of the DRAMSim3))
        std::queue<cmd_arg_t>   _suspended_request_addrs;   // requests to be transferred to the DRAMSim3's memory system 
        std::queue<uint32_t>    _suspended_request_cnt;     // the number of associated requests
    
        std::map<uint64_t, std::queue<Command *>> _ongoing_rd_req_cmd_map;     // maps to the Command associated with the finished read request  (used in 'read_callback'  method) 
        std::map<uint64_t, std::queue<Command *>> _ongoing_wr_req_cmd_map;     // maps to the Command associated with the finished write request (used in 'write_callback' method)
    
        const long unsigned int _cacheline_size;        // granularity of memory request transferred to the DRAMSim3's memory system
        const long unsigned int _command_queue_depth;   // the depth of the command queue ('_suspended_commands' queue)
    
    public:
        DRAMSim3DMAEngine(
            const std::string& config_file, 
            const std::string& output_dir, 
            const long unsigned int command_queue_depth,
            const long unsigned int cacheline_size
        );
    
        virtual ~DRAMSim3DMAEngine();
        
        void read_callback(uint64_t addr);
        void write_callback(uint64_t addr);
        void print_memsys_stats();
    
        virtual bool issue_command(Command *command_p)  override;
        virtual void tick_clock()                       override;
    };

}

#endif