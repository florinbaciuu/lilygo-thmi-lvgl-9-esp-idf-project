#include "pl_usb.h"

//==============================================================================

extern "C" void app_main(void) {
  auto usbDevice = std::make_shared<PL::UsbDevice>(TINYUSB_USBDEV_0);
  PL::UsbDeviceCdc usbDeviceCdc(usbDevice, TINYUSB_CDC_ACM_0);
  
  usbDeviceCdc.Initialize();
  usbDeviceCdc.Enable();

  char testData[] = "Test\n";
  while(1) {
    usbDeviceCdc.Write(testData, sizeof(testData) - 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}