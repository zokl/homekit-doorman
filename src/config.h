#pragma once

// The GPIO pin that is connected to a relay
#define GPIO_RELAY 5 // WeMos D1 Mini - Pin D1

// The GPIO pin that is connected to a LED
#define GPIO_LED 2 //WeMos D1 Mini - Pin Internal LED (Blue)

// The GPIO pin that is connected to a button
// const int button_gpio = 0;
#define GPIO_BUTTON 15 // WeMos D1 Mini - Pin D8

// The GPIO pin that is connected to a door bell
// const int button_gpio = 4;
#define GPIO_BELL 4 // WeMos D1 Mini - Pin D2

// Timeout in seconds to open lock for
#define UNLOCK_PERIOD 5 // 5 seconds

// Which signal to send to relay to open the lock (0 or 1)
#define RELAY_OPEN_SIGNAL 1

// Button timeout for log press in ms
#define BUTTON_LONG_PRESS_TIMEOUT 10000

// Button signal debounced time in ms
#define BUTTON_DEBOUNCED_TIME 50

// Bell signal debounced time in ms
#define BELL_DEBOUNCED_TIME 500

#define HOMEKIT_PASSWORD "111-11-111"
#define HOMEKIT_SETUP_ID "1QJ9"

