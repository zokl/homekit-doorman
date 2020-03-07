# Doorman HomeKit System
System of a doorbell and smart lock integration into Apple HomeKit.


## Compilation and deployment

```
esptool.py write_flash -fs 1MB -fm dout -ff 40m 0x0 rboot.bin 0x1000 blank_config.bin 0x2000 doorman.bin
```
All files are in ```bin``` directory.
