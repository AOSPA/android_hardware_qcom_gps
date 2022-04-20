/* Copyright (c) 2018-2021 The Linux Foundation. All rights reserved.
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
 *     * Neither the name of The Linux Foundation nor the names of its
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

#ifndef ILOCATIONAPI_H
#define ILOCATIONAPI_H

#include "LocationDataTypes.h"
#include <string>

class ILocationAPI
{
public:
    virtual ~ILocationAPI(){};
    virtual void destroy(locationApiDestroyCompleteCallback destroyCompleteCb=nullptr) = 0;

    /** @brief Updates/changes the callbacks that will be called.
        mandatory callbacks must be present for callbacks to be successfully updated
        no return value */
    virtual void updateCallbacks(LocationCallbacks&) = 0;

    /* ================================== TRACKING ================================== */

    /** @brief Starts a tracking session, which returns a session id that will be
       used by the other tracking APIs and also in the responseCallback to match command
       with response. locations are reported on the registered trackingCallback
       periodically according to LocationOptions.
       @return session id
        responseCallback returns:
                LOCATION_ERROR_SUCCESS if session was successfully started
                LOCATION_ERROR_ALREADY_STARTED if a startTracking session is already in progress
                LOCATION_ERROR_CALLBACK_MISSING if no trackingCallback was passed
                LOCATION_ERROR_INVALID_PARAMETER if LocationOptions parameter is invalid */
    virtual uint32_t startTracking(TrackingOptions&) = 0;

    /** @brief Stops a tracking session associated with id parameter.
        responseCallback returns:
                LOCATION_ERROR_SUCCESS if successful
                LOCATION_ERROR_ID_UNKNOWN if id is not associated with a tracking session */
    virtual void stopTracking(uint32_t id) = 0;

    /** @brief Changes the LocationOptions of a tracking session associated with id.
        responseCallback returns:
                LOCATION_ERROR_SUCCESS if successful
                LOCATION_ERROR_INVALID_PARAMETER if LocationOptions parameters are invalid
                LOCATION_ERROR_ID_UNKNOWN if id is not associated with a tracking session */
    virtual void updateTrackingOptions(uint32_t id, TrackingOptions&) = 0;

    /* ================================== BATCHING ================================== */

    /** @brief starts a batching session, which returns a session id that will be
       used by the other batching APIs and also in the responseCallback to match command
       with response. locations are reported on the batchingCallback passed in createInstance
       periodically according to LocationOptions. A batching session starts tracking on
       the low power processor and delivers them in batches by the batchingCallback when
       the batch is full or when getBatchedLocations is called. This allows for the processor
       that calls this API to sleep when the low power processor can batch locations in the
       backgroup and wake up the processor calling the API only when the batch is full, thus
       saving power.
       @return session id
        responseCallback returns:
                LOCATION_ERROR_SUCCESS if session was successful
                LOCATION_ERROR_ALREADY_STARTED if a startBatching session is already in progress
                LOCATION_ERROR_CALLBACK_MISSING if no batchingCallback
                LOCATION_ERROR_INVALID_PARAMETER if a parameter is invalid
                LOCATION_ERROR_NOT_SUPPORTED if batching is not supported */
    virtual uint32_t startBatching(BatchingOptions&) = 0;

    /** @brief Stops a batching session associated with id parameter.
        responseCallback returns:
                LOCATION_ERROR_SUCCESS if successful
                LOCATION_ERROR_ID_UNKNOWN if id is not associated with batching session */
    virtual void stopBatching(uint32_t id) = 0;

    /** @brief Changes the LocationOptions of a batching session associated with id.
        responseCallback returns:
                LOCATION_ERROR_SUCCESS if successful
                LOCATION_ERROR_INVALID_PARAMETER if LocationOptions parameters are invalid
                LOCATION_ERROR_ID_UNKNOWN if id is not associated with a batching session */
    virtual void updateBatchingOptions(uint32_t id, BatchingOptions&) = 0;

    /** @brief Gets a number of locations that are currently stored/batched
       on the low power processor, delivered by the batchingCallback passed in createInstance.
       Location are then deleted from the batch stored on the low power processor.
        responseCallback returns:
                LOCATION_ERROR_SUCCESS if successful, will be followed by batchingCallback call
                LOCATION_ERROR_CALLBACK_MISSING if no batchingCallback
                LOCATION_ERROR_ID_UNKNOWN if id is not associated with a batching session */
    virtual void getBatchedLocations(uint32_t id, size_t count) = 0;

    /* ================================== GEOFENCE ================================== */

    /** @brief Adds any number of geofences and returns an array of geofence ids that
       will be used by the other geofence APIs and also in the collectiveResponseCallback to
       match command with response. The geofenceBreachCallback will deliver the status of each
       geofence according to the GeofenceOption for each. The geofence id array returned will
       be valid until the collectiveResponseCallback is called and has returned.
       @return id array
        collectiveResponseCallback returns:
                LOCATION_ERROR_SUCCESS if session was successful
                LOCATION_ERROR_CALLBACK_MISSING if no geofenceBreachCallback
                LOCATION_ERROR_INVALID_PARAMETER if any parameters are invalid
                LOCATION_ERROR_NOT_SUPPORTED if geofence is not supported */
    virtual uint32_t* addGeofences(size_t count, GeofenceOption*, GeofenceInfo*) = 0;

    /** @brief Removes any number of geofences. Caller should delete ids array after
       removeGeofences returneds.
        collectiveResponseCallback returns:
                LOCATION_ERROR_SUCCESS if successful
                LOCATION_ERROR_ID_UNKNOWN if id is not associated with a geofence session */
    virtual void removeGeofences(size_t count, uint32_t* ids) = 0;

    /** @brief Modifies any number of geofences. Caller should delete ids array after
       modifyGeofences returns.
        collectiveResponseCallback returns:
                LOCATION_ERROR_SUCCESS if successful
                LOCATION_ERROR_ID_UNKNOWN if id is not associated with a geofence session
                LOCATION_ERROR_INVALID_PARAMETER if any parameters are invalid */
    virtual void modifyGeofences(size_t count, uint32_t* ids, GeofenceOption* options) = 0;

    /** @brief Pauses any number of geofences, which is similar to removeGeofences,
       only that they can be resumed at any time. Caller should delete ids array after
       pauseGeofences returns.
        collectiveResponseCallback returns:
                LOCATION_ERROR_SUCCESS if successful
                LOCATION_ERROR_ID_UNKNOWN if id is not associated with a geofence session */
    virtual void pauseGeofences(size_t count, uint32_t* ids) = 0;

    /** @brief Resumes any number of geofences that are currently paused. Caller should
       delete ids array after resumeGeofences returns.
        collectiveResponseCallback returns:
                LOCATION_ERROR_SUCCESS if successful
                LOCATION_ERROR_ID_UNKNOWN if id is not associated with a geofence session */
    virtual void resumeGeofences(size_t count, uint32_t* ids) = 0;

    /* ================================== GNSS ====================================== */

     /** @brief gnssNiResponse is called in response to a gnssNiCallback.
        responseCallback returns:
                LOCATION_ERROR_SUCCESS if session was successful
                LOCATION_ERROR_INVALID_PARAMETER if any parameters in GnssNiResponse are invalid
                LOCATION_ERROR_ID_UNKNOWN if id does not match a gnssNiCallback */
    virtual void gnssNiResponse(uint32_t id, GnssNiResponse response) = 0;

    /** @brief enableNetworkProvider enables Network Provider */
    virtual void enableNetworkProvider() {}

    /** @brief disableNetworkProvider disables Network Provider */
    virtual void disableNetworkProvider() {}

    /** @brief startNetworkLocation starts tracking session for
       network location request */
    virtual void startNetworkLocation(trackingCallback* callback) {}

    /** @brief stopNetworkLocation stops the ongoing tracking session for
       network location request */
    virtual void stopNetworkLocation(trackingCallback* callback) {}

    /** @brief Get energy consumed info of modem GNSS engine */
    virtual void getGnssEnergyConsumed(gnssEnergyConsumedCallback gnssEnergyConsumedCb,
            responseCallback responseCb) {}

    /** @brief Retrieve single-shot terrestrial position using the set of
        specified terrestrial technologies. */
    virtual void getSingleTerrestrialPosition(uint32_t timeoutMsec,
            TerrestrialTechMask techMask,
            float horQoS, trackingCallback terrestrialPositionCallback,
            responseCallback responseCb) {}

    /** @brief  Register/update listener to receive location system info
        that are not tied with positioning session, e.g.: next leap
        second event. */
    virtual void updateLocationSystemInfoListener(
            locationSystemInfoCallback locationSystemInfoCb,
            responseCallback responseCb) {}

    /** @brief
        Get Debug Report

        @param
        report: GnssDebugReport structure
    */
    virtual void getDebugReport(GnssDebugReport& report) {}
};

class ILocationControlAPI
{
public:
    virtual ~ILocationControlAPI(){};

    /* @brief Destroy/cleans up the instance, which should be called when
       ILocationControlAPI object is no longer needed.*/
    virtual void destroy() = 0;

    /** @brief
        API to update LocationControlCallbacks.

        @param
        callbacks: LocationControlCallbacks structure.

        @return
        Returns success or failure, i.e. zero or non-zero respectively.
    */
    virtual uint32_t updateCallbacks(LocationControlCallbacks&) {
        return false;
    };

    /* @brief Enable will enable specific location technology to be used for calculation
       locations and will effectively start a control session if call is successful,
       which returns a session id that will be returned in responseCallback to match
       command with response. The session id is also needed to call the disable command.
       This effect is global for all clients of LocationAPI
        responseCallback returns:
                LOCATION_ERROR_SUCCESS if successful
                LOCATION_ERROR_ALREADY_STARTED if an enable was already called for this techType
                LOCATION_ERROR_INVALID_PARAMETER if any parameters are invalid
                LOCATION_ERROR_GENERAL_FAILURE if failure for any other reason */
    virtual uint32_t enable(LocationTechnologyType techType) {
        return LOCATION_ERROR_GENERAL_FAILURE;
    }

    /* @brief Disable will disable specific location technology to be used for calculation
       locations and effectively ends the control session if call is successful.
       id parameter is the session id that was returned in enable responseCallback for techType.
       The session id is no longer valid after disable's responseCallback returns success.
       This effect is global for all clients of LocationAPI
        responseCallback returns:
                LOCATION_ERROR_SUCCESS if successful
                LOCATION_ERROR_ID_UNKNOWN if id was not returned from responseCallback from enable
                LOCATION_ERROR_GENERAL_FAILURE if failure for any other reason */
    virtual void disable(uint32_t id) {}

    /** @brief Updates the gnss specific configuration, which returns a session id array
       with an id for each of the bits set in GnssConfig.flags, order from low bits to high bits.
       The response for each config that is set will be returned in collectiveResponseCallback.
       The session id array returned will be valid until the collectiveResponseCallback is called
       and has returned. This effect is global for all clients of ILocationAPI.
        collectiveResponseCallback returns:
                LOCATION_ERROR_SUCCESS if session was successful
                LOCATION_ERROR_INVALID_PARAMETER if any other parameters are invalid
                LOCATION_ERROR_GENERAL_FAILURE if failure for any other reason */
    virtual uint32_t* gnssUpdateConfig(const GnssConfig& config) = 0;

    /* @brief gnssGetConfig fetches the current constellation and SV configuration
       on the GNSS engine.
       Returns a session id array with an id for each of the bits set in
       the mask parameter, order from low bits to high bits.
       Response is sent via the registered gnssConfigCallback.
       This effect is global for all clients of LocationAPI
       collectiveResponseCallback returns:
           LOCATION_ERROR_SUCCESS if session was successful
           LOCATION_ERROR_INVALID_PARAMETER if any parameter is invalid
           LOCATION_ERROR_CALLBACK_MISSING If no gnssConfigCallback
                                           was passed in createInstance
           LOCATION_ERROR_NOT_SUPPORTED If read of requested configuration
                                        is not supported

      PLEASE NOTE: It is caller's resposibility to FREE the memory of the return value.
                   The memory must be freed by delete [].*/
    virtual uint32_t* gnssGetConfig(GnssConfigFlagsMask mask) {
        return nullptr;
    }

    /** @brief Delete specific gnss aiding data for testing, which returns a session id
       that will be returned in responseCallback to match command with response.
       Only allowed in userdebug builds. This effect is global for all clients of ILocationAPI.
        responseCallback returns:
                LOCATION_ERROR_SUCCESS if successful
                LOCATION_ERROR_INVALID_PARAMETER if any parameters are invalid
                LOCATION_ERROR_NOT_SUPPORTED if build is not userdebug */
    virtual uint32_t gnssDeleteAidingData(GnssAidingData& data) = 0;

    /** @brief
        Configure the constellation and SVs to be used by the GNSS engine on
        modem.

        @param
        constellationEnablementConfig: configuration to enable/disable SV
        constellation to be used by SPE engine. When size in
        constellationEnablementConfig is set to 0, this indicates to reset SV
        constellation configuration to modem NV default.

        blacklistSvConfig: configuration to blacklist or unblacklist SVs
        used by SPE engine

        @return
        A session id that will be returned in responseCallback to
        match command with response. This effect is global for all
        clients of LocationAPI responseCallback returns:
                LOCATION_ERROR_SUCCESS if successful
                LOCATION_ERROR_INVALID_PARAMETER if any parameters are invalid
    */
    virtual uint32_t configConstellations(
            const GnssSvTypeConfig& constellationEnablementConfig,
            const GnssSvIdConfig&   blacklistSvConfig) = 0;

    /** @brief
        Configure the secondary band of constellations to be used by
        the GNSS engine on modem.

        @param
        secondaryBandConfig: configuration the secondary band usage
        for SPE engine

        @return
        A session id that will be returned in responseCallback to
        match command with response. This effect is global for all
        clients of LocationAPI responseCallback returns:
                LOCATION_ERROR_SUCCESS if successful
                LOCATION_ERROR_INVALID_PARAMETER if any parameters are invalid
    */
    virtual uint32_t configConstellationSecondaryBand(
            const GnssSvTypeConfig& secondaryBandConfig) = 0;

    /** @brief
        Enable or disable the constrained time uncertainty feature.

        @param
        enable: true to enable the constrained time uncertainty
        feature and false to disable the constrainted time
        uncertainty feature.

        @param
        tuncThreshold: this specifies the time uncertainty threshold
        that gps engine need to maintain, in units of milli-seconds.
        Default is 0.0 meaning that modem default value of time
        uncertainty threshold will be used. This parameter is
        ignored when requesting to disable this feature.

        @param
        energyBudget: this specifies the power budget that gps
        engine is allowed to spend to maintain the time uncertainty.
        Default is 0 meaning that GPS engine is not constained by
        power budget and can spend as much power as needed. The
        parameter need to be specified in units of 0.1 milli watt
        second. This parameter is ignored requesting to disable this
        feature.

        @return
        A session id that will be returned in responseCallback to
        match command with response. This effect is global for all
        clients of LocationAPI responseCallback returns:
                LOCATION_ERROR_SUCCESS if successful
                LOCATION_ERROR_INVALID_PARAMETER if any parameters
                are invalid
    */
    virtual uint32_t configConstrainedTimeUncertainty(
            bool enable, float tuncThreshold = 0.0,
            uint32_t energyBudget = 0) = 0;

    /** @brief
        Enable or disable position assisted clock estimator feature.

        @param
        enable: true to enable position assisted clock estimator and
        false to disable the position assisted clock estimator
        feature.

        @return
        A session id that will be returned in responseCallback to
        match command with response. This effect is global for all
        clients of LocationAPI responseCallback returns:
                LOCATION_ERROR_SUCCESS if successful
                LOCATION_ERROR_INVALID_PARAMETER if any parameters are invalid
    */
    virtual uint32_t configPositionAssistedClockEstimator(bool enable) = 0;

    /** @brief
        Sets the lever arm parameters for the vehicle.

        @param
        configInfo: lever arm configuration info regarding below two
        types of lever arm info:
        a: GNSS Antenna w.r.t the origin at the IMU e.g.: inertial
        measurement unit.
        b: lever arm parameters regarding the OPF (output frame)
        w.r.t the origin (at the GPS Antenna). Vehicle manufacturers
        prefer the position output to be tied to a specific point in
        the vehicle rather than where the antenna is placed
        (midpoint of the rear axle is typical).

        Caller can choose types of lever arm info to configure via the
        leverMarkTypeMask.

        @return
        A session id that will be returned in responseCallback to
        match command with response. This effect is global for all
        clients of LocationAPI responseCallback returns:
                LOCATION_ERROR_SUCCESS if successful
                LOCATION_ERROR_INVALID_PARAMETER if any parameters are invalid
    */
    virtual uint32_t configLeverArm(const LeverArmConfigInfo& configInfo) = 0;

    /** @brief
        Configure the robust location setting.

        @param
        enable: true to enable robust location and false to disable
        robust location.

        @param
        enableForE911: true to enable robust location when device is on
        E911 session and false to disable on E911 session.
        This parameter is only valid if robust location is enabled.

        @return
        A session id that will be returned in responseCallback to
        match command with response. This effect is global for all
        clients of LocationAPI responseCallback returns:
                LOCATION_ERROR_SUCCESS if successful
                LOCATION_ERROR_INVALID_PARAMETER if any parameters are invalid
    */
    virtual uint32_t configRobustLocation(bool enable, bool enableForE911) = 0;

    /** @brief
        Config the minimum GPS week used by modem GNSS engine.

        @param
        minGpsWeek: minimum GPS week to be used by modem GNSS engine.

        @return
        A session id that will be returned in responseCallback to
        match command with response. This effect is global for all
        clients of LocationAPI responseCallback returns:
                LOCATION_ERROR_SUCCESS if successful
                LOCATION_ERROR_INVALID_PARAMETER if any parameters are invalid
    */
    virtual uint32_t configMinGpsWeek(uint16_t minGpsWeek) = 0;

    /** @brief
        Configure the vehicle body-to-Sensor mount parameters and
        other parameters for dead reckoning position engine.

        @param
        dreConfig: vehicle body-to-Sensor mount angles and other
        parameters.

        @return
        A session id that will be returned in responseCallback to
        match command with response. This effect is global for all
        clients of LocationAPI responseCallback returns:
                LOCATION_ERROR_SUCCESS if successful
                LOCATION_ERROR_INVALID_PARAMETER if any parameters are invalid
    */
    virtual uint32_t configDeadReckoningEngineParams(
            const DeadReckoningEngineConfig& dreConfig)=0;

    /** @brief
        This API is used to instruct the specified engine to be in
        the pause/resume state. <br/>

        When the engine is placed in paused state, the engine will
        stop. If there is an on-going session, engine will no longer
        produce fixes. In the paused state, calling API to delete
        aiding data from the paused engine may not have effect.
        Request to delete Aiding data shall be issued after
        engine resume. <br/>

        Currently, only DRE engine will support pause/resume
        request. responseCb() will return not supported when request
        is made to pause/resume none-DRE engine. <br/>

        Request to pause/resume DRE engine can be made with or
        without an on-going session. With QDR engine, on resume,
        GNSS position & heading re-acquisition is needed for DR
        engine to engage. If DR engine is already in the requested
        state, the request will be no-op.  <br/>

        @param
        engType: the engine that is instructed to change its run
        state. <br/>

        engState: the new engine run state that the engine is
        instructed to be in. <br/>

        @return
        A session id that will be returned in responseCallback to
        match command with response. This effect is global for all
        clients of LocationAPI responseCallback returns:
                LOCATION_ERROR_SUCCESS if successful
                LOCATION_ERROR_INVALID_PARAMETER if any parameters are invalid
    */
    virtual uint32_t configEngineRunState(PositioningEngineMask engType,
                                          LocEngineRunState engState) = 0;

     /** @brief
        This API is used to config the NMEA sentence types.

        Without prior calling this API, all NMEA sentences supported
        in the system, as defined in NmeaTypesMask, will get
        generated and delivered to all the location clients that
        register to receive NMEA sentences.

        The NMEA sentence types are per-device setting and calling
        this API will impact all the location api clients that
        register to receive NMEA sentences. This API call is not
        incremental and the new NMEA sentence types will completely
        overwrite the previous call.

        If one or more unspecified bits are set in the NMEA mask,
        those bits will be ignored, but the rest of the
        configuration will get applied.

        Please note that the configured NMEA sentence types are not
        persistent.

        @param
        enabledNmeaTypes: specify the set of NMEA sentences the
        device will generate and deliver to the location api clients
        that register to receive NMEA sentences. <br/>

        @return
        A session id that will be returned in responseCallback to
        match command with response.
    */
    virtual uint32_t configOutputNmeaTypes(
            GnssNmeaTypesMask enabledNmeaTypes) = 0;

  /** @brief
        This API is used to send platform power events to GNSS adapters in order
        to handle GNSS sessions as per platform power event.

        @param
        powerState: Current vehicle/platform power state.

        @return
        No return value.
    */
    virtual void powerStateEvent(PowerStateType powerState) {};
    /** @brief
        This API is used to inject location into modem.

        @param
        location: location that contains PVT info. <br/>

        @return
        none
    */
    virtual void odcpiInject(const ::Location& location) {}

    /** @brief
        Resets all cached network info in HAL.
    */
    virtual void resetNetworkInfo() {};

    /** @brief
        Updates battery status in HAL as indicated by framework

        @param
        charging: Battery charging status
    */
    virtual void updateBatteryStatus(bool charging) {};

    /** @brief
        Inject location

        @param
        latitude : Location latitude in degree
        longitude : Location longitude in degree
        accuracy : Location accuracy in meters
    */
    virtual void injectLocation(double latitude, double longitude, float accuracy) {};

    /** @brief
        Request to open AGPS Data Connection

        @param
        apgpsType: Type of agps data connection to open
        apnName: Access Point Name
        apnLen: Length of apName string
        ipType: APN Bearer Type
    */
    virtual void agpsDataConnOpen(AGpsType agpsType, const char* apnName,
            int apnLen, int ipType) {}

    /** @brief
        Request to close AGPS data connection

        @param
        apgpsType: Type of agps data connection to close
    */
    virtual void agpsDataConnClosed(AGpsType agpsType) {}

    /** @brief
        Inform AGPS data connection failure

        @param
        apgpsType: Type of agps data connection that failed
    */
    virtual void agpsDataConnFailed(AGpsType agpsType) {}

    /** @brief
        Update connection status

        @param
        connected: Connected Status
        type: Type of connection, Eth, wifi, mobile etc
        roaming: Roaming status
        networkHandle: NetworkHandle type.
        apn: Access Point Name if needed

    */
    virtual void updateConnectionStatus(bool connected, int8_t type, bool roaming,
                                   NetworkHandle networkHandle, std::string& apn) {}

    /** @brief
        Set measurement correction

        @param
        gnssMeasCorr GnssMeasurementCorrections structure
    */
    virtual bool measCorrSetCorrections(const GnssMeasurementCorrections gnssMeasCorr) {
        return false;
    }

    /** @brief
        Close measurement corrections interface
    */
    virtual void measCorrClose() {}

    /** @brief
         Fetch antenna info from HAL
     */
    virtual void getGnssAntennaeInfo() {};

    /** @brief
        Close antenna info interface
    */
    virtual void antennaInfoClose() {}

    /** @brief
        Enables/disables permissions to non-framework application use of GNSS

        @param
        enable: true/false to enable / disable permission
    */
    virtual void enableNfwLocationAccess(std::vector<std::string>& enabledNfws) {}

    /** @brief
        Set the EULA opt-in status from system user. This is used as consent to
        use network-based positioning.

        @param
        userConsnt: user agrees to use GTP service or not.

        @return
        A session id that will be returned in responseCallback to
        match command with response.
    */
    virtual uint32_t setOptInStatus(bool userConsent) {
        return 0;
    }

    /** @brief
        This API is used to instruct the specified engine to use
        the provided integrity risk level for protection level
        calculation in position report. This API can be called via
        a position session is in progress.  <br/>

        Prior to calling this API for a particular engine, the
        engine shall not calcualte the protection levels and shall
        not include the protection levels in its position report.
        <br/>

        Currently, only PPE engine will support this function.
        LocConfigCb() will return LOC_INT_RESPONSE_NOT_SUPPORTED
        when request is made to none-PPE engines. <br/>

        @param
        engType: the engine that is instructed to use the specified
        integrity risk level for protection level calculation. The
        protection level will be returned back in
        LocationClientApi::GnssLocation. <br/>

        @param
        integrityRisk: the integrity risk level used for
        calculating protection level in
        LocationClientApi::GnssLocation. <br/>

        The integrity risk is defined as a probability per epoch,
        in unit of 2.5e-10. The valid range for actual integrity is
        [2.5e-10, 1-2.5e-10]), this corresponds to range of [1,
        4e9-1] of this parameter. <br/>

        If the specified value of integrityRisk is NOT in the valid
        range of [1, 4e9-1], the engine shall disable/invalidate
        the protection levels in the position report. <br/>

        @return true, if the API request has been accepted. The
        status will be returned via configCB. When returning
        true, LocConfigCb() will be invoked to deliver
        asynchronous processing status.
        <br/>

        @return false, if the API request has not been accepted for
        further processing. <br/>
    */
    virtual uint32_t configEngineIntegrityRisk(
            PositioningEngineMask engType, uint32_t integrityRisk) = 0;
};

#endif /* ILOCATIONAPI_H */
