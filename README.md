# Doorman HomeKit System
System of a doorbell and smart lock integration into Apple HomeKit. The project platform is ESP8266 - WeMos D1 Mini.

The SW part of the project is based on  https://github.com/maximkulkin/esp-homekit-demo.

## Initial configuration

The initial configuration could be modified in the file [config.h](https://github.com/zokl/homekit-doorman/blob/master/src/config.h).

### HomeKit Access Parameters

* HOMEKIT_PASSWORD - "111-00-111"
* HOMEKIT_SETUP_ID - "1QJ9"

### System Parameters

* The GPIO pin that is connected to a relay - WeMos D1 Mini - Pin D1
* The GPIO pin that is connected to a LED - WeMos D1 Mini - Pin D4 (Internal Blue LED)
* The GPIO pin that is connected to a button - WeMos D1 Mini - Pin D3
  * Button for door open or factory reset
* The GPIO pin that is connected to a door bell - WeMos D1 Mini - Pin D2
  * Door bell detection - iOS notify as moving
* Timeout in seconds to open lock for - 5 seconds
* Button timeout for log press - 10000 ms (factory reset)
  * Shorter press = open the door
* Doorbell notify timeout - 3000 ms
  * After this timeout system disarm the door bell notification


## PCB Design

PCB is designed for using in DIN rail box [Z103J ABS V0](https://github.com/zokl/homekit-doorman/blob/master/doc/dsh.627-099.1.pdf) with AC/DC power supply [MEAN WELL DR-15-5](https://github.com/zokl/homekit-doorman/blob/master/doc/dsh.751-263.1.pdf).

![Doorman v 1.1 - schematics](/hw/kicad_1v1/pictures/Doorman_1v1_schematics.png)

![Doorman v 1.1 - PCB](/hw/kicad_1v1/pictures/Doorman_1v1_pcb.png)

![Doorman v 1.1 - PCB render #1](/hw/kicad_1v1/pictures/Doorman_1v1_full.png)

![Doorman v 1.1 - PCB render #1](/hw/kicad_1v1/pictures/Doorman_1v1_full_module.png)

## Compilation and deployment
All binary files are in ```bin``` directory.

### Flash process

```
esptool.py write_flash -fs 1MB -fm dout -ff 40m 0x0 rboot.bin 0x1000 blank_config.bin 0x2000 doorman.bin
```

* [rboot.bin](https://github.com/zokl/homekit-doorman/blob/master/bin/rboot.bin) - boot loader
* [blank_config.bin](https://github.com/zokl/homekit-doorman/blob/master/bin/blank_config.bin) - blank config partition
* [doorman.bin](https://github.com/zokl/homekit-doorman/blob/master/bin/doorman_20200307.bin) - firmware
