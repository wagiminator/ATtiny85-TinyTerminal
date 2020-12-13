# TinyTerminal - USB controlled OLED based on ATtiny85
TinyTerminal is a simple USB controlled 128x64 pixels I2C OLED display. It uses [V-USB](https://www.obdev.at/products/vusb/index.html) to build a [USB communication device class (CDC)](https://en.wikipedia.org/wiki/USB_communications_device_class) for serial communication via USB with the ATtiny. Text messages of all kinds can be sent via the USB interface and shown on the OLED display. The integrated buzzer gives an acoustic signal for every message received.

![pic1.jpg](https://github.com/wagiminator/ATtiny85-TinyTerminal/blob/main/documentation/TinyTerminal_pic1.jpg)
![pic2.jpg](https://github.com/wagiminator/ATtiny85-TinyTerminal/blob/main/documentation/TinyTerminal_pic2.jpg)

# Hardware
The schematic is shown below:

![schematic.png](https://github.com/wagiminator/ATtiny85-TinyTerminal/blob/main/documentation/TinyTerminal_wiring.png)

# Software
The communication via USB is handled by the V-USB software-only implementation of a low-speed USB device. To simplify the software development with the Arduino IDE the [VUSB_AVR board package](https://github.com/wagiminator/VUSB-AVR) is used. It includes the VUSB_CDC library, which makes it easy to implement a serial communication via USB. The I²C OLED routine is based on [tinyOLEDdemo](https://github.com/wagiminator/ATtiny13-TinyOLEDdemo). The rest was adapted from the [Tiny Terminal by David Johnson-Davies](http://www.technoblogy.com/show?TV4).

# Compiling, Uploading and Testing
- Open Arduino IDE.
- [Install VUSB-AVR](https://github.com/wagiminator/VUSB-AVR#Installation).
- Go to **Tools -> Board -> VUSB AVR** and select **VUSB-AVR**.
- Go to **Tools -> CPU** and select **ATtiny85 (16.5 MHz internal)**.
- Connect your programmer to your PC and to the ICSP header of the device.
- Go to **Tools -> Programmer** and select your ISP programmer.
- Go to **Tools -> Burn Bootloader** to burn the fuses.
- Open the TinyTerminal sketch and click **Upload**.
- The buzzer will make some noise during the upload as it is connected to one of the ICSP pins.
- Disconnect the programmer and connect the device via USB to your PC.

Alternatively, the precompiled hex file can be uploaded:

```
avrdude -c usbasp -p t85 -U lfuse:w:0xe1:m -U hfuse:w:0xdd:m -U efuse:w:0xff:m -U flash:w:TinyTerminal_t85_v1.0.hex
```

The device will work under Linux out-of-the-box. Windows users need to install the Digistump drivers:

```
https://raw.githubusercontent.com/digistump/DigistumpArduino/master/tools/micronucleus-2.0a4-win.zip
```

To test the device you can open the Serial Monitor of the Arduino IDE and send a message. With Linux the port is usually /dev/ttyACM0. You can also send a message via a Terminal:

```
echo "Hello World!\n" > /dev/ttyACM0
```

**The device was only tested with Linux!**
