
Some testing of USB host capabilities - virtual com port, mass storage etc

Docker container: TODO

Build project:
```
idf.py set-target esp32s2
idf.py build
```


On host (after holding RST+0, let go RST, let go 0) - this will flash everything to the ESP32S2. The command is listed after every call to `idf.py build` and may possibly change depending on how it's built:

```
esptool.py -b 460800 --before default_reset --after hard_reset --chip esp32s2  write_flash --flash_mode dio --flash_size 2MB --flash_freq 80m 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x10000 build/cdc_acm_vcp.bin
```


Note: menuconfig

Component Config -> ESP System Settings -> Channel for Console Output: Custom UART
UART1
TX GPIO16
RX GPIO18
