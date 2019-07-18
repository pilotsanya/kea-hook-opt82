#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include <log/macros.h>
#include <log/logger_support.h>
#include <util/encode/hex.h>
#include <util/io_utilities.h>

#include <string>
#include <vector>

using namespace isc::dhcp;
using namespace isc::hooks;
using namespace isc::util::encode;

typedef std::map<uint8_t, OptionBuffer> OptionBufferMap;
isc::log::Logger option82_logger("option82_hook");

extern "C" {

int load(LibraryHandle& handle) {
    return 0;
}

int version() {
    return (KEA_HOOKS_VERSION);
}

/* IPv4 callouts */
int host4_identifier(CalloutHandle& handle) {
    //std::vector<std::string> env;
    Pkt4Ptr pkt4;
    OptionBuffer circuit_id, remote_id;
    handle.getArgument("query4", pkt4);

    OptionPtr rai = pkt4->getOption(DHO_DHCP_AGENT_OPTIONS);
    if (rai) {
        LOG_DEBUG(option82_logger, 99, "DHO_DHCP_AGENT_OPTIONS");
        LOG_DEBUG(option82_logger, 99, "Try get suboptions 1,2");
        OptionPtr circuit_id_opt = rai->getOption(RAI_OPTION_AGENT_CIRCUIT_ID);
        OptionPtr remote_id_opt = rai->getOption(RAI_OPTION_REMOTE_ID);
        if (remote_id_opt && circuit_id_opt) {
            const OptionBuffer& circuit_id_vec = circuit_id_opt->getData();
            const OptionBuffer& remote_id_vec = remote_id_opt->getData();

            circuit_id.assign(circuit_id_vec.begin(), circuit_id_vec.end());
            remote_id.assign(remote_id_vec.begin(), remote_id_vec.end());
        }

        if (!circuit_id.empty() && !remote_id.empty()) {
            LOG_INFO(option82_logger, "CircuitID = %1").arg(encodeHex(circuit_id));
            LOG_INFO(option82_logger, "RemoteID = %1").arg(encodeHex(remote_id));
                        //Add last byte of circuit_id to remote_id

            //int size = std::min((int)remote_id.size(), 6);
            //OptionBuffer opt82_vec(remote_id.end() - size, remote_id.end());
            //sibset
            const std::vector<uint8_t> delimeter = { 0x31, 0x30, 0x2e };
            auto it = std::search(remote_id.begin(), remote_id.end(), delimeter.begin(), delimeter.end());
            if (it == remote_id.end()) {
                int size = std::min((int)remote_id.size(), 6);
                OptionBuffer opt82_vec(remote_id.end() - size, remote_id.end());
                opt82_vec.push_back(0x2d);
                opt82_vec.push_back(circuit_id.back());
                LOG_INFO(option82_logger, "FlexID = %1").arg(encodeHex(opt82_vec));
                handle.setArgument("id_value", opt82_vec);
                return 0;
            }
            OptionBuffer opt82_vec(it, remote_id.end());
            opt82_vec.push_back(0x2d);
            //sibset
            opt82_vec.push_back(circuit_id.back());

            LOG_INFO(option82_logger, "FlexID = %1").arg(encodeHex(opt82_vec));

            handle.setArgument("id_value", opt82_vec);
            return 0;
        }
    }

    return 0;
}

//change opt61 to ip+port
int pkt4_receive(CalloutHandle& handle) {
    Pkt4Ptr pkt4;
    OptionBuffer circuit_id, remote_id, cl_id;
    handle.getArgument("query4", pkt4);
    
    OptionPtr rai = pkt4->getOption(DHO_DHCP_AGENT_OPTIONS);

    if (rai) {
        OptionPtr clid = pkt4->getOption(DHO_DHCP_CLIENT_IDENTIFIER);
        //if (clid) {
            //const OptionBuffer& cl_id_vec = clid->getData();
            //cl_id.assign(cl_id_vec.begin(), cl_id_vec.end());
            //LOG_INFO(option82_logger, "Original ClientID = %1").arg(encodeHex(cl_id));
            //pkt4->delOption(DHO_DHCP_CLIENT_IDENTIFIER);
        //}
        OptionPtr circuit_id_opt = rai->getOption(RAI_OPTION_AGENT_CIRCUIT_ID);
        OptionPtr remote_id_opt = rai->getOption(RAI_OPTION_REMOTE_ID);
        if (remote_id_opt && circuit_id_opt) {
            const OptionBuffer& circuit_id_vec = circuit_id_opt->getData();
            const OptionBuffer& remote_id_vec = remote_id_opt->getData();

            circuit_id.assign(circuit_id_vec.begin(), circuit_id_vec.end());
            remote_id.assign(remote_id_vec.begin(), remote_id_vec.end());
        }
        if (!circuit_id.empty() && !remote_id.empty()) {
                    //LOG_INFO(option82_logger, "CircuitID = %1").arg(encodeHex(circuit_id));
            //LOG_INFO(option82_logger, "RemoteID = %1").arg(encodeHex(remote_id));
            //Add last byte of circuit_id to remote_id

            //int size = std::min((int)remote_id.size(), 6);
            //OptionBuffer opt82_vec(remote_id.end() - size, remote_id.end());
            //sibset
            const std::vector<uint8_t> delimeter = { 0x31, 0x30, 0x2e };
            auto it = std::search(remote_id.begin(), remote_id.end(), delimeter.begin(), delimeter.end());
            if (it == remote_id.end()) {
                int size = std::min((int)remote_id.size(), 6);
                OptionBuffer opt82_vec(remote_id.end() - size, remote_id.end());
                opt82_vec.push_back(0x2d);
                opt82_vec.push_back(circuit_id.back());
                LOG_INFO(option82_logger, "ClientID = %1").arg(encodeHex(opt82_vec));
                if (clid) {
                    pkt4->delOption(DHO_DHCP_CLIENT_IDENTIFIER);
                }
                clid.reset(new Option(Option::V4,
                                    DHO_DHCP_CLIENT_IDENTIFIER,
                                    opt82_vec));
                pkt4->addOption(clid);
                handle.setArgument("query4", pkt4);
                return 0;
            }
            OptionBuffer opt82_vec(it, remote_id.end());
            opt82_vec.push_back(0x2d);
            //sibset
            opt82_vec.push_back(circuit_id.back());
            LOG_INFO(option82_logger, "ClientID = %1").arg(encodeHex(opt82_vec));

            if (clid) {
                pkt4->delOption(DHO_DHCP_CLIENT_IDENTIFIER);
            }
            //std::vector<uint8_t> cid({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
            clid.reset(new Option(Option::V4,
                                    DHO_DHCP_CLIENT_IDENTIFIER,
                                    opt82_vec));
            pkt4->addOption(clid);
            handle.setArgument("query4", pkt4);
        }
    }
    return 0;
}
    
}
