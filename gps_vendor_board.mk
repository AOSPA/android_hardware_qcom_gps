# Any TARGET_BOARD_PLATFORM being built that does
# not want vendor location modules built should be
# added to this exclude list to prevent building
LOC_BOARD_PLATFORM_EXCLUDE_LIST :=

# Define BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE if:
# (TARRGET_USES_QMMA is not true OR
#  TARGET_USES_QMAA_OVERRIDE_GPS is not false) AND
#  TARGET_BOARD_PLATFORM is not in LOC_BOARD_PLATFORM_EXCLUDE_LIST
ifneq ($(TARGET_USES_QMAA),true)
  ifeq (,$(filter $(LOC_BOARD_PLATFORM_EXCLUDE_LIST),$(TARGET_BOARD_PLATFORM)))
    BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE := default
  endif
else ifneq ($(TARGET_USES_QMAA_OVERRIDE_GPS),false)
  ifeq (,$(filter $(LOC_BOARD_PLATFORM_EXCLUDE_LIST),$(TARGET_BOARD_PLATFORM)))
    BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE := default
  endif
endif

