/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Not a Contribution
 */
/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2_0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2_0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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


#define LOG_TAG "GnssAidl"
#define LOG_NDEBUG 0

#include "Gnss.h"
#include <log_util.h>
#include "loc_misc_utils.h"
#include "LocationUtil.h"
#include "GnssConfiguration.h"
#include "AGnssRil.h"
#include "AGnss.h"
#include "GnssGeofence.h"
#include "GnssDebug.h"
#include "GnssAntennaInfo.h"
#include "GnssVisibilityControl.h"
#include "GnssBatching.h"
#include "GnssPowerIndication.h"
#include "GnssMeasurementInterface.h"
#include "MeasurementCorrectionsInterface.h"
#include "battery_listener.h"

namespace android {
namespace hardware {
namespace gnss {
namespace aidl {
namespace implementation {
using measurement_corrections::aidl::implementation::MeasurementCorrectionsInterface;
using ::android::hardware::gnss::visibility_control::aidl::implementation::GnssVisibilityControl;

static Gnss* sGnss;
void gnssServiceDied(void* cookie) {
    LOC_LOGe("IGnssCallback AIDL service died");
    Gnss* iface = static_cast<Gnss*>(cookie);
    if (iface != nullptr) {
        iface->close();
        iface = nullptr;
    }
}
ScopedAStatus Gnss::setCallback(const shared_ptr<IGnssCallback>& callback) {
    if (callback == nullptr) {
        LOC_LOGe("Null callback ignored");
        return ScopedAStatus::fromExceptionCode(STATUS_INVALID_OPERATION);
    }

    mMutex.lock();
    if (mGnssCallback != nullptr) {
        AIBinder_unlinkToDeath(mGnssCallback->asBinder().get(), mDeathRecipient, this);
    }
    mGnssCallback = callback;
    GnssAPIClient* api = getApi();

    if (mGnssCallback != nullptr) {
        AIBinder_linkToDeath(callback->asBinder().get(), mDeathRecipient, this);
    }
    mMutex.unlock();

    if (api != nullptr) {
        api->gnssUpdateCallbacks(callback);
        api->gnssEnable(LOCATION_TECHNOLOGY_TYPE_GNSS);
        api->requestCapabilities();
    }

    return ScopedAStatus::ok();
}

ScopedAStatus Gnss::close() {
    if (mApi != nullptr) {
        mApi->gnssDisable();
    }
    return ScopedAStatus::ok();
}

void location_on_battery_status_changed(bool charging) {
    LOC_LOGd("battery status changed to %s charging", charging ? "" : "not");
    if ((sGnss != nullptr) && (sGnss->getLocationControlApi() != nullptr)) {
        sGnss->getLocationControlApi()->updateBatteryStatus(charging);
    }
}

Gnss::Gnss(): mGnssCallback(nullptr),
    mDeathRecipient(AIBinder_DeathRecipient_new(&gnssServiceDied)) {
    memset(&mPendingConfig, 0, sizeof(GnssConfig));
    ENTRY_LOG_CALLFLOW();
    sGnss = this;
    // register health client to listen on battery change
    loc_extn_battery_properties_listener_init(location_on_battery_status_changed);
}

Gnss::~Gnss() {
    ENTRY_LOG_CALLFLOW();
    if (mApi != nullptr) {
        mApi->destroy();
        mApi = nullptr;
    }
    sGnss = nullptr;
}

ILocationControlAPI* Gnss::getLocationControlApi() {
    if (mLocationControlApi == nullptr) {

        LocationControlCallbacks locCtrlCbs;
        memset(&locCtrlCbs, 0, sizeof(locCtrlCbs));
        locCtrlCbs.size = sizeof(LocationControlCallbacks);

        locCtrlCbs.odcpiReqCb =
                [this](const OdcpiRequestInfo& odcpiRequest) {
            odcpiRequestCb(odcpiRequest);
        };

        mLocationControlApi = LocationControlAPI::getInstance(locCtrlCbs);
    }

    return mLocationControlApi;
}


ScopedAStatus Gnss::updateConfiguration(GnssConfig& gnssConfig) {
    ENTRY_LOG_CALLFLOW();
    GnssAPIClient* api = getApi();
    if (api) {
        api->gnssConfigurationUpdate(gnssConfig);
    } else if (gnssConfig.flags != 0) {
        // api is not ready yet, update mPendingConfig with gnssConfig
        mPendingConfig.size = sizeof(GnssConfig);

        if (gnssConfig.flags & GNSS_CONFIG_FLAGS_GPS_LOCK_VALID_BIT) {
            mPendingConfig.flags |= GNSS_CONFIG_FLAGS_GPS_LOCK_VALID_BIT;
            mPendingConfig.gpsLock = gnssConfig.gpsLock;
        }
        if (gnssConfig.flags & GNSS_CONFIG_FLAGS_SUPL_VERSION_VALID_BIT) {
            mPendingConfig.flags |= GNSS_CONFIG_FLAGS_SUPL_VERSION_VALID_BIT;
            mPendingConfig.suplVersion = gnssConfig.suplVersion;
        }
        if (gnssConfig.flags & GNSS_CONFIG_FLAGS_SET_ASSISTANCE_DATA_VALID_BIT) {
            mPendingConfig.flags |= GNSS_CONFIG_FLAGS_SET_ASSISTANCE_DATA_VALID_BIT;
            mPendingConfig.assistanceServer.size = sizeof(GnssConfigSetAssistanceServer);
            mPendingConfig.assistanceServer.type = gnssConfig.assistanceServer.type;
            if (mPendingConfig.assistanceServer.hostName != nullptr) {
                free((void*)mPendingConfig.assistanceServer.hostName);
                mPendingConfig.assistanceServer.hostName =
                    strdup(gnssConfig.assistanceServer.hostName);
            }
            mPendingConfig.assistanceServer.port = gnssConfig.assistanceServer.port;
        }
        if (gnssConfig.flags & GNSS_CONFIG_FLAGS_LPP_PROFILE_VALID_BIT) {
            mPendingConfig.flags |= GNSS_CONFIG_FLAGS_LPP_PROFILE_VALID_BIT;
            mPendingConfig.lppProfileMask = gnssConfig.lppProfileMask;
        }
        if (gnssConfig.flags & GNSS_CONFIG_FLAGS_LPPE_CONTROL_PLANE_VALID_BIT) {
            mPendingConfig.flags |= GNSS_CONFIG_FLAGS_LPPE_CONTROL_PLANE_VALID_BIT;
            mPendingConfig.lppeControlPlaneMask = gnssConfig.lppeControlPlaneMask;
        }
        if (gnssConfig.flags & GNSS_CONFIG_FLAGS_LPPE_USER_PLANE_VALID_BIT) {
            mPendingConfig.flags |= GNSS_CONFIG_FLAGS_LPPE_USER_PLANE_VALID_BIT;
            mPendingConfig.lppeUserPlaneMask = gnssConfig.lppeUserPlaneMask;
        }
        if (gnssConfig.flags & GNSS_CONFIG_FLAGS_AGLONASS_POSITION_PROTOCOL_VALID_BIT) {
            mPendingConfig.flags |= GNSS_CONFIG_FLAGS_AGLONASS_POSITION_PROTOCOL_VALID_BIT;
            mPendingConfig.aGlonassPositionProtocolMask = gnssConfig.aGlonassPositionProtocolMask;
        }
        if (gnssConfig.flags & GNSS_CONFIG_FLAGS_EM_PDN_FOR_EM_SUPL_VALID_BIT) {
            mPendingConfig.flags |= GNSS_CONFIG_FLAGS_EM_PDN_FOR_EM_SUPL_VALID_BIT;
            mPendingConfig.emergencyPdnForEmergencySupl = gnssConfig.emergencyPdnForEmergencySupl;
        }
        if (gnssConfig.flags & GNSS_CONFIG_FLAGS_SUPL_EM_SERVICES_BIT) {
            mPendingConfig.flags |= GNSS_CONFIG_FLAGS_SUPL_EM_SERVICES_BIT;
            mPendingConfig.suplEmergencyServices = gnssConfig.suplEmergencyServices;
        }
        if (gnssConfig.flags & GNSS_CONFIG_FLAGS_SUPL_MODE_BIT) {
            mPendingConfig.flags |= GNSS_CONFIG_FLAGS_SUPL_MODE_BIT;
            mPendingConfig.suplModeMask = gnssConfig.suplModeMask;
        }
        if (gnssConfig.flags & GNSS_CONFIG_FLAGS_BLACKLISTED_SV_IDS_BIT) {
            mPendingConfig.flags |= GNSS_CONFIG_FLAGS_BLACKLISTED_SV_IDS_BIT;
            mPendingConfig.blacklistedSvIds = gnssConfig.blacklistedSvIds;
        }
        if (gnssConfig.flags & GNSS_CONFIG_FLAGS_EMERGENCY_EXTENSION_SECONDS_BIT) {
            mPendingConfig.flags |= GNSS_CONFIG_FLAGS_EMERGENCY_EXTENSION_SECONDS_BIT;
            mPendingConfig.emergencyExtensionSeconds = gnssConfig.emergencyExtensionSeconds;
        }
    }
    return ScopedAStatus::ok();
}

ScopedAStatus Gnss::getExtensionGnssBatching(shared_ptr<IGnssBatching>* _aidl_return) {
    ENTRY_LOG_CALLFLOW();
    if (mGnssBatching == nullptr) {
        mGnssBatching = SharedRefBase::make<GnssBatching>();
    }
    *_aidl_return = mGnssBatching;
    return ScopedAStatus::ok();
}
ScopedAStatus Gnss::getExtensionGnssGeofence(shared_ptr<IGnssGeofence>* _aidl_return) {
    ENTRY_LOG_CALLFLOW();
    if (mGnssGeofence == nullptr) {
        mGnssGeofence = SharedRefBase::make<GnssGeofence>();
    }
    *_aidl_return = mGnssGeofence;
    return ScopedAStatus::ok();
}
ScopedAStatus Gnss::getExtensionAGnss(shared_ptr<IAGnss>* _aidl_return) {
    ENTRY_LOG_CALLFLOW();
    if (mAGnss == nullptr) {
        mAGnss = SharedRefBase::make<AGnss>(this);
    }
    *_aidl_return = mAGnss;
    return ScopedAStatus::ok();
}
ScopedAStatus Gnss::getExtensionAGnssRil(shared_ptr<IAGnssRil>* _aidl_return) {
    ENTRY_LOG_CALLFLOW();
    if (mAGnssRil == nullptr) {
        mAGnssRil = SharedRefBase::make<AGnssRil>(this);
    }
    *_aidl_return = mAGnssRil;
    return ScopedAStatus::ok();
}
ScopedAStatus Gnss::getExtensionGnssDebug(shared_ptr<IGnssDebug>* _aidl_return) {
    ENTRY_LOG_CALLFLOW();
    if (mGnssDebug == nullptr) {
        mGnssDebug = SharedRefBase::make<GnssDebug>(this);
    }
    *_aidl_return = mGnssDebug;
    return ScopedAStatus::ok();
}
ScopedAStatus Gnss::getExtensionGnssVisibilityControl(
        shared_ptr<IGnssVisibilityControl>* _aidl_return) {
    ENTRY_LOG_CALLFLOW();
    if (mGnssVisibCtrl == nullptr) {
        mGnssVisibCtrl = SharedRefBase::make<GnssVisibilityControl>(this);
    }
    *_aidl_return = mGnssVisibCtrl;
    return ScopedAStatus::ok();
}
ScopedAStatus Gnss::start() {
    ENTRY_LOG_CALLFLOW();
    GnssAPIClient* api = getApi();
    if (api) {
        api->gnssStart();
    }
    return ScopedAStatus::ok();
}

ScopedAStatus Gnss::stop()  {
    ENTRY_LOG_CALLFLOW();
    GnssAPIClient* api = getApi();
    if (api) {
        api->gnssStop();
    }
    return ScopedAStatus::ok();
 }
ScopedAStatus Gnss::injectTime(int64_t timeMs, int64_t timeReferenceMs,
            int32_t uncertaintyMs) {
    return ScopedAStatus::ok();
}
ScopedAStatus Gnss::injectLocation(const GnssLocation& location) {
    ENTRY_LOG_CALLFLOW();
    ILocationControlAPI* pCtrlApi = getLocationControlApi();
    if (pCtrlApi != nullptr) {
        pCtrlApi->injectLocation(location.latitudeDegrees, location.longitudeDegrees,
                location.horizontalAccuracyMeters);
    }
    return ScopedAStatus::ok();
}
ScopedAStatus Gnss::injectBestLocation(const GnssLocation& gnssLocation) {
    ENTRY_LOG_CALLFLOW();
    ILocationControlAPI* pCtrlApi = getLocationControlApi();
    if (pCtrlApi != nullptr) {
        Location location = {};
        convertGnssLocation(gnssLocation, location);
        location.techMask |= LOCATION_TECHNOLOGY_HYBRID_BIT;
        pCtrlApi->odcpiInject(location);
    }
    return ScopedAStatus::ok();
}
ScopedAStatus Gnss::deleteAidingData(IGnss::GnssAidingData aidingDataFlags) {
    ENTRY_LOG_CALLFLOW();
    GnssAPIClient* api = getApi();
    if (api) {
        api->gnssDeleteAidingData(aidingDataFlags);
    }
    return ScopedAStatus::ok();
}
void Gnss::odcpiRequestCb(const OdcpiRequestInfo& request) {
    ENTRY_LOG_CALLFLOW();
    if (ODCPI_REQUEST_TYPE_STOP == request.type) {
        return;
    }
    mMutex.lock();
    auto gnssCb = mGnssCallback;
    mMutex.unlock();
    if (gnssCb != nullptr) {
        // For emergency mode, request DBH (Device based hybrid) location
        // Mark Independent from GNSS flag to false.
        if (ODCPI_REQUEST_TYPE_START == request.type) {
            LOC_LOGd("gnssRequestLocationCb isUserEmergency = %d", request.isEmergencyMode);
            auto r = gnssCb->gnssRequestLocationCb(!request.isEmergencyMode,
                                                                 request.isEmergencyMode);
            if (!r.isOk()) {
                LOC_LOGe("Error invoking gnssRequestLocationCb");
            }
        } else {
            LOC_LOGv("Unsupported ODCPI request type: %d", request.type);
        }
    } else {
        LOC_LOGe("ODCPI request not supported.");
    }
}
ScopedAStatus Gnss::setPositionMode(IGnss::GnssPositionMode mode,
            IGnss::GnssPositionRecurrence recurrence, int32_t minIntervalMs,
            int32_t preferredAccuracyMeters, int32_t preferredTimeMs,
            bool lowPowerMode) {
    ENTRY_LOG_CALLFLOW();
    GnssAPIClient* api = getApi();
    if (api) {
        api->gnssSetPositionMode(mode, recurrence, minIntervalMs, preferredAccuracyMeters,
                preferredTimeMs);
    }
    return ScopedAStatus::ok();
}
ScopedAStatus Gnss::getExtensionGnssAntennaInfo(shared_ptr<IGnssAntennaInfo>* _aidl_return) {
    ENTRY_LOG_CALLFLOW();
    if (mGnssAntennaInfo == nullptr) {
        mGnssAntennaInfo = SharedRefBase::make<GnssAntennaInfo>(this);
    }
    *_aidl_return = mGnssAntennaInfo;
    return ScopedAStatus::ok();
}
ScopedAStatus Gnss::getExtensionMeasurementCorrections(
        shared_ptr<IMeasurementCorrectionsInterface>* _aidl_return) {
    ENTRY_LOG_CALLFLOW();
    if (mGnssMeasCorr == nullptr) {
        mGnssMeasCorr = SharedRefBase::make<MeasurementCorrectionsInterface>(this);
    }
    *_aidl_return = mGnssMeasCorr;
    return ScopedAStatus::ok();
}
ScopedAStatus Gnss::getExtensionGnssConfiguration(
        shared_ptr<IGnssConfiguration>* _aidl_return) {
    ENTRY_LOG_CALLFLOW();
    if (mGnssConfiguration == nullptr) {
        mGnssConfiguration = SharedRefBase::make<GnssConfiguration>(this);
    }
    *_aidl_return = mGnssConfiguration;
    return ScopedAStatus::ok();
}

ScopedAStatus Gnss::getExtensionGnssPowerIndication(
        shared_ptr<IGnssPowerIndication>* _aidl_return) {
    ENTRY_LOG_CALLFLOW();
    if (mGnssPowerIndication == nullptr) {
        mGnssPowerIndication = SharedRefBase::make<GnssPowerIndication>();
    }
    *_aidl_return = mGnssPowerIndication;
    return ScopedAStatus::ok();
}
ScopedAStatus Gnss::getExtensionGnssMeasurement(
        shared_ptr<IGnssMeasurementInterface>* _aidl_return) {
    ENTRY_LOG_CALLFLOW();
    if (mGnssMeasurementInterface == nullptr) {
        mGnssMeasurementInterface = SharedRefBase::make<GnssMeasurementInterface>();
    }
    *_aidl_return = mGnssMeasurementInterface;
    return ScopedAStatus::ok();
}

GnssAPIClient* Gnss::getApi() {
    if (mApi != nullptr) {
        return mApi;
    }

    if (mGnssCallback != nullptr) {
        mApi = new GnssAPIClient(mGnssCallback);
    } else {
        LOC_LOGw("] GnssAPIClient is not ready");
        return mApi;
    }

    if (mPendingConfig.size == sizeof(GnssConfig)) {
        // we have pending GnssConfig
        mApi->gnssConfigurationUpdate(mPendingConfig);
        // clear size to invalid mPendingConfig
        mPendingConfig.size = 0;
        if (mPendingConfig.assistanceServer.hostName != nullptr) {
            free((void*)mPendingConfig.assistanceServer.hostName);
        }
    }

    return mApi;
}

}  // namespace implementation
}  // namespace aidl
}  // namespace gnss
}  // namespace hardware
}  // namespace android
