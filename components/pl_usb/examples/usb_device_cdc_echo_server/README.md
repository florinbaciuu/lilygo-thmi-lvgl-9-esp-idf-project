# USB Device CDC Echo Server Example

1. UsbDeviceCdcEchoServer class inherits StreamServer class and overrides HandleRequest method to implement echo functionality.
1. USB device and USB device CDC endpoint are initialized.
2. USB device CDC endpoint is enabled.
5. UsbDeviceCdcEchoServer is created with USB device CDC endpoint as a stream.
6. UsbDeviceCdcEchoServer is enabled.
