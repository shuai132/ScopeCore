#pragma once

#include <cstdint>
#include <chrono>

#include "noncopyable.h"
#include "Portable.h"
#include "PacketProcessor.h"

namespace scope {

class ScopeGUI : noncopyable {
public:
    /// 上位机消息接口
    struct Comm {
        virtual ~Comm() = default;
        virtual void sendToMcu(const uint8_t* data, size_t size) = 0;
        virtual void onMessage(Message* message, size_t size) = 0;
    };

public:
    explicit ScopeGUI(Comm* comm, uint16_t cmdIntervalMs = 0) noexcept;

public:
    /**
     * 从MCU接收到数据 数据可以是间歇的 内部自动进行打包
     * @param data
     * @param size
     */
    void onMcuData(const uint8_t* data, size_t size);

    void sendCmd(Cmd cmd);

    void sendCmd(Cmd::Type type, Cmd::Data data = {});

private:
    void onMessage(Message* message, size_t size);

private:
    Comm* comm_;

    PacketProcessor packetProcessor_;

    const std::chrono::milliseconds cmdInterval_;
    std::chrono::steady_clock::time_point lastCmdTime_ = std::chrono::steady_clock::now();
};

}
