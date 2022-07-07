/**
 *
 * HX711 library for Arduino
 * https://github.com/bogde/HX711
 *
 * MIT License
 * (c) 2018 Bogdan Necula
 *
**/
#ifndef HX711_h
#define HX711_h

#include <stdbool.h>
#include "driver/gpio.h"
#include <stdint.h>

// Initialize library with data output pin, clock input pin and gain factor.
// Channel selection is made by passing the appropriate gain:
// - With a gain factor of 64 or 128, channel A is selected
// - With a gain factor of 32, channel B is selected
// The library default is "128" (Channel A).
void begin(gpio_num_t dout, gpio_num_t pd_sck, uint8_t gain);

// Check if HX711 is ready
// from the datasheet: When output data is not ready for retrieval, digital output pin DOUT is high. Serial clock
// input PD_SCK should be low. When DOUT goes to low, it indicates data is ready for retrieval.
bool is_ready();

// Wait for the HX711 to become ready
void wait_ready(); // default 0

bool wait_ready_retry(int retries, unsigned long delay_ms); // default retries = 3, delay_ms = 0
//		bool wait_ready_timeout(unsigned long timeout = 1000, unsigned long delay_ms = 0);

// set the gain factor; takes effect only after a call to read()
// channel A can be set for a 128 or 64 gain; channel B has a fixed 32 gain
// depending on the parameter, the channel is also set to either A or B
void set_gain(uint8_t gain); // default 128

// waits for the chip to be ready and returns a reading
long get_reading();

// returns an average reading; times = how many times to read
long read_average(uint16_t times); // default 10

// returns (read_average() - OFFSET), that is the current value without the tare weight; times = how many readings to do
double get_value(uint16_t times); // default 1

// returns get_value() divided by SCALE, that is the raw value divided by a value obtained via calibration
// times = how many readings to do
float get_units(uint16_t times); // default 1

// set the OFFSET value for tare weight; times = how many times to read the tare value
void tare(uint16_t times); // default 10

// set the SCALE value; this value is used to convert the raw data to "human readable" data (measure units)
void set_scale(double scale); //default scale = 1.f

// get the current SCALE
double get_scale();

// set OFFSET, the value that's subtracted from the actual reading (tare weight)
void set_offset(long offset); // default 0

// get the current OFFSET
long get_offset();

// puts the chip into power down mode
void power_down();

// wakes up the chip after power down mode
void power_up();

#endif /* HX711_h */