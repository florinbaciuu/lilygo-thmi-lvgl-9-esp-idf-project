#pragma once
#include "pl_usb_device.h"

#if CONFIG_TINYUSB_CDC_ENABLED

#include "tusb_cdc_acm.h"

//==============================================================================

namespace PL {

//==============================================================================

/// @brief USB device CDC endpoint class
class UsbDeviceCdc : public HardwareInterface, public Stream {
public:
  /// @brief Default hardware interface name
  static const std::string defaultName;
  /// @brief Default operation timeout in FreeRTOS ticks
  static const TickType_t defaultReadTimeout = 300 / portTICK_PERIOD_MS;

  /// @brief Creates a USB device CDC endpoint
  /// @param usbDevice USB device
  /// @param port CDC port
  /// @param rxBufferSize RX buffer size
  UsbDeviceCdc(std::shared_ptr<UsbDevice> usbDevice, tinyusb_cdcacm_itf_t port);
  ~UsbDeviceCdc();
  UsbDeviceCdc(const UsbDeviceCdc&) = delete;
  UsbDeviceCdc& operator=(const UsbDeviceCdc&) = delete;
  
  esp_err_t Lock(TickType_t timeout = portMAX_DELAY) override;
  esp_err_t Unlock() override;

  esp_err_t Initialize() override;
  esp_err_t Enable() override;
  esp_err_t Disable() override;

  esp_err_t Read(void* dest, size_t size) override;
  esp_err_t Write(const void* src, size_t size) override;

  bool IsEnabled() override;
  
  size_t GetReadableSize() override;

  TickType_t GetReadTimeout() override;
  esp_err_t SetReadTimeout(TickType_t timeout) override;

private:
  Mutex mutex;
  bool enabled = false;
  std::shared_ptr<UsbDevice> usbDevice;
  tinyusb_cdcacm_itf_t port;
  TickType_t readTimeout = defaultReadTimeout;
};

//==============================================================================

}

#endif