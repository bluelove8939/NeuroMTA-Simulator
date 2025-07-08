#include "dma_module.h"
#include "memory_system.h"    // DRAMSim3

namespace neuromta {

DMAModule::DMAModule(
    const int   ref_clock_freq,
    const int   mem_clock_freq,
    const int   dma_block_size,
    const int   channel_num,
    const float dma_load_latency,
    const float dma_store_latency,
    const bool  is_ddr,
    const int   request_queue_depth,
    const int   command_queue_depth
): _ref_clock_freq      (ref_clock_freq),
   _mem_clock_freq      (mem_clock_freq),
   _dma_block_size      (dma_block_size),
   _channel_num         (channel_num),
   _is_ddr              (is_ddr),
   _request_queue_depth (request_queue_depth),
   _command_queue_depth (command_queue_depth)
{
    this->_dma_load_cycles  = dma_load_latency  * this->_ref_clock_freq;
    this->_dma_store_cycles = dma_store_latency * this->_ref_clock_freq;

    this->_concurrent_request_cnt = 0;
}

bool DMAModule::issue_command(Command *command_p) {
    int req_num;
    int req_latency;
    
    if (this->_suspended_commands.size() < this->_command_queue_depth) {
        if (command_p->action == "load") {
            req_num = (command_p->args[1] / this->_channel_num) / this->_dma_block_size;
            req_latency = this->_dma_load_cycles; 
        } else if (command_p->action == "store") {
            req_num = (command_p->args[1] / this->_channel_num) / this->_dma_block_size;
            req_latency = this->_dma_store_cycles;
        } else {
            throw std::invalid_argument("Invalid argument");
        }

        this->_suspended_commands.push(command_p);
        this->_suspended_cycles.push(req_latency);
        this->_suspended_requests.push(req_num);

        return true;
    }

    return false;
}

void DMAModule::tick_clock() {
    if (this->_suspended_commands.size()) {
        if (this->_suspended_requests.front() > 0) {
            this->_suspended_requests.front()--;
            this->_concurrent_request_cnt++;
        }

        if (this->_suspended_requests.front() == 0) {
            if (this->_suspended_cycles.front() > 0) {
                this->_suspended_cycles.front()--;
            }

            if (this->_suspended_cycles.front() == 0) {
                this->_suspended_commands.front()->commit_time = this->context_p->get_timestamp();
                this->_suspended_commands.pop();
                this->_suspended_requests.pop();
                this->_suspended_cycles.pop();
            }
        }
    }
}



DRAMSim3DMAModule::DRAMSim3DMAModule(
    const std::string& config_file, 
    const std::string& output_dir, 
    const long unsigned int command_queue_depth,
    const long unsigned int cacheline_size
): _cacheline_size(cacheline_size),
   _command_queue_depth(command_queue_depth)
{
    dramsim3::MemorySystem *_msys_p = new dramsim3::MemorySystem(
        config_file, output_dir,
        std::bind(&DRAMSim3DMAModule::read_callback, this, std::placeholders::_1),
        std::bind(&DRAMSim3DMAModule::write_callback, this, std::placeholders::_1)
    );

    this->_memory_system_p = (void *)_msys_p;
}

DRAMSim3DMAModule::~DRAMSim3DMAModule() {
    dramsim3::MemorySystem *_msys_p = (dramsim3::MemorySystem *)(this->_memory_system_p);
    delete _msys_p;
}

void DRAMSim3DMAModule::read_callback(uint64_t addr) {
    if (this->_ongoing_rd_req_cmd_map.find(addr) == this->_ongoing_rd_req_cmd_map.end())
        return;

    while (this->_ongoing_rd_req_cmd_map[addr].size()) {
        Command *command_p = this->_ongoing_rd_req_cmd_map[addr].front();
        command_p->args[2]--;

        if (command_p->args[2] == 0)
            command_p->commit_time = this->context_p->get_timestamp();
        
        this->_ongoing_rd_req_cmd_map[addr].pop();
    }

    this->_ongoing_rd_req_cmd_map.erase(addr);
}

void DRAMSim3DMAModule::write_callback(uint64_t addr) {
    if (this->_ongoing_wr_req_cmd_map.find(addr) == this->_ongoing_wr_req_cmd_map.end())
        return;

    while (this->_ongoing_wr_req_cmd_map[addr].size()) {
        Command *command_p = this->_ongoing_wr_req_cmd_map[addr].front();
        command_p->args[2]--;

        if (command_p->args[2] == 0)
            command_p->commit_time = this->context_p->get_timestamp();
        
        this->_ongoing_wr_req_cmd_map[addr].pop();
    }

    this->_ongoing_wr_req_cmd_map.erase(addr);
}

void DRAMSim3DMAModule::print_memsys_stats() {
    dramsim3::MemorySystem *_msys_p = (dramsim3::MemorySystem *)(this->_memory_system_p);
    _msys_p->PrintStats();
}

bool DRAMSim3DMAModule::issue_command(Command *command_p) {    
    if (this->_suspended_commands.size() < this->_command_queue_depth) {
        auto req_st_addr = command_p->args[0];
        auto req_ed_addr = req_st_addr + command_p->args[1];

        uint32_t req_cnt = 0;
        for (uint64_t req_addr = req_st_addr; req_addr < req_ed_addr; req_addr = req_addr + this->_cacheline_size) {
            this->_suspended_request_addrs.push(req_addr);
            req_cnt++;
        }

        this->_suspended_commands.push(command_p);
        this->_suspended_request_cnt.push(req_cnt);

        while (command_p->args.size() < 3)
            command_p->args.push_back(0);
        command_p->args[2] = req_cnt;  // third temporary argument is the number of associated requests (read / write)

        return true;
    }

    return false;
}

void DRAMSim3DMAModule::tick_clock() {
    dramsim3::MemorySystem *_msys_p = (dramsim3::MemorySystem *)(this->_memory_system_p);

    _msys_p->ClockTick();
    
    if (this->_suspended_request_addrs.size()) {
        Command *command_p = this->_suspended_commands.front();

        uint64_t addr = this->_suspended_request_addrs.front();
        auto is_write = (command_p->action == "store");

        if (_msys_p->WillAcceptTransaction(addr, is_write)) {
            _msys_p->AddTransaction(addr, is_write);

            if (!is_write) {
                if (this->_ongoing_rd_req_cmd_map.find(addr) == this->_ongoing_rd_req_cmd_map.end())
                    this->_ongoing_rd_req_cmd_map[addr] = std::queue<Command *>();
                this->_ongoing_rd_req_cmd_map[addr].push(command_p);
            } else {
                if (this->_ongoing_wr_req_cmd_map.find(addr) == this->_ongoing_wr_req_cmd_map.end())
                    this->_ongoing_wr_req_cmd_map[addr] = std::queue<Command *>();
                this->_ongoing_wr_req_cmd_map[addr].push(command_p);
            }
            
            this->_suspended_request_addrs.pop();
            this->_suspended_request_cnt.front()--;
            
            if (this->_suspended_request_cnt.front() == 0) {
                this->_suspended_request_cnt.pop();
                this->_suspended_commands.pop();
            }
        }
    }
}

}