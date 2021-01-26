/*
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <aidl/android/hardware/gnss/IGnss.h>
#include <android/hardware/gnss/2.1/IGnss.h>
#include <hidl/LegacySupport.h>
#include "loc_cfg.h"
#include "loc_misc_utils.h"
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include "Gnss.h"
#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <pthread.h>
#include <log_util.h>

extern "C" {
#include "vndfwk-detect.h"
}
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "android.hardware.gnss-aidl-impl-qti"

#ifdef ARCH_ARM_32
#define DEFAULT_HW_BINDER_MEM_SIZE 65536
#endif

using android::hardware::configureRpcThreadpool;
using android::hardware::registerPassthroughServiceImplementation;
using android::hardware::joinRpcThreadpool;
using ::android::sp;

using android::status_t;
using android::OK;

typedef int vendorEnhancedServiceMain(int /* argc */, char* /* argv */ []);

using GnssAidl = ::android::hardware::gnss::aidl::implementation::Gnss;
using android::hardware::configureRpcThreadpool;
using ::android::hardware::gnss::V1_0::GnssLocation;
using android::hardware::gnss::V2_1::IGnss;

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(1);
    ABinderProcess_startThreadPool();
    ALOGI("%s, start Gnss HAL process", __FUNCTION__);

    std::shared_ptr<GnssAidl> gnssAidl = ndk::SharedRefBase::make<GnssAidl>();
    const std::string instance = std::string() + GnssAidl::descriptor + "/default";
    binder_status_t status =
            AServiceManager_addService(gnssAidl->asBinder().get(), instance.c_str());
    if (STATUS_OK == status) {
        ALOGD("register IGnss AIDL service success");
    } else {
        ALOGD("Error while register IGnss AIDL service, status: %d", status);
    }

    int vendorInfo = getVendorEnhancedInfo();
    bool vendorEnhanced = ( 1 == vendorInfo || 3 == vendorInfo );
    setVendorEnhanced(vendorEnhanced);

#ifdef ARCH_ARM_32
    android::hardware::ProcessState::initWithMmapSize((size_t)(DEFAULT_HW_BINDER_MEM_SIZE));
#endif
    configureRpcThreadpool(1, true);

    status_t ret;
    ret = registerPassthroughServiceImplementation<IGnss>();
    if (ret == OK) {
        // Loc AIDL service
#define VENDOR_AIDL_LIB "vendor.qti.gnss-service.so"

        void* libAidlHandle = NULL;
        vendorEnhancedServiceMain* aidlMainMethod = (vendorEnhancedServiceMain*)
            dlGetSymFromLib(libAidlHandle, VENDOR_AIDL_LIB, "main");
        if (NULL != aidlMainMethod) {
            ALOGI("start LocAidl service");
            (*aidlMainMethod)(0, NULL);
        }
        // Loc AIDL service end
        joinRpcThreadpool();
        ABinderProcess_joinThreadPool();
    } else {
        ALOGE("Error while registering IGnss HIDL 2.1 service: %d", ret);
    }

    return EXIT_FAILURE;  // should not reach
}
