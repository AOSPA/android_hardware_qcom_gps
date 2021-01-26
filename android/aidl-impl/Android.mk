
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.gnss-aidl-impl-qti
LOCAL_SANITIZE += $(GNSS_SANITIZE)
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
    location_api/GnssAPIClient.cpp

LOCAL_HEADER_LIBRARIES := \
    libgps.utils_headers \
    libloc_core_headers \
    libloc_pla_headers \
    liblocation_api_headers \
    liblocbatterylistener_headers

LOCAL_SHARED_LIBRARIES := \
    libbase \
    libhidlbase \
    libbinder_ndk \
    android.hardware.gnss-ndk_platform \
    liblog \
    libcutils \
    libqti_vndfwk_detect \
    libutils \
    libloc_core \
    libgps.utils \
    libdl \
    liblocation_api \

LOCAL_CFLAGS += $(GNSS_CFLAGS)

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.gnss-aidl-service-qti
LOCAL_SANITIZE += $(GNSS_SANITIZE)
# activate the following line for debug purposes only, comment out for production
#LOCAL_SANITIZE_DIAG += $(GNSS_SANITIZE_DIAG)
LOCAL_VINTF_FRAGMENTS := android.hardware.gnss-aidl-service-qti.xml
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_INIT_RC := android.hardware.gnss-aidl-service-qti.rc
LOCAL_SRC_FILES := \
    service.cpp \
    Gnss.cpp \
    GnssConfiguration.cpp \
    GnssPowerIndication.cpp \
    GnssMeasurementInterface.cpp \
    location_api/GnssAPIClient.cpp

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
    libqti_vndfwk_detect \
    libbinder_ndk \

LOCAL_SHARED_LIBRARIES += \
    libhidlbase \
    android.hardware.gnss@1.0 \
    android.hardware.gnss@1.1 \
    android.hardware.gnss@2.0 \
    android.hardware.gnss@2.1 \
    android.hardware.gnss-ndk_platform \

LOCAL_CFLAGS += $(GNSS_CFLAGS)

ifneq ($(LOC_HIDL_VERSION),)
LOCAL_CFLAGS += -DLOC_HIDL_VERSION='"$(LOC_HIDL_VERSION)"'
endif

include $(BUILD_EXECUTABLE)
