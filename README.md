# homekit-doorman
System of doorbell integration into HomeKit


## Compilation and deployment

```
esptool.py write_flash -fs 1MB -fm dout -ff 40m 0x0 rboot.bin 0x1000 blank_config.bin 0x2000 doorman.bin
```
