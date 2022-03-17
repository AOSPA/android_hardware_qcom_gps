
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.gnss-aidl-impl-qti
# activate the following line for debug purposes only, comment out for production
#LOCAL_SANITIZE_DIAG += $(GNSS_SANITIZE_DIAG)

LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_VINTF_FRAGMENTS := android.hardware.gnss-aidl-service-qti.xml

LOCAL_SRC_FILES := \
    Gnss.cpp \
    GnssConfiguration.cpp \
    GnssPowerIndication.cpp \
    GnssMeasurementInterface.cpp \
    GnssBatching.cpp \
    GnssGeofence.cpp \
    AGnss.cpp \
    AGnssRil.cpp \
    GnssDebug.cpp \
    GnssAntennaInfo.cpp \
    MeasurementCorrectionsInterface.cpp \
    GnssVisibilityControl.cpp \
    location_api/GnssAPIClient.cpp \
    location_api/BatchingAPIClient.cpp \
    location_api/GeofenceAPIClient.cpp \
    location_api/LocationUtil.cpp

LOCAL_HEADER_LIBRARIES := \
    libgps.utils_headers \
    libloc_core_headers \
    libloc_pla_headers \
    liblocbatterylistener_headers \
    liblocation_api_headers

LOCAL_C_INCLUDES:= \
    $(LOCAL_PATH)/location_api

LOCAL_STATIC_LIBRARIES := liblocbatterylistener
LOCAL_STATIC_LIBRARIES += libhealthhalutils

LOCAL_SHARED_LIBRARIES := \
    libbase \
    libbinder_ndk \
    android.hardware.gnss-V2-ndk \
    android.hardware.health@1.0 \
    android.hardware.health@2.0 \
    android.hardware.health@2.1 \
    libhidlbase \
    liblog \
    libcutils \
    libqti_vndfwk_detect_vendor \
    libutils \
    libloc_core \
    libgps.utils \
    libdl \
    liblocation_api

LOCAL_CFLAGS += $(GNSS_CFLAGS)

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.gnss-aidl-service-qti
# activate the following line for debug purposes only, comment out for production
#LOCAL_SANITIZE_DIAG += $(GNSS_SANITIZE_DIAG)
LOCAL_VINTF_FRAGMENTS := android.hardware.gnss-aidl-service-qti.xml
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_INIT_RC := android.hardware.gnss-aidl-service-qti.rc
LOCAL_SRC_FILES := \
    service.cpp

LOCAL_HEADER_LIBRARIES := \
    libgps.utils_headers \
    libloc_core_headers \
    libloc_pla_headers \
    liblocation_api_headers

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libdl \
    libbase \
    libutils \
    libgps.utils \
    liblocation_api \
    libqti_vndfwk_detect_vendor \
    libbinder_ndk

LOCAL_SHARED_LIBRARIES += \
    libhidlbase \
    android.hardware.gnss@1.0 \
    android.hardware.gnss@1.1 \
    android.hardware.gnss@2.0 \
    android.hardware.gnss@2.1 \
    android.hardware.gnss-V2-ndk \
    android.hardware.gnss-aidl-impl-qti

LOCAL_CFLAGS += $(GNSS_CFLAGS)

include $(BUILD_EXECUTABLE)
