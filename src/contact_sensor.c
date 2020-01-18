#include <string.h>
#include <etstimer.h>
#include <esplibs/libmain.h>
#include "contact_sensor.h"
#include "config.h"

typedef struct _contact_sensor {
    uint8_t gpio_num;
    contact_sensor_callback_fn callback;
    uint16_t debounce_time;
    uint32_t last_event_time;

    struct _contact_sensor *next;
} contact_sensor_t;


contact_sensor_t *sensors = NULL;


static contact_sensor_t *contact_sensor_find_by_gpio(const uint8_t gpio_num) {
    contact_sensor_t *sensor = sensors;
    while (sensor && sensor->gpio_num != gpio_num)
        sensor = sensor->next;

    return sensor;
}

contact_sensor_state_t contact_sensor_state_get(uint8_t gpio_num) {
    return gpio_read(gpio_num);
}


void contact_sensor_intr_callback(uint8_t gpio) {
    contact_sensor_t *sensor = contact_sensor_find_by_gpio(gpio);
    if (!sensor)
        return;

    uint32_t now = xTaskGetTickCountFromISR();
    if ((now - sensor->last_event_time) * portTICK_PERIOD_MS < sensor->debounce_time) {
        // debounce time, ignore events
        return;
    }
    sensor->last_event_time = now;

    sensor->callback(sensor->gpio_num, contact_sensor_state_get(sensor->gpio_num));    
}

int contact_sensor_create(const uint8_t gpio_num, contact_sensor_callback_fn callback) {
    contact_sensor_t *sensor = contact_sensor_find_by_gpio(gpio_num);
    if (sensor)
        return -1;

    sensor = malloc(sizeof(contact_sensor_t));
    memset(sensor, 0, sizeof(*sensor));
    sensor->gpio_num = gpio_num;
    sensor->callback = callback;

    sensor->next = sensors;
    sensors = sensor;

    uint32_t now = xTaskGetTickCountFromISR();
    sensor->last_event_time = now;

    // times in milliseconds
    sensor->debounce_time = DOORBELL_DEBOUNCED_TIME;

    //gpio_set_pullup(sensor->gpio_num, true, true);
    gpio_set_interrupt(sensor->gpio_num, GPIO_INTTYPE_EDGE_ANY, contact_sensor_intr_callback);

    return 0;
}


void contact_sensor_delete(const uint8_t gpio_num) {
    if (!sensors)
        return;

    contact_sensor_t *sensor = NULL;
    if (sensors->gpio_num == gpio_num) {
        // Skip first element:
        sensor = sensors;
        sensors = sensors->next;
    } else {
        contact_sensor_t *b = sensors;
        while (b->next) {
            if (b->next->gpio_num == gpio_num) {
                // Unlink middle element:
                sensor = b->next;
                b->next = b->next->next;
                break;
            }
        }
    }

    if (sensor) {
        gpio_set_interrupt(sensor->gpio_num, GPIO_INTTYPE_EDGE_ANY, NULL);
    }
}