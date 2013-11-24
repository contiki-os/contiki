/*
 * File: stack/phy/phy-library.h
 * Description: Interface definition for library functionality.
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */


//---------------------------------------------------------------------------
// Definitions
//------------

/**
 * @name SIMPLEMAC version defines
 *@{
 */


/**
 * @brief Version major number
 */
#define SIMPLEMAC_VERSION_MAJOR 1

/**
 * @brief Version minor number
 */
#define SIMPLEMAC_VERSION_MINOR 0

/**
 * @brief Version patch number
 */
#define SIMPLEMAC_VERSION_PATCH 2

/**
 * @brief Version qualifier
 */
#define SIMPLEMAC_VERSION_QUAL  ""

#define _SQUOTEME(a) #a
#define SQUOTEME(a) _SQUOTEME(a)

/**
 * @brief Version string
 */
#define SIMPLEMAC_VERSION_STRING SQUOTEME(SIMPLEMAC_VERSION_MAJOR) "." SQUOTEME(SIMPLEMAC_VERSION_MINOR) "." SQUOTEME(SIMPLEMAC_VERSION_PATCH) SIMPLEMAC_VERSION_QUAL

//@} \\END SIMPLEMAC version defines


#define SECURITY_BLOCK_SIZE  16 // bytes
#define MIN_RADIO_POWER     -43 // dBm
#define MAX_RADIO_POWER       3 // dBm

#ifndef  PHY_H_
enum {
  ST_RADIO_POWER_MODE_RX_ON,
  ST_RADIO_POWER_MODE_OFF
};
typedef u8 RadioPowerMode;

//---------------------------------------------------------------------------
// Transmit Configuration
//-----------------------
// The application must declare this structure and initialize each member
// variable.  radioTransmitConfig must be initialized prior to calling
// ST_RadioTransmit() and may be modified only while no transmit operation is
// in progress.

#define RADIO_CCA_ATTEMPT_MAX_DEFAULT      4
#define RADIO_BACKOFF_EXPONENT_MIN_DEFAULT 3
#define RADIO_BACKOFF_EXPONENT_MAX_DEFAULT 5

typedef struct {
  boolean waitForAck;       // Wait for ACK if ACK request set in FCF.
  boolean checkCca;         // backoff and check CCA before transmit.
  u8 ccaAttemptMax;      // The number of CCA attempts before failure;
  u8 backoffExponentMin; // Backoff exponent for the initial CCA attempt.
  u8 backoffExponentMax; // Backoff exponent for the final CCA attempt(s).
  boolean appendCrc;        // Append CRC to transmitted packets.
} RadioTransmitConfig;
#endif// PHY_H_

#ifndef ST_TYPES_H
/**
 * @name txPowerModes for stSetTxPowerMode and mfglibSetPower 
 */
//@{

/** @brief The application should call ::stSetTxPowerMode() with the
  * txPowerMode parameter set to this value to disable all power mode options,
  * resulting in normal power mode and bi-directional RF transmitter output.
  */
#define ST_TX_POWER_MODE_DEFAULT             0x0000
/** @brief The application should call ::stSetTxPowerMode() with the
  * txPowerMode parameter set to this value to enable boost power mode.
  */
#define ST_TX_POWER_MODE_BOOST               0x0001
/** @brief The application should call ::stSetTxPowerMode() with the
  * txPowerMode parameter set to this value to enable the alternate transmitter
  * output.
  */
#define ST_TX_POWER_MODE_ALTERNATE           0x0002
/** @brief The application should call ::stSetTxPowerMode() with the
  * txPowerMode parameter set to this value to enable both boost mode and the
  * alternate transmitter output.
  */
#define ST_TX_POWER_MODE_BOOST_AND_ALTERNATE (ST_TX_POWER_MODE_BOOST     \
                                                |ST_TX_POWER_MODE_ALTERNATE)
#ifndef DOXYGEN_SHOULD_SKIP_THIS
// The application does not ever need to call stSetTxPowerMode() with the
// txPowerMode parameter set to this value.  This value is used internally by
// the stack to indicate that the default token configuration has not been
// overidden by a prior call to stSetTxPowerMode().
#define ST_TX_POWER_MODE_USE_TOKEN           0x8000
#endif//DOXYGEN_SHOULD_SKIP_THIS
//@} \\END  Definitions

/**
 * @brief The maximum 802.15.4 channel number is 26.
 */
#define ST_MAX_802_15_4_CHANNEL_NUMBER 26

/**
 * @brief The minimum 802.15.4 channel number is 11.
 */
#define ST_MIN_802_15_4_CHANNEL_NUMBER 11

/**
 * @brief There are sixteen 802.15.4 channels.
 */
#define ST_NUM_802_15_4_CHANNELS \
  (ST_MAX_802_15_4_CHANNEL_NUMBER - ST_MIN_802_15_4_CHANNEL_NUMBER + 1)

#endif //ST_TYPES_H




//---------------------------------------------------------------------------
/** @name
 * Radio power state control APIs
 *
 * @brief
 * These APIs control the overall radio initialization and power state.
 */
//@{

/** @brief 
 * This function performs one-time radio initialization and calibration.
 * This function must be called once after chip reset or wake from deep sleep.
 * This function will power up the radio while it configures the radio channel.
 * After the radio is configured and the channel is set the radio will be left
 * in the state specified by the \c initialRadioPowerMode parameter.
 * This function calls ST_RadioSetPowerMode(),
 * ST_RadioEnableAddressFiltering(), ST_RadioEnableAutoAck(),
 * ST_RadioSetCoordinator(), ST_RadioSetPower(), ST_RadioSetChannel(),
 * ST_RadioEnablePacketTrace(), and ST_RadioEnableReceiveCrc()
 * providing the last used argument for each function.  If these functions have
 * not been called before ST_RadioInit() then the default for each is used.
 * Only the functions listed above can be called before ST_RadioInit().
 * All other library functions must not be called until after
 * ST_RadioInit() has been called once after each chip reset or wake from deep
 * sleep.
 *
 * @param
 * initialRadioPowerMode specifies the state that the function will leave the
 * radio in after it is configured.  This parameter can be either:
 * ST_RADIO_POWER_MODE_OFF - radio will be powered down.
 * ST_RADIO_POWER_MODE_RX_ON - radio will be left in the on (receive) state.
 *
 * @return ::ST_SUCCESS or a status code indicating the reason for failure.
 */
StStatus ST_RadioInit(RadioPowerMode initialRadioPowerMode);

/** @brief 
 * Turns on the radio.  The last radio configuration is used.
 */
void ST_RadioWake(void);

/** @brief 
 * This function turns off the radio.
 * Any transmit or receive packets in progress are aborted.
 * The library may or may not call ST_RadioTransmitCompleteIsrCallback() for an
 * aborted transmit packet.
 * ST_RadioWake() must not be called within 250us of having called
 * ST_RadioSleep().
 */
void ST_RadioSleep(void);

//@}//END Radio power state control APIs


//---------------------------------------------------------------------------
/** @name
 * Channel APIs
 *
 * @brief
 * These APIs control channel selection and calibration.
 */
//@{

/** @brief 
 * Configures the radio to communicate on a 802.15.4 channel.
 * If ST_RadioInit() has not been called yet, the channel number is recorded
 * but no calibration takes place.  The specified channel will be configured
 * when ST_RadioInit() is eventually called.
 * If the radio is asleep this function will wake it to perform channel
 * calibration and then return it to sleep before exiting.
 * The first time a channel is selected all radio parameters will be calibrated
 * for that channel.  This full calibration process can take up to 200ms to
 * complete.  The results of some of these calibrations are stored in flash
 * tokens for use the next time the same channel is selected.  Subsequent calls
 * to ST_RadioSetChannel() requesting the same channel will take less time
 * because the stored values will be retrieved from the flash tokens and reused.
 *
 * @param channel the 802.15.4 channel that the radio will communicate on.
 *
 * @return ::ST_SUCCESS or a status code indicating the reason for failure.
 */
StStatus ST_RadioSetChannel(u8 channel);

/** @brief
 * This function gets the 802.15.4 channel that the radio is configured to use.
 *
 * @return the 802.15.4 channel that the radio is configured to use.
 */
u8 ST_RadioGetChannel(void);

/** @brief
 * This function is identical to ST_RadioSetChannel() except that it
 * calibrates all radio parameters regardless of whether calibration is
 * required.
 *
 * NOTE: This function does not need to be called under normal operating
 * conditions.  The library will reuse available stored calibration values
 * and will perform necessary re-calibration automatically.  Calling this
 * function will cause the library to calibrate all radio parameters and
 * store the results in flash, overwriting previous stored values.  Use of
 * this function will cause unnecessary flash wear and will take longer to
 * complete than a call to ST_RadioSetChannel().  This function should only
 * be called to recover from hardware-related calibration problems, which
 * should only occur during hardware development.
 * 
 * @param channel the 802.15.4 channel that the radio will communicate on.
 *
 * @return ::ST_SUCCESS or a status code indicating the reason for failure.
 */
StStatus ST_RadioSetChannelAndForceCalibration(u8 channel);

/** @brief
 * This function checks to see if the radio needs calibrating to maintain
 * expected performance as environmental conditions change.
 * If this function indicates that the calibration is needed, the application
 * should call ST_RadioCalibrateCurrentChannel().
 *
 * NOTE: This function must not be called while a transmit is in progress.
 *
 * @return TRUE if calibration is needed to compensate for temperature changes,
 * FALSE otherwise.
 */
boolean ST_RadioCheckRadio(void);

/** @brief
 * This function performs necessary recalibration to counteract the effects of
 * temperature changes since the last calibration.  It should be called by the
 * application when ST_RadioCheckRadio() indicates that calibration is needed.
 */
void ST_RadioCalibrateCurrentChannel(void);

//@}//END Channel APIs


//---------------------------------------------------------------------------
/** @name
 * Transmit APIs
 *
 * @brief
 * These APIs control the transmission of packets.
 */
//@{

/** @brief
 * This function transmits a packet using the configuration specified in
 * \c radioTransmitConfig.
 *
 * @param *packet A pointer to the packet to be transmitted.  The first byte of
 * \c packet must be set to the number of payload bytes to be transmitted.
 * If \c radioTransmitConfig.appendCrc is TRUE the length byte must accommodate
 * the hardware-appended two-byte CRC.
 * e.g. A packet with a two-byte payload would be represented in memory as:
 * {0x04, 0x00, 0x01, 0xc0, 0xc1} where 0xc0 and 0xc1 are the CRC bytes.
 * If \c radioTransmitConfig.checkCca is TRUE this function performs CSMA-CA
 * backoff(s) and CCA check(s) before transmitting, else it starts the
 * transmit process immediately.
 * 
 * @return ::ST_SUCCESS if the transmission process is successfully started.
 * In this case ST_RadioTransmitCompleteIsrCallback() will eventually be
 * called to indicate the completion status.  If the radio is busy transmitting,
 * this function returns an error code and
 * ST_RadioTransmitCompleteIsrCallback() will not be called.
 */
StStatus ST_RadioTransmit(u8* packet);

/** @brief
 * This function is called by the library once after each ST_RadioTransmit()
 * call that returned successfully.  If the ST_RadioTransmit() call returned
 * an error ST_RadioTransmitCompleteIsrCallback() will not be called.
 * 
 * NOTE: ST_RadioTransmit() can be called again within the context of this
 * callback.
 * 
 * @param status parameter indicates one of the following conditions:
 * ::ST_SUCCESS - the last byte of the non-ACK-request packet has been
 * transmitted.
 * ::ST_PHY_ACK_RECEIVED - the requested ACK was received.
 * ::ST_MAC_NO_ACK_RECEIVED - the requested ACK was not received in time.
 * ::ST_PHY_TX_CCA_FAIL - unable to transmit due to lack of clear channel on
 * all attempts.
 * ::ST_PHY_TX_UNDERFLOW - DMA underflow occurred while transmitting.  Should
 * never happen.
 * ::ST_PHY_TX_INCOMPLETE - The PLL synthesizer failed to lock while
 * transmitting.  Should never happen.
 * 
 * @param sfdSentTime the value of the MAC timer captured when the SFD was sent.
 *
 * @param framePending TRUE if the received ACK indicates that data is
 * pending for this node, FALSE otherwise.
 */
extern void ST_RadioTransmitCompleteIsrCallback(StStatus status,
                                                  u32 sfdSentTime,
                                                  boolean framePending);

/** @brief
 * This function sets the Energy Detection Clear Channel Assessment threshold.
 *
 * @param threshold the energy level in dBm below which the channel will be
 * considered clear.
 */
void ST_RadioSetEdCcaThreshold(s8 threshold);

/** @brief
 * This function get the Energy Detection Clear Channel Assessment threshold.
 *
 * @return the Energy Detection Clear Channel Assessment threshold in units of
 * dBm.
 */
s8 ST_RadioGetEdCcaThreshold(void);

/** @brief This function enables or disables notification of the SFD sent event
 * via the ST_RadioSfdSentIsrCallback().
 *
 * @param enable TRUE if the notification is to be enabled, FALSE if the
 * notification is to be disabled.
 */
void ST_RadioEnableSfdSentNotification(boolean enable);

/** @brief
 * This function indicates whether the SFD sent notification via
 * \c ST_RadioSfdSentIsrCallback() is enabled or disabled.
 *
 * @return TRUE if the SFD sent notification is enabled, FALSE otherwise.
 */
boolean ST_RadioSfdSentNotificationEnabled(void);

/** @brief
 * This function is called by the library in response to an SFD sent event if
 * this notification has been enabled by a call to
 * \c ST_RadioEnableSfdSentNotification().
 *
 * NOTE: This callback is called for ACKs as well as normal packets.
 * 
 * NOTE: In cases of extreme interrupt latency it is possible that 
 * \c sfdSentTime may contain the SFD time of the last received packet instead
 * of the time of the last transmitted packet.
 *
 * @param sfdSentTime the value of the MAC timer when the SFD was sent in the
 * last transmitted packet.
 */
void ST_RadioSfdSentIsrCallback(u32 sfdSentTime);

/** @brief
 * This function sets the radio transmit power to a new level within the minimum
 * and maximum values specified in the datasheet for the specific device.
 *
 * NOTE: It is possible for this function to succeed and set the power level to
 * a value other than that specified in the \c power parameter.  The reason for
 * for this behavior is that not all integer power levels are available at lower
 * power levels.  When a specific power level is not available the next higher
 * power level is used.
 *
 * @return ::ST_SUCCESS if the power level has been changed, or an error
 * status code if not (e.g. if the requested value is out of range).
 *
 * @param power the desired power level in units of dBm.
 */
StStatus ST_RadioSetPower(s8 power);

/** @brief
 * This function gets the radio transmit power level.
 * 
 * @return the radio transmit power level in units of dBm.
 */
s8 ST_RadioGetPower(void);

//@}//END Transmit APIs


//---------------------------------------------------------------------------
/** @name
 * Receive APIs
 *
 * @brief
 * These APIs control the reception of packets.
 */
//@{

/** @brief
 * This function is called by the library when a packet has been received.
 *
 * @param *packet points to the packet data beginning with the length byte.
 * The CRC bytes will have been removed from the packet.
 *
 * @param ackFramePendingSet TRUE if the library set the Frame Pending bit in
 * the hardware-generated MAC ACK to this packet, FALSE otherwise.
 *
 * @param time The value of the MAC timer when the SFD was received for this
 * packet.
 *
 * @param errors The number of correlator errors in the packet.
 *
 * @param rssi The energy detected over the last 8 symbols of the packet in
 * units of dBm.
 */
extern void ST_RadioReceiveIsrCallback(u8 *packet,
                                         boolean ackFramePendingSet,
                                         u32 time,
                                         u16 errors,
                                         s8 rssi);

/** @brief
 * This function enables or disables address filtering on PAN ID, node ID, and
 * EUI 64.
 *
 * NOTE: Address filtering is enabled by default.
 *
 * @param enable TRUE to enable address filtering, FALSE otherwise.
 */
void ST_RadioEnableAddressFiltering(boolean enable);

/** @brief
 * This function gets the address filtering status of the device.
 *
 * @return TRUE if address filtering is enabled, FALSE otherwise.
 */ 
boolean ST_RadioAddressFilteringEnabled(void);

/** @brief
 * This function enables or disables automatic transmission of ACKs in response
 * to received packets which request ACKs.
 *
 * NOTE: Address filtering must be enabled for automatic transmission of ACKs to
 * occur.
 *
 * NOTE: Automatic acknowledgement is enabled by default.
 *
 * @param enable TRUE to enable automatic acknowledgement transmission, FALSE
 * otherwise.
 */
void ST_RadioEnableAutoAck(boolean enable);

/** @brief
 * This function gets the automatic acknowledgement status of the device.
 *
 * @return TRUE if automatic acknowledgement is enabled, FALSE otherwise.
 */
boolean ST_RadioAutoAckEnabled(void);

/** @brief
 * This function sets the short address of the node.
 *
 * @param nodeId the 16-bit address to use for filtering short-addressed
 * packets when address filtering is enabled.
 *
 */
void ST_RadioSetNodeId(u16 nodeId);

/** @brief
 * This function gets the short address of the node.
 *
 * @return nodeId the 16-bit address to use for filtering short-addressed
 * packets.
 */
u16 ST_RadioGetNodeId(void);

/** @brief
 * This function sets the PAN id of the node.
 *
 * @param panId the 16-bit PAN id to use for filtering packets when address
 * filtering is enabled.
 */
void ST_RadioSetPanId(u16 panId);

/** @brief
 * This function gets the PAN id of the node.
 *
 * @return 16-bit PAN id to use for filtering packets when address
 * filtering is enabled.
 */
u16 ST_RadioGetPanId(void);

/** @brief
 * This function get the EUI 64 of the node.
 *
 * NOTE: The EUI 64 is set via manufacturing tokens (See the Programming and
 * Serialization Specification for details).
 *
 * @return the memory address of the 64-bit EUI address to use for filtering
 * long-addressed packets when address filtering is enabled.
 */
u8* ST_RadioGetEui64(void);

/** @brief
 * This function is called by the library after the long address fields of a
 * packet have been received.  The library will set the frame pending bit in the
 * outgoing ACK only if the return value is TRUE.  The application must lookup
 * the \c eui64 in its own data structures and return TRUE if there is data
 * pending, FALSE otherwise.  It is critical that this function complete as
 * quickly as possible to ensure the frame pending bit can be set before the ACK
 * is transmitted.
 *
 * @return TRUE if the frame pending bit should be set in the outgoing ACK.
 */
boolean ST_RadioDataPendingLongIdIsrCallback(u8* longId);

/** @brief
 * This function is called by the library after the short address fields of a
 * packet have been received.  The library will set the frame pending bit in the
 * outgoing ACK only if the return value is TRUE.  The application must lookup
 * \c shortId in its own data structures and return TRUE if there is data
 * pending, FALSE otherwise.  It is critical that this function complete as
 * quickly as possible to ensure the frame pending bit can be set before the ACK
 * is transmitted.
 *
 * @return TRUE if the frame pending bit should be set in the outgoing ACK.
 */
boolean ST_RadioDataPendingShortIdIsrCallback(u16 shortId);

/** @brief
 * This function sets or clears coordinator mode for this node.  A
 * coordinator is able to receive 802.15.4. DATA frames that have no destination
 * address.  A node that is not a coordinator will not receive these packets.
 * 
 * NOTE: The source PAN id of the DATA frame with no destination address must
 * still match the node PAN id in order for it to be received by the
 * coordinator node.
 *
 * NOTE: A node is not a coordinator by default.
 *
 * @param coordinator TRUE to enable coordinator mode, FALSE to disable
 * coordinator mode.
 */
void ST_RadioSetCoordinator(boolean coordinator);

/** @brief
 * This function gets the coordinator status of the node.
 *
 * @return TRUE if the node is configured as a coordinator, FALSE otherwise.
 */
boolean ST_RadioDeviceIsCoordinator(void);

/** @brief
 * This function enables or disables notification of DMA receive buffer overflow
 * events via ST_RadioOverflowIsrCallback().
 *
 * @param enable TRUE to enable overflow notification, FALSE otherwise.
 */
void ST_RadioEnableOverflowNotification(boolean enable);

/** @brief
 * This function indicates whether the overflow notification via
 * ST_RadioOverflowIsrCallback() is enabled or disabled.
 *
 * @return TRUE if overflow notification is enabled, FALSE otherwise.
 */
boolean ST_RadioOverflowNotificationEnabled(void);

/** @brief
 * This function is called by the library in response to a receive overflow
 * event if this notification is enabled by a call to
 * ST_RadioEnableOverflowNotification().
 */
void ST_RadioOverflowIsrCallback(void);

/** @brief
 * This function enables or disables discarding received packets that fail the
 * Cyclic Redundancy Check.
 *
 * NOTE: When this option is enabled the library automatically strips the CRC
 * bytes off of packets that pass CRC check.
 *
 * NOTE: Discarding packets that fail CRC is enabled by default.
 *
 * @param enable TRUE to enable discarding packets that fail CRC, FALSE
 * otherwise.
 */
void ST_RadioEnableReceiveCrc(boolean enable);

/** @brief
 * This function gets the receive CRC configuration of the node.
 *
 * @return TRUE if received packets that fail CRC will be discarded, FALSE
 * otherwise.
 */
boolean ST_RadioReceiveCrcEnabled(void);

//@}//END Receive APIs


//---------------------------------------------------------------------------
/** @name
 * MAC Timer APIs
 *
 * @brief
 * These APIs expose an interface to the MAC Timer.
 * The MAC timer is 20-bits long with each LSB tick representing 1us.
 * The MAC timer rolls over to zero approximately once every second.
 * The MAC timer is free-running from the time that ST_RadioInit() is called.
 */
//@{

/** @brief
 * This function gets an instantaneous reading of the MAC timer.
 *
 * @return the current value of the MAC timer.
 */
u32 ST_RadioGetMacTimer(void);

/** @brief
 * This function enables or disables comparison of the MAC timer against an
 * application-supplied value and notification via
 * ST_RadioMacTimerCompareIsrCallback().
 *
 * @param enable TRUE to enable MAC timer comparison and notification via a
 * callback.
 */
void ST_RadioEnableMacTimerCompare(boolean enable);

/** @brief
 * This function indicates whether MAC timer comparison and callback
 * notification are enabled or disabled.
 *
 * @return TRUE if MAC timer comparison and notification are enabled, FALSE
 * otherwise.
 */
boolean ST_RadioMacTimerCompareEnabled(void);

/** @brief
 * This function sets the value to compare against the MAC timer.
 *
 * @param value the value to compare against the MAC timer.
 */
void ST_RadioSetMacTimerCompare(u32 value);

/** @brief
 * This function gets the value to compare against the MAC timer.
 *
 * @return the value to compare against the MAC timer.
 */
u32 ST_RadioGetMacTimerCompare(void);

/** @brief
 * This function is called by the library in response to MAC timer comparison
 * event.
 */
extern void ST_RadioMacTimerCompareIsrCallback(void);
//@}//END MAC Timer APIs


//---------------------------------------------------------------------------
/** @name
 * Cryptography APIs
 *
 * @brief
 * These APIs provide an interface to the hardware AES coprocessor.
 */
//@{

/** @brief
 * This function sets the key to use during AES encryption.
 *
 * @param *key pointer to 128 bits of key data.
 */
void ST_AesSetKey(u8* key);

/**
 * This function gets the key that is used during AES encryption.
 *
 * @param *key pointer to memory where 128 bits of key data will be written.
 */
void ST_AesGetKey(u8* key);

/** @brief
 * This function encrypts the 128 bits of plaintext data located at \c block
 * using the AES coprocessor previously configured by ST_AesSetKey().
 * The resulting 128 bits of cyphertext are stored at \c block, overwriting
 * the supplied plaintext.
 *
 * @param block pointer to memory containing the plaintext when this function is
 * called and containing the cyphertext after this function has returned.
 */
void ST_AesEncrypt(u8* block);

//@}//END Cryptography APIs


//---------------------------------------------------------------------------
/** @name
 * Miscellaneous APIs
 *
 * @brief
 * These APIs control diagnostic and configuration functionality.
 */
//@{

/** @brief
 * This function starts transmission of a carrier wave at the current channel
 * center frequency.  The carrier wave will be transmitted until
 * ST_RadioStopTransmitTone() is called.
 *
 * NOTE: The radio must be idle (not transmitting) before entering this mode.
 * 
 * NOTE: Other radio APIs must not be called while in this mode.
 */
void ST_RadioStartTransmitTone(void);

/** @brief
 * This function stops transmission of carrier wave started by
 * ST_RadioStartTransmitTone().
 */
void ST_RadioStopTransmitTone(void);

/** @brief
 * this function starts transmission of a continuous stream of modulated data.
 * No packet framing is present in this transmission.  Random symbols will be
 * transmitted until ST_RadioStopTransmitStream() is called.
 *
 * NOTE: The radio must be idle (not transmitting) before entering this mode.
 * 
 * NOTE: Other radio APIs must not be called while in this mode.
 */
void ST_RadioStartTransmitStream(void);

/** @brief
 * This function stops transmission of continuous stream of modulated data
 * started by ST_RadioStartTransmitStream().
 */
void ST_RadioStopTransmitStream(void);

/** @brief
 * This function gets a reading of the average energy detected over the previous
 * eight symbol periods (128us total).
 *
 * @return the energy level detected in units of dBm.
 */
s8 ST_RadioEnergyDetection(void);

/** @brief
 * This function accesses radio hardware to obtain true random numbers.
 *
 * @param *rn pointer to memory to hold \c count random numbers.
 * 
 * @param count the number of 16-bit random numbers to get.
 */
void ST_RadioGetRandomNumbers(u16 *rn, u8 count);

/** @brief
 * This function gets the clear or busy status of the channel.
 *
 * @return TRUE if channel is clear, FALSE if channel is busy.
 */
boolean ST_RadioChannelIsClear(void);

/** @brief
 * This function enables or disables Packet Trace output.
 * Before being enabled, the associated IO pins must be separately 
 * configured to allow for the packet trace peripheral to control the pins.
 *
 * NOTE: Packet Trace is on by default.
 *
 * @param enable TRUE to enable Packet Trace, FALSE otherwise.
 */
void ST_RadioEnablePacketTrace(boolean enable);

/** @brief
 * This function indicates whether Packet Trace is enabled or not.
 *
 * @return TRUE if Packet Trace is enabled, FALSE otherwise.
 */
boolean ST_RadioPacketTraceEnabled(void);

/** @brief
 * This function sets the radio power mode according to the bits
 * encoded in \c powerMode.
 *
 * NOTE: The default power mode is whatever is configured in the PHY_CONFIG
 * token (normal, bi-directional mode if not explicitly set otherwise.
 *
 * NOTE: It is preferable to set this configuration via the PHY_CONFIG token
 * rather than using this API.
 *
 * @param txPowerMode encodes the power mode as follows:
 * bit 0 set to 0: Normal mode.
 * bit 0 set to 1: Boost mode.
 * bit 1 set to 0: Use bi-directional transmit path.
 * bit 1 set to 1: Use alternate transmit path.
 *
 * @return ::ST_SUCCESS always.
 */
StStatus ST_RadioSetPowerMode(u16 txPowerMode);

/** @brief
 * This function gets the radio power mode.
 * 
 * @return the radio power mode (boost/normal, bi-directional/alternate transmit
 * path) encoded as bits in a 16-bit word (see ST_RadioSetPowerMode()
 * documentation for details).
 */
u16 ST_RadioGetPowerMode(void);

//@}//END Miscellaneous APIs
