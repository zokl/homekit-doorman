#pragma once

// The GPIO pin that is connected to a relay
#define GPIO_RELAY 5 // WeMos D1 Mini - Pin D1

// The GPIO pin that is connected to a LED
#define GPIO_LED 2 //WeMos D1 Mini - Pin D4 (Internal Blue LED)

// The GPIO pin that is connected to a button
// const int button_gpio = 0;
#define GPIO_BUTTON 0 // WeMos D1 Mini - Pin D3

// The GPIO pin that is connected to a door bell
// const int button_gpio = 4;
#define GPIO_BELL 4 // WeMos D1 Mini - Pin D2

// Timeout in seconds to open lock for
#define UNLOCK_PERIOD 5 // 5 seconds

// Which signal to send to relay to open the lock (0 or 1)
#define RELAY_OPEN_SIGNAL 1

// The expected value when the button is pressed. 
// For buttons connected to ground this is 0/false, 
// for other buttons this might be 1/true.
#define BUTTON_PRESSED_EXPECTED_VALUE 0

// Button timeout for log press in ms
#define BUTTON_LONG_PRESS_TIMEOUT 10000

// Button signal debounced time in ms
#define BUTTON_DEBOUNCED_TIME 50

// Doorbell signal debounced time in ms
#define DOORBELL_DEBOUNCED_TIME 20

// Doorbell notify timeout in ms
#define DOORBELL_OFF_DELAY 3000

#define HOMEKIT_PASSWORD "250-68-301"
#define HOMEKIT_SETUP_ID "1QJ9"

