/*
 * Implemenetation of lock mechanism accessory for a magnet lock.
 * When unlocked, it changes relay state (unlocks) for configured period and then
 * changes it back.
 */

#include <stdio.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>
#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <FreeRTOS.h>
#include <task.h>
#include <etstimer.h>
#include <esplibs/libmain.h>

#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <wifi_config.h>

#include "config.h"
#include "button.h"
#include "contact_sensor.h"

uint16_t state_old = CONTACT_CLOSED;

void lock_lock();
void lock_unlock();

void relay_write(int value) {
    gpio_write(GPIO_RELAY, value ? 1 : 0);
}

void led_write(bool on) {
    gpio_write(GPIO_LED, on ? 0 : 1);
}

void show_config() {
    printf(">>> Initial config <<<\n");
    printf(">> Door Lock GPIO: %d\n", GPIO_RELAY);
    printf(">> Led GPIO: %d\n", GPIO_LED);
    printf(">> Unlock period: %d s\n", UNLOCK_PERIOD);
    printf(">> ------------------------------------------\n");
    printf(">> Button GPIO: %d\n", GPIO_BUTTON);
    printf(">> Button long press timeout %d ms\n", BUTTON_LONG_PRESS_TIMEOUT);
    printf(">> Button bounced time: %d ms\n", BUTTON_DEBOUNCED_TIME);
    printf(">> ------------------------------------------\n");
    printf(">> Doorbell GPIO: %d\n", GPIO_BELL);
    printf(">> Doorbell bounced interval: %d ms\n", DOORBELL_DEBOUNCED_TIME);
    printf(">> ------------------------------------------\n");
    printf(">> HomeKit setup id: %s\n", HOMEKIT_SETUP_ID);
    printf(">> HomeKit password: %s\n", HOMEKIT_PASSWORD);
    printf(">> ------------------------------------------\n\n");
}

void reset_configuration_task() {
    //Flash the LED first before we start the reset
    for (int i=0; i<3; i++) {
        led_write(true);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        led_write(false);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    printf("> Resetting Wifi Config\n");

    wifi_config_reset();

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    printf("> Resetting HomeKit Config\n");

    homekit_server_reset();

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    printf("> Restarting\n");

    sdk_system_restart();

    vTaskDelete(NULL);
}

void reset_configuration() {
    printf("> Resetting configuration\n");
    xTaskCreate(reset_configuration_task, "Reset configuration", 256, NULL, 2, NULL);
}

void gpio_init() {
    gpio_enable(GPIO_LED, GPIO_OUTPUT);
    led_write(false);

    gpio_enable(GPIO_RELAY, GPIO_OUTPUT);
    relay_write(!RELAY_OPEN_SIGNAL);
}

void button_callback(uint8_t gpio, button_event_t event) {
    switch (event) {
        case button_event_single_press:
            printf(">> Toggling relay <<\n");
            lock_unlock();
            break;
        case button_event_long_press:
            printf(">> Reset configuration !!!\n");
            reset_configuration();
            break;
        default:
            printf(">> Unknown button event: %d\n", event);
    }
}

void lock_identify_task(void *_args) {
    // We identify the device by Flashing it's LED.
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            led_write(true);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            led_write(false);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        vTaskDelay(250 / portTICK_PERIOD_MS);
    }

    led_write(false);

    vTaskDelete(NULL);
}

void lock_identify(homekit_value_t _value) {
    printf("> Lock identify\n");
    xTaskCreate(lock_identify_task, "Lock identify", 128, NULL, 2, NULL);
}

void doorbell_identify(homekit_value_t _value) {
    printf("> Bell identify\n");
    xTaskCreate(lock_identify_task, "Lock identify", 128, NULL, 2, NULL);
}

typedef enum {
    lock_state_unsecured = 0,
    lock_state_secured = 1,
    lock_state_jammed = 2,
    lock_state_unknown = 3,
} lock_state_t;

homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, "Doorman");

homekit_characteristic_t lock_current_state = HOMEKIT_CHARACTERISTIC_(
    LOCK_CURRENT_STATE,
    lock_state_unknown,
);

void lock_target_state_setter(homekit_value_t value);

homekit_characteristic_t lock_target_state = HOMEKIT_CHARACTERISTIC_(
    LOCK_TARGET_STATE,
    lock_state_secured,
    .setter=lock_target_state_setter,
);

void lock_target_state_setter(homekit_value_t value) {
    lock_target_state.value = value;

    if (value.int_value == 0) {
        lock_unlock();
    } else {
        lock_lock();
    }
}

void lock_control_point(homekit_value_t value) {
    // Nothing to do here
}

/**
 * Returns the bell state as a homekit value.
 **/
homekit_value_t doorbell_state_getter() {
    // printf(">> Doorbell state was requested (%s).\n", contact_sensor_state_get(GPIO_BELL) == CONTACT_OPEN ? "ringing" : "silence");
    // return HOMEKIT_UINT8(contact_sensor_state_get(GPIO_BELL) == CONTACT_OPEN ? 1 : 0);
    return HOMEKIT_BOOL(contact_sensor_state_get(GPIO_BELL) == CONTACT_OPEN ? 1 : 0);
}

/**
 * The sensor characteristic as global variable.
 **/
// homekit_characteristic_t doorbell_push_characteristic = HOMEKIT_CHARACTERISTIC_(OCCUPANCY_DETECTED, 0);
homekit_characteristic_t doorbell_push_characteristic = HOMEKIT_CHARACTERISTIC_(MOTION_DETECTED, 0);

/**
 * Called (indirectly) from the interrupt handler to notify the client of a state change.
 **/
// void contact_sensor_callback(uint8_t gpio, contact_sensor_state_t state) {
//     switch (state) {
//         case CONTACT_OPEN:
//         case CONTACT_CLOSED:
//             led_write(true);
//             doorbell_push_characteristic.value = doorbell_state_getter();
//             printf(">> Pushing bell state '%s'.\n", state == CONTACT_OPEN ? "ringing" : "silence");
//             homekit_characteristic_notify(&doorbell_push_characteristic, doorbell_push_characteristic.value);
//             led_write(false);
//             break;
//         default:
//             printf(">> Unknown bell event: %d\n", state);
//     }
// }

ETSTimer bell_timer;

void contact_sensor_callback(uint8_t gpio, contact_sensor_state_t state) {
    switch (state) {
        case CONTACT_OPEN:
        case CONTACT_CLOSED:
            led_write(true);
            doorbell_push_characteristic.value = doorbell_state_getter();
            // If doorbell on, start timeout for DOORBELL_OFF_DELAY, after expiration set force homekit notify to off.
            if (state == CONTACT_OPEN && state_old == CONTACT_CLOSED) {
                printf("Doorbell ringing !!!\n");
                homekit_characteristic_notify(&doorbell_push_characteristic, doorbell_push_characteristic.value);
                printf("Doorbell off delay timer for %d ms is starting ... \n", DOORBELL_OFF_DELAY);
                sdk_os_timer_arm(&bell_timer, DOORBELL_OFF_DELAY, false);
                state_old  = CONTACT_OPEN;
            }
            led_write(false);
            break;
        default:
            printf(">> Unknown bell event: %d\n", state);
    }
}

ETSTimer lock_timer;

void lock_lock() {
    sdk_os_timer_disarm(&lock_timer);

    relay_write(!RELAY_OPEN_SIGNAL);
    led_write(false);

    if (lock_current_state.value.int_value != lock_state_secured) {
        lock_current_state.value = HOMEKIT_UINT8(lock_state_secured);
        homekit_characteristic_notify(&lock_current_state, lock_current_state.value);
    }
}

void lock_timeout() {
    if (lock_target_state.value.int_value != lock_state_secured) {
        lock_target_state.value = HOMEKIT_UINT8(lock_state_secured);
        homekit_characteristic_notify(&lock_target_state, lock_target_state.value);
    }

    lock_lock();
}

// Doorbell timeout callback
void doorbell_timeout() {
    printf("Forcing doorbell homekit notification off \n"); 
    sdk_os_timer_disarm(&bell_timer);
    homekit_characteristic_notify(&doorbell_push_characteristic, HOMEKIT_BOOL(CONTACT_CLOSED));
    state_old = CONTACT_CLOSED;
}

void lock_init() {
    lock_current_state.value = HOMEKIT_UINT8(lock_state_secured);
    homekit_characteristic_notify(&lock_current_state, lock_current_state.value);

    sdk_os_timer_disarm(&lock_timer);
    sdk_os_timer_setfn(&lock_timer, lock_timeout, NULL);

}

void  doorbell_init () {
    homekit_characteristic_notify(&doorbell_push_characteristic, doorbell_state_getter());

    sdk_os_timer_disarm(&bell_timer);
    sdk_os_timer_setfn(&bell_timer, doorbell_timeout, NULL);
}

void lock_unlock() {
    relay_write(RELAY_OPEN_SIGNAL);
    led_write(true);

    lock_current_state.value = HOMEKIT_UINT8(lock_state_unsecured);
    homekit_characteristic_notify(&lock_current_state, lock_current_state.value);

    if (UNLOCK_PERIOD) {
        sdk_os_timer_disarm(&lock_timer);
        sdk_os_timer_arm(&lock_timer, UNLOCK_PERIOD * 1000, 0);
    }
}

homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_door_lock, .services=(homekit_service_t*[]){
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]){
            &name,
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "zokl@2020"),
            // HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "123456"),
            HOMEKIT_CHARACTERISTIC(MODEL, "Door Lock"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, lock_identify),
            NULL
        }),
        HOMEKIT_SERVICE(LOCK_MECHANISM, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Lock"),
            &lock_current_state,
            &lock_target_state,
            NULL
        }),
        HOMEKIT_SERVICE(LOCK_MANAGEMENT, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(LOCK_CONTROL_POINT, 
                .setter=lock_control_point
            ),
            HOMEKIT_CHARACTERISTIC(VERSION, "1"),
            NULL
        }),
        NULL
    }),

    HOMEKIT_ACCESSORY(.id=2, .category=homekit_accessory_category_switch, .services=(homekit_service_t*[]){
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]){
            // HOMEKIT_CHARACTERISTIC(NAME, "Home Bell"),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "zokl@2020"),
            // HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "0012346"),
            HOMEKIT_CHARACTERISTIC(MODEL, "Doorbell"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, doorbell_identify),
            NULL
        }),
        HOMEKIT_SERVICE(MOTION_SENSOR, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Doorbell"),
            &doorbell_push_characteristic,
            NULL
        }),
        NULL
    }),

    NULL
};

homekit_server_config_t config = {
    .accessories = accessories,
    .password = HOMEKIT_PASSWORD,
    .setupId = HOMEKIT_SETUP_ID,
};

void on_wifi_ready() {
    homekit_server_init(&config);
}

void create_accessory_name() {
    uint8_t macaddr[6];
    sdk_wifi_get_macaddr(STATION_IF, macaddr);

    int name_len = snprintf(NULL, 0, "Doorman-%02X%02X%02X",
                            macaddr[3], macaddr[4], macaddr[5]);
    char *name_value = malloc(name_len+1);
    snprintf(name_value, name_len+1, "Doorman-%02X%02X%02X",
             macaddr[3], macaddr[4], macaddr[5]);

    name.value = HOMEKIT_STRING(name_value);
}

void user_init(void) {
    uart_set_baud(0, 9600);
    show_config();
    create_accessory_name();

    wifi_config_init("doorman", NULL, on_wifi_ready);
    gpio_init();
    lock_init();
    doorbell_init();

    if (button_create(GPIO_BUTTON, BUTTON_PRESSED_EXPECTED_VALUE, BUTTON_LONG_PRESS_TIMEOUT, button_callback)) {
        printf("> Failed to initialize button\n");
    }

    if (contact_sensor_create(GPIO_BELL, contact_sensor_callback)) {
        printf("> Failed to initialize bell\n");
    }
}
