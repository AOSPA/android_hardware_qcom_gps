/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Not a Contribution
 */
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

#define LOG_TAG "GnssMeasurementInterfaceAidl"

#include "GnssMeasurementInterface.h"
#include <android/binder_auto_utils.h>
#include <log_util.h>

namespace android {
namespace hardware {
namespace gnss {
namespace aidl {
namespace implementation {

void gnssMeasurementDied(void* cookie) {
    LOC_LOGe("IGnssMeasurementCallback AIDL service died");
    GnssMeasurementInterface* iface = static_cast<GnssMeasurementInterface*>(cookie);
    //clean up, i.e.  iface->close();
    if (iface != nullptr) {
        iface->close();
    }
}

::ndk::ScopedAStatus GnssMeasurementInterface::close() {
    mGnssMeasurementCb = nullptr;
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus GnssMeasurementInterface::setCallback(
        const std::shared_ptr<IGnssMeasurementCallback>& in_callback,
        bool in_enableFullTracking, bool in_enableCorrVecOutputs) {
    AIBinder_DeathRecipient* recipient = AIBinder_DeathRecipient_new(&gnssMeasurementDied);
    AIBinder_linkToDeath(in_callback->asBinder().get(), recipient, this);
    mGnssMeasurementCb = in_callback;
    return ndk::ScopedAStatus::ok();
}

}  // namespace implementation
}  // namespace aidl
}  // namespace gnss
}  // namespace hardware
}  // namespace android
