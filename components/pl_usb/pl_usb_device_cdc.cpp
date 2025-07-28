#include "pl_usb_device_cdc.h"
#include "esp_check.h"

#if CONFIG_TINYUSB_CDC_ENABLED

//==============================================================================

static const char* TAG = "pl_usb_device_cdc";

//==============================================================================

namespace PL {

//==============================================================================

const std::string UsbDeviceCdc::defaultName = "USB CDC";

//==============================================================================

UsbDeviceCdc::UsbDeviceCdc(std::shared_ptr<UsbDevice> usbDevice, tinyusb_cdcacm_itf_t port) :
    usbDevice(usbDevice), port(port) {
  SetName(defaultName);
}

//==============================================================================

UsbDeviceCdc::~UsbDeviceCdc() {
  if (tusb_cdc_acm_initialized(port))
    tusb_cdc_acm_deinit(port);
}

//==============================================================================

esp_err_t UsbDeviceCdc::Lock(TickType_t timeout) {
  esp_err_t error = mutex.Lock(timeout);
  if (error == ESP_OK)
    return ESP_OK;
  if (error == ESP_ERR_TIMEOUT && timeout == 0)
    return ESP_ERR_TIMEOUT;
  ESP_RETURN_ON_ERROR(error, TAG, "mutex lock failed");
  return ESP_OK;
}

//==============================================================================

esp_err_t UsbDeviceCdc::Unlock() {
  ESP_RETURN_ON_ERROR(mutex.Unlock(), TAG, "mutex unlock failed");
  return ESP_OK;
}

//==============================================================================

esp_err_t UsbDeviceCdc::Initialize() {
  LockGuard lg(*this, *usbDevice);
  if (!usbDevice->initialized) {
    ESP_RETURN_ON_ERROR(usbDevice->Initialize(), TAG, "USB device initialize failed");
  }

  if (!tusb_cdc_acm_initialized(port)) {
    tinyusb_config_cdcacm_t config = {};
    config.usb_dev = usbDevice->device;
    config.cdc_port = port;
    ESP_RETURN_ON_ERROR(tusb_cdc_acm_init(&config), TAG, "USB CDC init failed");
  }
  return ESP_OK;
}

//==============================================================================

esp_err_t UsbDeviceCdc::Enable() {
  LockGuard lg(*this);
  ESP_RETURN_ON_FALSE(tusb_cdc_acm_initialized(port), ESP_ERR_INVALID_STATE, TAG, "USB CDC is not initialized");
  if (enabled)
    return ESP_OK;
  enabled = true; 
  Read(NULL, GetReadableSize());
  enabledEvent.Generate();
  return ESP_OK;
}

//==============================================================================

esp_err_t UsbDeviceCdc::Disable() {
  LockGuard lg(*this);
  ESP_RETURN_ON_FALSE(tusb_cdc_acm_initialized(port), ESP_ERR_INVALID_STATE, TAG, "USB CDC is not initialized");
  if (!enabled)
    return ESP_OK;
  enabled = false;
  disabledEvent.Generate();
  return ESP_OK;
}

//==============================================================================

esp_err_t UsbDeviceCdc::Read(void* dest, size_t size) {
  LockGuard lg(*this);
  ESP_RETURN_ON_FALSE(enabled, ESP_ERR_INVALID_STATE, TAG, "USB CDC is not enabled");
  if (!size)
    return ESP_OK;
  
  TimeOut_t xTimeOut;
  vTaskSetTimeOutState(&xTimeOut);

  size_t rxSize;
  do {
    if (dest) {
      ESP_RETURN_ON_ERROR(tinyusb_cdcacm_read(port, (uint8_t*)dest, size, &rxSize), TAG, "USB CDC read failed");
      size -= rxSize;
      dest = (uint8_t*)dest + rxSize;
    }
    else {
      uint8_t data;
      do {
        ESP_RETURN_ON_ERROR(tinyusb_cdcacm_read(port, &data, 1, &rxSize), TAG, "USB CDC read failed");
        size -= rxSize;
      } while(size && rxSize);
    }

    if (size == 0)
      return ESP_OK;

  } while(xTaskCheckForTimeOut(&xTimeOut, &readTimeout) == pdFALSE);

  ESP_RETURN_ON_ERROR(ESP_ERR_TIMEOUT, TAG, "timeout");
  return ESP_OK;
}

//==============================================================================

esp_err_t UsbDeviceCdc::Write(const void* src, size_t size) {
  LockGuard lg(*this);
  ESP_RETURN_ON_FALSE(enabled, ESP_ERR_INVALID_STATE, TAG, "USB CDC is not enabled");
  if (!size)
    return ESP_OK;
  ESP_RETURN_ON_FALSE(src, ESP_ERR_INVALID_ARG, TAG, "src is null");

  uint8_t bufferFullEvents = 0, maxBufferFullEvents = 5;
  while (size) {
    size_t txSize = tinyusb_cdcacm_write_queue(port, (uint8_t*)src, size);
    tinyusb_cdcacm_write_flush(port, 0);
    size -= txSize;
    src = (uint8_t*)src + txSize;

    if (txSize == 0) {
      bufferFullEvents++;
      if (bufferFullEvents >= maxBufferFullEvents) {
        ESP_RETURN_ON_ERROR(ESP_FAIL, TAG, "USB CDC write failed");
      }
      vTaskDelay(1);
    }
    else
      bufferFullEvents = 0;
  }

  return ESP_OK;
}

//==============================================================================

bool UsbDeviceCdc::IsEnabled() {
  LockGuard lg(*this);
  return enabled;
}

//==============================================================================

size_t UsbDeviceCdc::GetReadableSize() {
  LockGuard lg(*this);
  if (!enabled)
    return 0;
  return tud_cdc_n_available(port);
}

//==============================================================================

TickType_t UsbDeviceCdc::GetReadTimeout() {
  LockGuard lg(*this);
  return readTimeout;
}

//==============================================================================

esp_err_t UsbDeviceCdc::SetReadTimeout(TickType_t timeout) {
  LockGuard lg(*this);
  readTimeout = timeout;
  return ESP_OK;
}

//==============================================================================

}

#endif