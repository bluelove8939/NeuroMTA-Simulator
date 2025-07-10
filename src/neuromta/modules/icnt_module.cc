#include "icnt_module.h"
#include "mta_trafficmanager.hpp"
#include "config_utils.hpp"

namespace neuromta {

InterconnectModuleBase::InterconnectModuleBase(
    const std::string& config_file,
    const long unsigned int command_queue_depth
): _command_queue_depth(command_queue_depth)
{
    Configuration config;
    vector<Network *> net;

    config.ParseFile(config_file);

    InitializeRoutingMap(config);

    int subnets = config.GetInt("subnets");
    net.resize(subnets);
    for (int i = 0; i < subnets; ++i) {
        net[i] = Network::New(config, "");
    }

    MTATrafficManagerInterface *p = new MTATrafficManagerInterface(config, net);

    this->_tfm_if_vp = (void *)p;
    this->_node_num = (unsigned long)(net[0]->NumNodes());
}

InterconnectModuleBase::~InterconnectModuleBase() {
    delete (MTATrafficManagerInterface *)this->_tfm_if_vp;
}

bool InterconnectModuleBase::issue_command(Command *command_p) {    
    if (this->_suspended_commands.size() < this->_command_queue_depth) {
        this->_suspended_commands.push(command_p);
        return true;
    }

    return false;
}

void InterconnectModuleBase::tick_clock() {
    MTATrafficManagerInterface *_tfm_if_p = (MTATrafficManagerInterface *)(this->_tfm_if_vp);

    _tfm_if_p->Step();
    
    if (this->_suspended_commands.size()) {
        MTAPacketDescriptor packet_desc;
        // Handle packet
        for (int n = 0; n < this->_node_num; n++) {
            if (_tfm_if_p->IsNodeBusy(n)) {
                packet_desc = _tfm_if_p->GetPacketDescriptor(n);
                
            }
        }

        // Send packet
        Command *command_p = this->_suspended_commands.front();
        
        int src_id = command_p->args[0];
        int dst_id = command_p->args[1];

        if (command_p->action == "data") {
            int addr        = command_p->args[2];
            int size        = command_p->args[3];
            int is_write    = command_p->args[4];
            
            packet_desc = MTAPacketDescriptor::NewDataPacket(addr, size, is_write, false);
        } else if (command_p->action == "control") {
            packet_desc = MTAPacketDescriptor::NewControlPacket(1, command_p->args, false);
        }

        _tfm_if_p->SendPacket(src_id, dst_id, 0, packet_desc);  // TODO: currently only supports a single subnet
    }
}

}