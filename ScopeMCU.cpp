#include "ScopeMCU.h"

namespace scope {

ScopeMCU::ScopeMCU(size_t maxSn, uint8_t* buffer) noexcept
    : processor_(nullptr, false)
    {
    maxSampleNum_ = maxSn;
    if (buffer != nullptr) {
        message_.reset(reinterpret_cast<Message*>(buffer));
    } else {
        message_.reset(reinterpret_cast<Message*>(new uint8_t[Message::CalcBytes(maxSn)]));
    }
    processor_.setMaxBufferSize(sizeof(Cmd));
    processor_.setOnPacketHandle([this](uint8_t* payload, size_t size) {
        stopSample();
        Cmd* cmd = (Cmd*)payload;
        Cmd::Data data = cmd->data;
        switch (cmd->type) {
            case Cmd::NONE:
                break;
            case Cmd::SET_SAMPLE_FS:
                updateFs(data.sampleFs);
                break;
            case Cmd::SET_SAMPLE_SN:
                updateSampleNum(data.sampleSn > maxSampleNum_ ? maxSampleNum_ : data.sampleSn);
                break;
            case Cmd::SET_TRIGGER_MODE:
                updateTriggerMode(data.triggerMode);
                break;
            case Cmd::SET_TRIGGER_SLOPE:
                updateTriggerSlope(data.triggerSlope);
                break;
            case Cmd::SET_TRIGGER_LEVEL:
                updateTriggerLevel(data.triggerLevel);
                break;
            case Cmd::SOFTWARE_TRIGGER:
                startSample();
                break;
        }
    });
}

void ScopeMCU::setMcuImpl(MCU mcu) {
    mcu_ = std::move(mcu);
    updateFs(50000);
    updateSampleNum(512);
    updateTriggerMode(TriggerMode::NORMAL);
    updateTriggerSlope(TriggerSlope::UP);
    updateTriggerLevel(1000);
    mcu_.startADC();
}

void ScopeMCU::onADC(SampleVo_t vol) {
    if (sampling_) {
        addADC(vol);
        return;
    }

    // trigger logic
    if (sampleInfo_.triggerMode == TriggerMode::NORMAL) {
        switch (sampleInfo_.triggerSlope) {
            case TriggerSlope::UP:
                if (lastVol_ < sampleInfo_.triggerLevel && sampleInfo_.triggerLevel < vol) {
                    startSample();
                    addADC(lastVol_);
                    addADC(vol);
                }
                break;
            case TriggerSlope::DOWN:
                if (lastVol_ > sampleInfo_.triggerLevel && sampleInfo_.triggerLevel > vol) {
                    startSample();
                    addADC(lastVol_);
                    addADC(vol);
                }
                break;
        }
    }
    else if (sampleInfo_.triggerMode == TriggerMode::ALWAYS) {
        startSample();
    }
    lastVol_ = vol;
}

void ScopeMCU::setVolLimits(SampleVo_t volMin, SampleVo_t volMax) {
    sampleInfo_.volMinmV = volMin;
    sampleInfo_.volMaxmV = volMax;
}

void ScopeMCU::setFsLimits(SampleFs_t fsMinSps, SampleFs_t fsMaxSps) {
    sampleInfo_.fsMinSps = fsMinSps;
    sampleInfo_.fsMaxSps = fsMaxSps;
}

void ScopeMCU::onRead(uint8_t* data, size_t size) {
    processor_.feed(data, size);
}

bool ScopeMCU::isSampling() {
    return sampling_;
}

void ScopeMCU::addADC(SampleVo_t vol) {
    message_->sampleData[samplePos_] = vol;
    if (++samplePos_ >= sampleInfo_.sampleSn) {
        onSampleFinish();
    }
};

void ScopeMCU::startSample() {
    sampling_ = true;
    samplePos_ = 0;
    mcu_.onSampling(true);
}

void ScopeMCU::stopSample() {
    sampling_ = false;
    samplePos_ = 0;
    mcu_.onSampling(false);
}

void ScopeMCU::onSampleFinish() {
    stopSample();
    message_->sampleInfo = sampleInfo_;
    processor_.packForeach(message_.get(), Message::CalcBytes(sampleInfo_.sampleSn), [this](uint8_t* data, size_t size) {
        mcu_.sendData(data, size);
    });

    if (sampleInfo_.triggerMode == TriggerMode::ALWAYS) {
        startSample();
    }
}

void ScopeMCU::updateFs(SampleFs_t fs) {
    if (fs > sampleInfo_.fsMaxSps) {
        fs = sampleInfo_.fsMaxSps;
    }
    auto realFs = mcu_.setSampleFs(fs);
    sampleInfo_.sampleFs = realFs;
}

void ScopeMCU::updateSampleNum(SampleSn_t sn) {
    if (sn > maxSampleNum_) {
        sn = maxSampleNum_;
    } else if (sn < 2) {
        sn = 2;
    }
    sampleInfo_.sampleSn = sn;
}

void ScopeMCU::updateTriggerMode(TriggerMode mode) {
    sampleInfo_.triggerMode = mode;
    if (mode == TriggerMode::SOFTWARE) {
        startSample();
    }
}

void ScopeMCU::updateTriggerSlope(TriggerSlope slope) {
    sampleInfo_.triggerSlope = slope;
}

void ScopeMCU::updateTriggerLevel(TriggerLevel vol) {
    sampleInfo_.triggerLevel = vol;
}

}
