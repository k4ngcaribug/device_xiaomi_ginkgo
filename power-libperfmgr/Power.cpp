/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "android.hardware.power-service.xiaomi-libperfmgr"

#include "Power.h"

#include <mutex>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/stringprintf.h>
#include <android-base/strings.h>

#include <utils/Log.h>

namespace aidl {
namespace google {
namespace hardware {
namespace power {
namespace impl {
namespace pixel {

constexpr char kPowerHalStateProp[] = "vendor.powerhal.state";
constexpr char kPowerHalAudioProp[] = "vendor.powerhal.audio";
constexpr char kPowerHalRenderingProp[] = "vendor.powerhal.rendering";

Power::Power(std::shared_ptr<HintManager> hm)
    : mHintManager(hm),
      mInteractionHandler(nullptr),
      mSustainedPerfModeOn(false),
      mBatterySaverOn(false) {
    mInteractionHandler = std::make_unique<InteractionHandler>(mHintManager);
    mInteractionHandler->Init();

    std::string state = ::android::base::GetProperty(kPowerHalStateProp, "");
    if (state == "SUSTAINED_PERFORMANCE") {
        ALOGI("Initialize with SUSTAINED_PERFORMANCE on");
        mHintManager->DoHint("SUSTAINED_PERFORMANCE");
        mSustainedPerfModeOn = true;
    } else {
        ALOGI("Initialize PowerHAL");
    }

    state = ::android::base::GetProperty(kPowerHalAudioProp, "");
    if (state == "AUDIO_STREAMING_LOW_LATENCY") {
        ALOGI("Initialize with AUDIO_LOW_LATENCY on");
        mHintManager->DoHint(state);
    }

    state = ::android::base::GetProperty(kPowerHalRenderingProp, "");
    if (state == "EXPENSIVE_RENDERING") {
        ALOGI("Initialize with EXPENSIVE_RENDERING on");
        mHintManager->DoHint("EXPENSIVE_RENDERING");
    }

    // Now start to take powerhint
    ALOGI("PowerHAL ready to process hints");
}

void endAllHints(std::shared_ptr<HintManager> mHintManager) {
    for (std::string hint: mHintManager->GetHints()) {
        mHintManager->EndHint(hint);
    }
}

ndk::ScopedAStatus Power::setMode(Mode type, bool enabled) {
    LOG(DEBUG) << "Power setMode: " << toString(type) << " to: " << enabled;
    switch (type) {
#ifdef TAP_TO_WAKE_NODE
        case Mode::DOUBLE_TAP_TO_WAKE:
            ::android::base::WriteStringToFile(enabled ? "1" : "0", TAP_TO_WAKE_NODE, true);
            break;
#endif
        case Mode::SUSTAINED_PERFORMANCE:
            if (enabled) {
                endAllHints(mHintManager);
                mHintManager->DoHint("SUSTAINED_PERFORMANCE");
            } else {
                mHintManager->EndHint("SUSTAINED_PERFORMANCE");
            }
            mSustainedPerfModeOn = enabled;
            break;
        case Mode::LOW_POWER:
            if (enabled) {
                endAllHints(mHintManager);
                mHintManager->DoHint("LOW_POWER");
            } else {
                mHintManager->EndHint("LOW_POWER");
            }
            mBatterySaverOn = enabled;
            break;
        case Mode::LAUNCH:
            [[fallthrough]];
#ifndef TAP_TO_WAKE_NODE
        case Mode::DOUBLE_TAP_TO_WAKE:
            [[fallthrough]];
#endif
        case Mode::FIXED_PERFORMANCE:
            [[fallthrough]];
        case Mode::EXPENSIVE_RENDERING:
            [[fallthrough]];
        case Mode::INTERACTIVE:
            [[fallthrough]];
        case Mode::DEVICE_IDLE:
            [[fallthrough]];
        case Mode::DISPLAY_INACTIVE:
            [[fallthrough]];
        case Mode::AUDIO_STREAMING_LOW_LATENCY:
            [[fallthrough]];
        default:
            if (mBatterySaverOn || mSustainedPerfModeOn) {
                break;
            }
            if (enabled) {
                mHintManager->DoHint(toString(type));
            } else {
                mHintManager->EndHint(toString(type));
            }
            break;
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Power::isModeSupported(Mode type, bool *_aidl_return) {
    bool supported = mHintManager->IsHintSupported(toString(type));
#ifdef TAP_TO_WAKE_NODE
    if (type == Mode::DOUBLE_TAP_TO_WAKE) {
        supported = true;
    }
#endif
    LOG(INFO) << "Power mode " << toString(type) << " isModeSupported: " << supported;
    *_aidl_return = supported;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Power::setBoost(Boost type, int32_t durationMs) {
    LOG(DEBUG) << "Power setBoost: " << toString(type) << " duration: " << durationMs;
    switch (type) {
        case Boost::INTERACTION:
            if (mSustainedPerfModeOn || mBatterySaverOn) {
                break;
            }
            mInteractionHandler->Acquire(durationMs);
            break;
        case Boost::DISPLAY_UPDATE_IMMINENT:
            [[fallthrough]];
        case Boost::ML_ACC:
            [[fallthrough]];
        case Boost::AUDIO_LAUNCH:
            [[fallthrough]];
        default:
            if (mSustainedPerfModeOn || mBatterySaverOn) {
                break;
            }
            if (durationMs > 0) {
                mHintManager->DoHint(toString(type), std::chrono::milliseconds(durationMs));
            } else if (durationMs == 0) {
                mHintManager->DoHint(toString(type));
            } else {
                mHintManager->EndHint(toString(type));
            }
            break;
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Power::isBoostSupported(Boost type, bool *_aidl_return) {
    bool supported = mHintManager->IsHintSupported(toString(type));
    LOG(INFO) << "Power boost " << toString(type) << " isBoostSupported: " << supported;
    *_aidl_return = supported;
    return ndk::ScopedAStatus::ok();
}

constexpr const char *boolToString(bool b) {
    return b ? "true" : "false";
}

binder_status_t Power::dump(int fd, const char **, uint32_t) {
    std::string buf(::android::base::StringPrintf(
            "HintManager Running: %s\n"
            "SustainedPerformanceMode: %s\n"
            "BatterySaverMode: %s\n",
            boolToString(mHintManager->IsRunning()),
            boolToString(mSustainedPerfModeOn),
            boolToString(mBatterySaverOn)));
    // Dump nodes through libperfmgr
    mHintManager->DumpToFd(fd);
    if (!::android::base::WriteStringToFd(buf, fd)) {
        PLOG(ERROR) << "Failed to dump state to fd";
    }
    fsync(fd);
    return STATUS_OK;
}

}  // namespace pixel
}  // namespace impl
}  // namespace power
}  // namespace hardware
}  // namespace google
}  // namespace aidl
