#pragma once
#include "pl_common.h"

#if defined(CONFIG_IDF_TARGET_ESP32P4) || defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3)

#include "tinyusb.h"

//==============================================================================

namespace PL {

//==============================================================================

/// @brief USB device class
class UsbDevice : Lockable {
  friend class UsbDeviceCdc;

public:
  /// @brief Creates a USB device
  /// @param device USB device
  /// @param vbusMonitorPin GPIO that is connected to VBUS monitor circuit
  UsbDevice(tinyusb_usbdev_t device, int vBusMonitorPin = -1);
  ~UsbDevice();
  UsbDevice(const UsbDevice&) = delete;
  UsbDevice& operator=(const UsbDevice&) = delete;
  
  esp_err_t Lock(TickType_t timeout = portMAX_DELAY) override;
  esp_err_t Unlock() override;

  /// @brief Initializes the USB device
  /// @return error code
  esp_err_t Initialize();

private:
  Mutex mutex;
  bool initialized = false;
  tinyusb_usbdev_t device;
  int vBusMonitorPin = vBusMonitorPin;
};

//==============================================================================

}

#endif