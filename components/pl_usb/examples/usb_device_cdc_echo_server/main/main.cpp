#include "pl_usb.h"

//==============================================================================

class UsbDeviceCdcEchoServer : public PL::StreamServer {
public:
  using PL::StreamServer::StreamServer;

protected:
  esp_err_t HandleRequest(PL::Stream& stream) override;
};

//==============================================================================

extern "C" void app_main(void) {
  auto usbDevice = std::make_shared<PL::UsbDevice>(TINYUSB_USBDEV_0);
  auto usbDeviceCdc = std::make_shared<PL::UsbDeviceCdc>(usbDevice, TINYUSB_CDC_ACM_0);
  
  usbDeviceCdc->Initialize();
  usbDeviceCdc->Enable();

  UsbDeviceCdcEchoServer server(usbDeviceCdc);
  server.Enable();

  while (1) {
    vTaskDelay(1);
  }
}

//==============================================================================

esp_err_t UsbDeviceCdcEchoServer::HandleRequest(PL::Stream& stream) {
  uint8_t dataByte;
  while (stream.GetReadableSize()) {
    if (stream.Read(&dataByte, sizeof(dataByte)) == ESP_OK) {
      stream.Write(&dataByte, sizeof(dataByte));
    }
  }
  return ESP_OK;
}