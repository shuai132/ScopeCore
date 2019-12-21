#include <cassert>

#include "ScopeGUI.h"

namespace scope {

ScopeGUI::ScopeGUI(Comm* comm_) noexcept
    : comm_(comm_), packetProcessor_(false), cmdInterval_(100) {
    packetProcessor_.setOnPacketHandle([this](uint8_t* data, size_t size) {
        onMessage((Message*)data, size);
    });
}

void ScopeGUI::onMcuData(const uint8_t* data, size_t size) {
    packetProcessor_.feed(data, size);
}

void ScopeGUI::sendCmd(Cmd cmd) {
    packetProcessor_.packForeach((uint8_t*)&cmd, sizeof(cmd), [this](uint8_t* data, size_t size) {
        comm_->sendToMcu(data, size);
    });
}

void ScopeGUI::sendCmd(Cmd::Type type, Cmd::Data data) {
    auto now = std::chrono::steady_clock::now();
    if (now - lastCmdTime_ < cmdInterval_) return;
    lastCmdTime_ = now;

    Cmd cmd;
    cmd.type = type;
    cmd.data = data;
    sendCmd(cmd);
}

void ScopeGUI::onMessage(Message* message, size_t size) {
    comm_->onMessage(message, size);
}

}
