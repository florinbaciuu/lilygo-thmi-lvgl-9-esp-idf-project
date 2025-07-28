#include "pl_usb_device.h"
#include "esp_check.h"
#include "esp_mac.h"

#if defined(CONFIG_IDF_TARGET_ESP32P4) || defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3)

//==============================================================================

static const char* TAG = "pl_usb_device";

char macString[13] = ""; 

const char *string_descriptor[] = {
  (char[]){0x09, 0x04},                    // 0: is supported language is English (0x0409)
  CONFIG_TINYUSB_DESC_MANUFACTURER_STRING, // 1: Manufacturer
  CONFIG_TINYUSB_DESC_PRODUCT_STRING,      // 2: Product
  CONFIG_TINYUSB_DESC_SERIAL_STRING,       // 3: Serials, should use chip ID

#if CONFIG_TINYUSB_CDC_ENABLED
  CONFIG_TINYUSB_DESC_CDC_STRING,          // 4: CDC Interface
#endif

#if CONFIG_TINYUSB_MSC_ENABLED
  CONFIG_TINYUSB_DESC_MSC_STRING,          // 5: MSC Interface
#endif

#if CONFIG_TINYUSB_NET_MODE_ECM_RNDIS || CONFIG_TINYUSB_NET_MODE_NCM
  "USB net",                               // 6. NET Interface
  "",                                      // 7. MAC
#endif

#if CFG_TUD_VENDOR
  "Vendor specific",                       // 8. Vendor specific
#endif
  NULL                                     // NULL: Must be last. Indicates end of array
};

//==============================================================================

namespace PL {

//==============================================================================

UsbDevice::UsbDevice(tinyusb_usbdev_t device, int vBusMonitorPin) : device(device), vBusMonitorPin(vBusMonitorPin) { }

//==============================================================================

UsbDevice::~UsbDevice() {
  if (initialized)
    tinyusb_driver_uninstall();
}

//==============================================================================

esp_err_t UsbDevice::Lock(TickType_t timeout) {
  esp_err_t error = mutex.Lock(timeout);
  if (error == ESP_OK)
    return ESP_OK;
  if (error == ESP_ERR_TIMEOUT && timeout == 0)
    return ESP_ERR_TIMEOUT;
  ESP_RETURN_ON_ERROR(error, TAG, "mutex lock failed");
  return ESP_OK;
}

//==============================================================================

esp_err_t UsbDevice::Unlock() {
  ESP_RETURN_ON_ERROR(mutex.Unlock(), TAG, "mutex unlock failed");
  return ESP_OK;
}

//==============================================================================

esp_err_t UsbDevice::Initialize() {
  LockGuard lg(*this);
  if (initialized)
    return ESP_OK;

  // Set serial number to MAC address if not specified in menuconfig
  if (string_descriptor[3][0] == 0) {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    sprintf(macString, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    string_descriptor[3] = macString;
  }
      
  tinyusb_config_t config = {};
  config.string_descriptor = string_descriptor;
  config.self_powered = vBusMonitorPin >= 0;
  config.vbus_monitor_io = vBusMonitorPin;
  ESP_RETURN_ON_ERROR(tinyusb_driver_install(&config), TAG, "driver install failed");
  initialized = true;
  return ESP_OK;
}

//==============================================================================

}

#endif