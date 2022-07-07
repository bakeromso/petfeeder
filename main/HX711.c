/**
 *
 * HX711 library for ESP
 * https://github.com/bakeromso/HX711
 *
 * MIT License
 * (c) 2018 Bogdan Necula
 *
**/
#include <stdint.h>
#include "driver/gpio.h"
#include "rom/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "HX711.h"

gpio_num_t PD_SCK;	// Power Down and Serial Clock Input Pin
gpio_num_t DOUT;		// Serial Data Output Pin
uint8_t GAIN;		// amplification factor
long OFFSET = 0;	// used for tare weight
double SCALE = -351.8;	// used to return weight in grams, kg, ounces, whatever
static const char *TAG = "HX711";

// Make shiftIn() be aware of clockspeed for
// faster CPUs like ESP32, Teensy 3.x and friends.
// See also:
// - https://github.com/bogde/HX711/issues/75
// - https://github.com/arduino/Arduino/issues/6561
// - https://community.hiveeyes.org/t/using-bogdans-canonical-hx711-library-on-the-esp32/539
uint8_t shiftInSlow(gpio_num_t dataPin, gpio_num_t clockPin) {
    uint8_t value = 0;
    uint8_t i;

    for(i = 0; i < 8; ++i) {
        gpio_set_level(clockPin, 1);
        ets_delay_us(1);
        value |= gpio_get_level(dataPin) << (7 - i);
        gpio_set_level(clockPin, 0);
        ets_delay_us(1);
    }
    return value;
}

void begin(gpio_num_t dout, gpio_num_t pd_sck, uint8_t gain) {
    PD_SCK = pd_sck;
    DOUT = dout;

    // configure gpio output
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask = (1ULL<<PD_SCK);
    //disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    //disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL<<DOUT);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    set_gain(gain);
}

bool is_ready() {
    return gpio_get_level(DOUT) == 0;
}

void set_gain(uint8_t gain) {
    switch (gain) {
        case 128:		// channel A, gain factor 128
            GAIN = 1;
            break;
        case 64:		// channel A, gain factor 64
            GAIN = 3;
            break;
        case 32:		// channel B, gain factor 32
            GAIN = 2;
            break;
    }

}

long get_reading() {

    ESP_LOGD(TAG, "Waiting to become ready");
    // Wait for the chip to become ready.
    wait_ready();
    ESP_LOGD(TAG, "Chip is ready");


    // Define structures for reading data into.
    unsigned long value = 0;
    uint8_t data[3] = { 0 };
    uint8_t filler = 0x00;

    // Protect the read sequence from system interrupts.  If an interrupt occurs during
    // the time the PD_SCK signal is high it will stretch the length of the clock pulse.
    // If the total pulse time exceeds 60 uSec this will cause the HX711 to enter
    // power down mode during the middle of the read sequence.  While the device will
    // wake up when PD_SCK goes low again, the reset starts a new conversion cycle which
    // forces DOUT high until that cycle is completed.
    //
    // The result is that all subsequent bits read by shiftIn() will read back as 1,
    // corrupting the value returned by get_reading().  The ATOMIC_BLOCK macro disables
    // interrupts during the sequence and then restores the interrupt mask to its previous
    // state after the sequence completes, insuring that the entire read-and-gain-set
    // sequence is not interrupted.  The macro has a few minor advantages over bracketing
    // the sequence between `noInterrupts()` and `interrupts()` calls.


    // Begin of critical section.
    // Critical sections are used as a valid protection method
    // against simultaneous access in vanilla FreeRTOS.
    // Disable the scheduler and call portDISABLE_INTERRUPTS. This prevents
    // context switches and servicing of ISRs during a critical section.
    taskENTER_CRITICAL();


    // Pulse the clock pin 24 times to read the data.
    data[2] = shiftInSlow(DOUT, PD_SCK);
    data[1] = shiftInSlow(DOUT, PD_SCK);
    data[0] = shiftInSlow(DOUT, PD_SCK);

    // Set the channel and the gain factor for the next reading using the clock pin.
    for (unsigned int i = 0; i < GAIN; i++) {
        gpio_set_level(PD_SCK, 1);
        ets_delay_us(1);
        gpio_set_level(PD_SCK, 0);
        ets_delay_us(1);
    }

    // End of critical section.
    taskEXIT_CRITICAL();

    // Replicate the most significant bit to pad out a 32-bit signed integer
    if (data[2] & 0x80) {
        filler = 0xFF;
    } else {
        filler = 0x00;
    }

    // Construct a 32-bit signed integer
    value = ( (unsigned long) (filler << 24)
              | (unsigned long)(data[2] << 16)
              | (unsigned long)(data[1] << 8)
              |(unsigned long)(data[0] ) );

    return (long)(value);
}

void wait_ready() {
    // Wait for the chip to become ready.
    // This is a blocking implementation and will
    // halt the sketch until a load cell is connected.
    while (!is_ready()) {
        // Probably will do no harm on AVR but will feed the Watchdog Timer (WDT) on ESP.
        // https://github.com/bogde/HX711/issues/73
//        vTaskDelay(10 / portTICK_RATE_MS); // wait 10 ms
        vTaskDelay(1);
    }
}

bool wait_ready_retry(int retries, unsigned long delay_ms) {
    // Wait for the chip to become ready by
    // retrying for a specified amount of attempts.
    // https://github.com/bogde/HX711/issues/76
    int count = 0;
    while (count < retries) {
        if (is_ready()) {
            return true;
        }
        vTaskDelay(delay_ms / portTICK_RATE_MS);
        count++;
    }
    return false;
}

/// Not yet implemented
//bool wait_ready_timeout(unsigned long timeout, unsigned long delay_ms) {
//	// Wait for the chip to become ready until timeout.
//	// https://github.com/bogde/HX711/pull/96
//	unsigned long millisStarted = millis();
//	while (millis() - millisStarted < timeout) {
//		if (is_ready()) {
//			return true;
//		}
//        vTaskDelay(delay_ms / portTICK_RATE_MS);
//	}
//	return false;
//}

long read_average(uint16_t times) {
    long sum = 0;
    for (uint16_t i = 0; i < times; i++) {
        sum += get_reading();
        // Probably will do no harm on AVR but will feed the Watchdog Timer (WDT) on ESP.
        // https://github.com/bogde/HX711/issues/73
        vTaskDelay(1);
    }
    return sum / times;
}

double get_value(uint16_t times) {
    return (double) (read_average(times) - OFFSET);
}

float get_units(uint16_t times) {
    return (float) (get_value(times) / SCALE);
}

void tare(uint16_t times) {
    long average = read_average(times);
    set_offset(average);
}

void set_scale(double scale) {
    SCALE = scale;
}

double get_scale() {
    return SCALE;
}

void set_offset(long offset) {
    OFFSET = offset;
}

long get_offset() {
    return OFFSET;
}

void power_down() {
    gpio_set_level(PD_SCK, 0);
    gpio_set_level(PD_SCK, 1);
}

void power_up() {
    gpio_set_level(PD_SCK, 0);
}
