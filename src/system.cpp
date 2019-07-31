#include <memory>

#include <h/amigastubs.h>
#include "system.h"

// OS/portability functions

void PutMsg(MsgPort *port, Message *msg)
{
    ///TODO: Fake
    (void)port;
    (void)msg;
}

Message *GetMsg(MsgPort *port)
{
    ///TODO: Fake
    (void)port;
    return nullptr;
}

#include <map>
#include <string>

using PortTable = std::map<std::string, std::unique_ptr<MsgPort>>;
static PortTable s_portTable;

MsgPort *FindPort(const char *portName)
{
    auto it = s_portTable.find(portName);
    return (it != s_portTable.end()) ? it->second.get() : nullptr;
}

MsgPort *CreatePort(const char *portName, uint32_t priority)
{
    if (auto port = FindPort(portName); port)
        return port;
    auto port = std::make_unique<MsgPort>();
    port->mp_Node.ln_Pri = priority;
    port->mp_Node.ln_Name = portName;
    auto portp = port.get();

    s_portTable.emplace(portName, std::move(port));

    return portp;
}
