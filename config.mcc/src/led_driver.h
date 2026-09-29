/*
 * led_driver.h
 *
 * Created: 18-09-2017 11:45:02
 *  Author: I41681
 * Ported to XC8: 12-03-2024
 *  Editor: C13999
 */

#ifndef LED_DRIVER_H_
#define LED_DRIVER_H_

#define ENABLE_LED 1u

#if ENABLE_LED == 1u

#include "definitions.h"

#define I2C_CLIENT_ADDR_LED_DRIVER 0x20

// LED types
enum eLedTypes{LED_BUTTON, LED_SCROLLER};

void init_led_driver(void);
void led_gpio_update(uint8_t data, uint8_t type);
void led_reset(void);
void touch_update_leds(void);
uint8_t reverseBitsInByte(uint8_t b);
extern volatile uint8_t measurement_done_touch;

#endif

#endif /* LED_DRIVER_H_ */
