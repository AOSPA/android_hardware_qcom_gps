/* Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation, nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/*
Changes from Qualcomm Innovation Center are provided under the following license:

Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted (subject to the limitations in the
disclaimer below) provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.

    * Neither the name of Qualcomm Innovation Center, Inc. nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GNSS_API_CLINET_H
#define GNSS_API_CLINET_H


#include <mutex>
#include <android/hardware/gnss/2.1/IGnss.h>
#include <android/hardware/gnss/2.1/IGnssCallback.h>
#include <LocationAPIClientBase.h>

namespace android {
namespace hardware {
namespace gnss {
namespace V2_1 {
namespace implementation {

using ::android::sp;

class GnssAPIClient : public LocationAPIClientBase
{
public:
    GnssAPIClient(const sp<V1_0::IGnssCallback>& gpsCb,
            const sp<V1_0::IGnssNiCallback>& niCb);
    GnssAPIClient(const sp<V2_0::IGnssCallback>& gpsCb);
    GnssAPIClient(const sp<V2_1::IGnssCallback>& gpsCb);
    GnssAPIClient(const GnssAPIClient&) = delete;
    GnssAPIClient& operator=(const GnssAPIClient&) = delete;

    // for GpsInterface
    void gnssUpdateCallbacks(const sp<V1_0::IGnssCallback>& gpsCb,
            const sp<V1_0::IGnssNiCallback>& niCb);
    void gnssUpdateCallbacks_2_0(const sp<V2_0::IGnssCallback>& gpsCb);
    void gnssUpdateCallbacks_2_1(const sp<V2_1::IGnssCallback>& gpsCb);
    bool gnssStart();
    bool gnssStop();
    bool gnssSetPositionMode(V1_0::IGnss::GnssPositionMode mode,
            V1_0::IGnss::GnssPositionRecurrence recurrence,
            uint32_t minIntervalMs,
            uint32_t preferredAccuracyMeters,
            uint32_t preferredTimeMs,
            GnssPowerMode powerMode = GNSS_POWER_MODE_INVALID,
            uint32_t timeBetweenMeasurement = 0);

    // for GpsNiInterface
    void gnssNiRespond(int32_t notifId, V1_0::IGnssNiCallback::GnssUserResponseType userResponse);

    // these apis using LocationAPIControlClient
    void gnssDeleteAidingData(V1_0::IGnss::GnssAidingData aidingDataFlags);
    void gnssEnable(LocationTechnologyType techType);
    void gnssDisable();
    void gnssConfigurationUpdate(const GnssConfig& gnssConfig);

    inline LocationCapabilitiesMask gnssGetCapabilities() const {
        return mLocationCapabilitiesMask;
    }
    void requestCapabilities();

    // callbacks we are interested in
    void onCapabilitiesCb(LocationCapabilitiesMask capabilitiesMask) final;
    void onTrackingCb(const Location& location) final;
    void onGnssNiCb(uint32_t id, const GnssNiNotification& gnssNiNotification) final;
    void onGnssSvCb(const GnssSvNotification& gnssSvNotification) final;
    void onGnssNmeaCb(GnssNmeaNotification gnssNmeaNotification) final;
    void onEngineLocationsInfoCb(uint32_t count,
            GnssLocationInfoNotification* engineLocationInfoNotification);


    void onStartTrackingCb(LocationError error) final;
    void onStopTrackingCb(LocationError error) final;

private:
    virtual ~GnssAPIClient();
    void setCallbacks();
    void initLocationOptions();

    sp<V1_0::IGnssCallback> mGnssCbIface;
    sp<V1_0::IGnssNiCallback> mGnssNiCbIface;
    std::mutex mMutex;
    LocationAPIControlClient* mControlClient;
    LocationCapabilitiesMask mLocationCapabilitiesMask;
    bool mLocationCapabilitiesCached;
    TrackingOptions mTrackingOptions;
    bool mTracking;
    sp<V2_0::IGnssCallback> mGnssCbIface_2_0;
    sp<V2_1::IGnssCallback> mGnssCbIface_2_1;
};

}  // namespace implementation
}  // namespace V2_1
}  // namespace gnss
}  // namespace hardware
}  // namespace android
#endif // GNSS_API_CLINET_H
