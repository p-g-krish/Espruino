/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Common low-level device handling (Events, IO buffers)
 * ----------------------------------------------------------------------------
 */
#ifndef JSDEVICES_H_
#define JSDEVICES_H_

#include "jsutils.h"
#include "platform_config.h"
#include "jsvar.h"

/** Initialize any device-specific structures, like flow control states.
 * Called from jshInit */
void jshInitDevices();

/** Reset any devices that could have been set up differently by JS code.
 * Called from jshReset */
void jshResetDevices();

/** Flags used to describe events put in the txBuffer and ioBuffer queues.
 *
 * This should be 1 byte. Bottom 6 bits are the type of device, and the
 * top 2 bits are extra info (number of characters, serial errors, whether
 * device is high or similar).
 */
typedef enum {
  // device type
  EV_NONE,
  EV_EXTI0,  ///< External Interrupt
  EV_EXTI_MAX = EV_EXTI0 + ESPR_EXTI_COUNT - 1,
  EV_SERIAL_START,
  EV_LOOPBACKA = EV_SERIAL_START,
  EV_LOOPBACKB,
  EV_LIMBO,     ///< Where console output goes right after boot - one sec later we move it to USB/Serial
#ifdef USE_TELNET
  EV_TELNET,
#endif
#ifdef USE_TERMINAL
  EV_TERMINAL, // Built-in VT100 terminal
#endif
  EV_SERIAL_DEVICE_STATE_START, // The point at which we start storing device state (jshSerialDevice*)
  _EV_SERIAL_DEVICE_STATE_START_MINUS_ONE=EV_SERIAL_DEVICE_STATE_START-1, // means that the next enum should==EV_SERIAL_DEVICE_STATE_START
#ifdef USB
  EV_USBSERIAL, ///< USB CDC Serial Data
#endif
#ifdef BLUETOOTH
  EV_BLUETOOTH, ///< Bluetooth LE
#endif
#ifdef USE_SWDCON
  EV_SWDCON, /// console over in memory buffer accessible via SWD
#endif
#if ESPR_USART_COUNT>=1
  EV_SERIAL1, // Used for IO for UARTS
#endif
#if ESPR_USART_COUNT>=2
  EV_SERIAL2,
#endif
#if ESPR_USART_COUNT>=3
  EV_SERIAL3,
#endif
#if ESPR_USART_COUNT>=4
  EV_SERIAL4,
#endif
#if ESPR_USART_COUNT>=5
  EV_SERIAL5,
#endif
#if ESPR_USART_COUNT>=6
  EV_SERIAL6,
#endif
#if ESPR_USART_COUNT>=1
  EV_SERIAL_MAX = EV_SERIAL1 + ESPR_USART_COUNT - 1,
  EV_SERIAL1_STATUS, // Used to store serial status info
  EV_SERIAL_STATUS_MAX = EV_SERIAL1_STATUS + ESPR_USART_COUNT - 1,
#else
  _EV_SERIAL_MAX_PLUS_ONE,
  EV_SERIAL_MAX = _EV_SERIAL_MAX_PLUS_ONE-1,
#endif
#ifdef BLUETOOTH
  EV_BLUETOOTH_PENDING,      // Tasks that came from the Bluetooth Stack in an IRQ
#endif
  EV_CUSTOM, ///< Custom event (See IOCustomEventFlags)
#ifdef BANGLEJS
  EV_BANGLEJS,               // sent whenever Bangle.js-specific data needs to be queued
#endif
#if ESPR_SPI_COUNT>=1
  EV_SPI1, ///< SPI Devices
#endif
#if ESPR_SPI_COUNT>=2
  EV_SPI2,
#endif
#if ESPR_SPI_COUNT>=3
  EV_SPI3,
#endif
#if ESPR_SPI_COUNT>=1
  EV_SPI_MAX = EV_SPI1 + ESPR_SPI_COUNT - 1,
#endif
#if ESPR_I2C_COUNT>=1
  EV_I2C1, ///< I2C Devices
#endif
#if ESPR_I2C_COUNT>=2
  EV_I2C2,
#endif
#if ESPR_I2C_COUNT>=3
  EV_I2C3,
#endif
#if ESPR_I2C_COUNT>=1
  EV_I2C_MAX = EV_I2C1 + ESPR_I2C_COUNT - 1,
#endif
  EV_DEVICE_MAX,
  // EV_DEVICE_MAX should not be >64 - see DEVICE_INITIALISED_FLAGS
  EV_TYPE_MASK = NEXT_POWER_2(EV_DEVICE_MAX) - 1,
  // ----------------------------------------- SERIAL STATUS
  EV_SERIAL_STATUS_FRAMING_ERR = EV_TYPE_MASK+1,
  EV_SERIAL_STATUS_PARITY_ERR = EV_SERIAL_STATUS_FRAMING_ERR<<1,
  // ----------------------------------------- WATCH EVENTS
  EV_EXTI_IS_HIGH = EV_TYPE_MASK+1,           //< if the pin we're watching is high, the handler sets this
  EV_EXTI_DATA_PIN_HIGH = EV_EXTI_IS_HIGH<<1  //< If a data pin was specified, its value is high. OR on Bangle.js it causes us not to call user code
} PACKED_FLAGS IOEventFlags; // should be one byte

#define DEVICE_SANITY_CHECK() if (EV_TYPE_MASK>63) jsError("DEVICE_SANITY_CHECK failed")

/** Event types for EV_CUSTOM */
typedef enum {
  EVC_NONE,
#ifdef NRF52_SERIES
  EVC_LPCOMP, // jswrap_espruino: E.setComparator / E.on("comparator" event
#endif
  EVC_TYPE_MASK = 255,
  EVC_DATA_LPCOMP_UP = 256
} PACKED_FLAGS IOCustomEventFlags;

/// True is the device is a serial device (could be a USART, Bluetooth, USB, etc)
#define DEVICE_IS_SERIAL(X) (((X)>=EV_SERIAL_START) && ((X)<=EV_SERIAL_MAX))
/// True if the device has device state stored for it (eg. flow control state)
#define DEVICE_HAS_DEVICE_STATE(X) (((X)>=EV_SERIAL_DEVICE_STATE_START) && ((X)<=EV_SERIAL_MAX))
/// If DEVICE_HAS_DEVICE_STATE, this is the index where device state is stored
#define TO_SERIAL_DEVICE_STATE(X) ((X)-EV_SERIAL_DEVICE_STATE_START)

#if ESPR_USART_COUNT>=1
/// Return true if the device is a USART (hardware serial)
#define DEVICE_IS_USART(X) (((X)>=EV_SERIAL1) && ((X)<=EV_SERIAL_MAX))
#define DEVICE_IS_USART_STATUS(X) (((X)>=EV_SERIAL1_STATUS) && ((X)<=EV_SERIAL_STATUS_MAX))
#else
#define DEVICE_IS_USART(X) (false)
#define DEVICE_IS_USART_STATUS(X) (false)
#endif

// Return true if the device is an SPI.
#if ESPR_SPI_COUNT>=1
#define DEVICE_IS_SPI(X) (((X)>=EV_SPI1) && ((X)<=EV_SPI_MAX))
#else
#define DEVICE_IS_SPI(X) (false)
#endif
#if ESPR_I2C_COUNT>=1
#define DEVICE_IS_I2C(X) (((X)>=EV_I2C1) && ((X)<=EV_I2C_MAX))
#else
#define DEVICE_IS_I2C(X) (false)
#endif
#define DEVICE_IS_EXTI(X) (((X)>=EV_EXTI0) && ((X)<=EV_EXTI_MAX))

#if ESPR_USART_COUNT>=1
#define IOEVENTFLAGS_SERIAL_TO_SERIAL_STATUS(X) ((X) + EV_SERIAL1_STATUS - EV_SERIAL1)
#define IOEVENTFLAGS_SERIAL_STATUS_TO_SERIAL(X) ((X) + EV_SERIAL1 - EV_SERIAL1_STATUS)
#endif

#define IOEVENTFLAGS_GETTYPE(X) ((X)&EV_TYPE_MASK)

// maximum length for an event. BLE is the biggest event we have so deal with that if we need to
#define IOEVENT_MAX_LEN 64
#if defined(NRF_SDH_BLE_GATT_MAX_MTU_SIZE) && NRF_SDH_BLE_GATT_MAX_MTU_SIZE+3>IOEVENT_MAX_LEN
#undef IOEVENT_MAX_LEN
#define IOEVENT_MAX_LEN (NRF_SDH_BLE_GATT_MAX_MTU_SIZE+3)
#endif

#include "jspin.h"

/// Push an IO event (max IOEVENT_MAX_LEN) into the ioBuffer (designed to be called from IRQ), returns true on success, Calls jshHadEvent();
bool CALLED_FROM_INTERRUPT jshPushEvent(IOEventFlags evt, uint8_t *data, unsigned int length);
/// Add this IO event to the IO event queue. Calls jshHadEvent();
void jshPushIOEvent(IOEventFlags channel, JsSysTime time);
/// Signal an IO watch event as having happened. Calls jshHadEvent();
void jshPushIOWatchEvent(IOEventFlags channel);
/// Push a single character event (for example USART RX)
void jshPushIOCharEvent(IOEventFlags channel, char ch);
/// Push many character events at once (for example USB RX)
void jshPushIOCharEvents(IOEventFlags channel, char *data, unsigned int count);

/// pop an IO event, returns EV_NONE on failure. data must be IOEVENT_MAX_LEN bytes
IOEventFlags jshPopIOEvent(uint8_t *data, unsigned int *length);
// pop an IO event of type eventType, returns true on success. data must be IOEVENT_MAX_LEN bytes
IOEventFlags jshPopIOEventOfType(IOEventFlags eventType, uint8_t *data, unsigned int *length);
/// Do we have any events pending? Will jshPopIOEvent return true?
bool jshHasEvents();
/// Check if the top event is for the given device
bool jshIsTopEvent(IOEventFlags eventType);

/// How many event blocks are left? compare this to IOBUFFERMASK
int jshGetEventsUsed();

/// Do we have enough space for N characters?
bool jshHasEventSpaceForChars(int n);
/// How many characters can we write?
int jshGetIOCharEventsFree();

const char *jshGetDeviceString(IOEventFlags device);
IOEventFlags jshFromDeviceString(const char *device);

/// Gets a device's object from a device, or return 0 if it doesn't exist
JsVar *jshGetDeviceObject(IOEventFlags device);

// ----------------------------------------------------------------------------
//                                                         DATA TRANSMIT BUFFER
/// Queue a character for transmission
void jshTransmit(IOEventFlags device, unsigned char data);
// Queue a formatted string for transmission
void jshTransmitPrintf(IOEventFlags device, const char *fmt, ...);
/// Wait for transmit to finish
void jshTransmitFlush();
/// Wait for all data in the transmit queue to be written for a specific device - this can hang if the device isn't being emptied!
void jshTransmitFlushDevice(IOEventFlags device);
/// Clear everything from a device
void jshTransmitClearDevice(IOEventFlags device);
/// Move all output from one device to another
void jshTransmitMove(IOEventFlags from, IOEventFlags to);
/// Do we have anything we need to send?
bool jshHasTransmitData();
// Return the device at the top of the transmit queue (or EV_NONE)
IOEventFlags jshGetDeviceToTransmit();
/// Try and get a character for transmission - could just return -1 if nothing
int jshGetCharToTransmit(IOEventFlags device);


/// Set whether the host should transmit or not
void jshSetFlowControlXON(IOEventFlags device, bool hostShouldTransmit);

/// To be called on idle when the input queue has enough space
void jshSetFlowControlAllReady();

/// Set whether to use flow control on the given device or not. Whether to use software, and if hardware, the pin to use for RTS
void jshSetFlowControlEnabled(IOEventFlags device, bool software, unsigned char/*Pin*/ pinCTS);

// Functions that can be called in an IRQ when a pin changes state
typedef void(*JshEventCallbackCallback)(bool state, IOEventFlags flags);

/// Set a callback function to be called when an event occurs
void jshSetEventCallback(IOEventFlags channel, JshEventCallbackCallback callback);

/// Set a data pin to be read when an event occurs. Shares same storage as jshSetEventCallback
void jshSetEventDataPin(IOEventFlags channel, Pin pin);

/// Get a data pin to be read when an event occurs
Pin jshGetEventDataPin(IOEventFlags channel);

/// Set whether a Serial device puts framing/parity errors into the input queue
void jshSetErrorHandlingEnabled(IOEventFlags device, bool errorHandling);

/// Get whether a Serial device puts framing/parity errors into the input queue
bool jshGetErrorHandlingEnabled(IOEventFlags device);

#endif /* JSDEVICES_H_ */
