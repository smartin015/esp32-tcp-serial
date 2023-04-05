Control USB serial devices with an ESP32S2 (tested with the Lolin S2 Mini)

https://youtu.be/zkpkUem1ZGw

## Start build environment

Run the latest esp-idf docker container with repository mounted: 

```
docker run -it --rm -v $(pwd):/project -w /project espressif/idf:latest /bin/bash
```

## Configure wifi, console output etc.

```
idf.py menuconfig

Component Config -> ESP System Settings -> Channel for Console Output: Custom UART

UART1
TX GPIO16
RX GPIO18

Component Config -> Example Connection Configuration

Set SSID and password to your wifi
```

## Build the project 

```
idf.py set-target esp32s2
idf.py build
```

## Flash firmware

On host (after holding RST+0, let go RST, let go 0) - this will flash everything to the ESP32S2. The command is listed after every call to `idf.py build` and may possibly change depending on how it's built:
```
esptool.py -b 460800 --before default_reset --after hard_reset --chip esp32s2  write_flash --flash_mode dio --flash_size 2MB --flash_freq 80m 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x10000 build/cdc_acm_vcp.bin
```

## Use it

Connect your USB client device (e.g. a 3D printer) to the USB-C port on the Lolin S2 Mini. Ensure the mini also has power, as sometimes client devices don't power the USB bus.

Connect a UART serial reader (e.g. FTDI friend) with RX on pin 16 and TX on pin 18. `screen /dev/ttyUSB0 115200` or similar command to read the console output from the chip.

Find the IP address of the device and connect via e.g. `nc -vv 192.168.1.113 3333`. Type some text, then hit enter to send it to the client device. Client responses are printed on the UART console output.
