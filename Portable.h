#pragma once

#include <cstdint>
#include <cstddef>

namespace scope {

using SampleFs_t = uint32_t;
using SampleSn_t = uint16_t;
using SampleVo_t = uint16_t;

#pragma pack(push, 4)

enum class TriggerMode : uint8_t {
    ALWAYS = 0,
    NORMAL,
    SOFTWARE,
};

enum class TriggerSlope : uint8_t {
    UP = 0,
    DOWN,
};

using TriggerLevel = SampleVo_t;

struct SampleInfo {
    // device info
    SampleVo_t volMinmV;
    SampleVo_t volMaxmV;
    SampleFs_t fsMinSps;
    SampleFs_t fsMaxSps;
    SampleSn_t sampleSnMax;

    // sample info
    SampleSn_t sampleSn;
    SampleFs_t sampleFs;
    TriggerMode triggerMode;
    TriggerSlope triggerSlope;
    TriggerLevel triggerLevel;
};

/// Only used for transmission!
struct Message  {
    SampleInfo sampleInfo;
    SampleVo_t sampleData[0];

    constexpr static size_t CalcBytes(SampleSn_t sn) noexcept {
        return sizeof(SampleInfo) + sizeof(SampleVo_t) * sn;
    }

    Message() = delete;
};

struct Cmd  {
    enum Type : uint8_t {
        NONE = 0,
        SET_SAMPLE_FS,      // data: fs(Hz)
        SET_SAMPLE_SN,      // data: sn(point)
        SET_TRIGGER_MODE,   // data: mode(TriggerMode)
        SET_TRIGGER_SLOPE,  // data: slope(TriggerSlope)
        SET_TRIGGER_LEVEL,  // data: level(TriggerLevel)
        SOFTWARE_TRIGGER,   // no data
    };

    union Data {
        SampleFs_t sampleFs;
        SampleSn_t sampleSn;
        TriggerMode triggerMode;
        TriggerSlope triggerSlope;
        TriggerLevel triggerLevel;
    };

    Type type = NONE;
    Data data{};
};

#pragma pack(pop)

}
