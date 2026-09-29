/*
 * led_driver.c
 *
 * Created: 18-09-2017 11:44:47
 *  Author: I41681
 * Ported to XC8: 12-03-2024
 *  Editor: C13999
 * Ported to Harmony SERCOM I2C PLIB: 09-29-2026
 */

// led driver uses the MCP23017 16b i2c port expander IC
// client address is 0x20
// chip uses IOCON.BANK = 0 address mapping by default
// leds are driven on anode side; 0 = off, 1 = on
// i2c communication via SERCOM0 I2C master peripheral

#include "led_driver.h"


#if ENABLE_LED == 1u

/*============================================================================
void init_led_driver(void)
------------------------------------------------------------------------------
Purpose: initializes MCP23017 I/O expander chip
Input  : none
Output : none
Notes  : chip uses IOCON.BANK = 0 address mapping by default
============================================================================*/
void init_led_driver(void) {
    uint8_t i2c_clientAddr = I2C_CLIENT_ADDR_LED_DRIVER; // 7b client address
    uint8_t i2c_writeBuffer[4];
    
    // IODIRA - I/O Direction A
    i2c_writeBuffer[0] = 0x00; // direction register index
    i2c_writeBuffer[1] = 0x00; // 0 = output, 1 = input
    SERCOM0_I2C_Write(i2c_clientAddr, i2c_writeBuffer, 2); // write to i2c via SERCOM0
    while (SERCOM0_I2C_IsBusy()); // wait for write operation to complete
    // OLATA - Output Latch A
    i2c_writeBuffer[0] = 0x14; // latch register index
    i2c_writeBuffer[1] = 0x00; // 0 = low, 1 = high
    SERCOM0_I2C_Write(i2c_clientAddr, i2c_writeBuffer, 2); // write to i2c via SERCOM0
    while (SERCOM0_I2C_IsBusy()); // wait for write operation to complete
    // OLATB - Output Direction B
    i2c_writeBuffer[0] = 0x01; // direction register index
    i2c_writeBuffer[1] = 0x00; // 0 = output, 1 = input
    SERCOM0_I2C_Write(i2c_clientAddr, i2c_writeBuffer, 2); // write to i2c via SERCOM0
    while (SERCOM0_I2C_IsBusy()); // wait for write operation to complete
    // OLATB - Output Latch B
    i2c_writeBuffer[0] = 0x15; // latch register index
    i2c_writeBuffer[1] = 0x00; // 0 = low, 1 = high
    SERCOM0_I2C_Write(i2c_clientAddr, i2c_writeBuffer, 2); // write to i2c via SERCOM0
    while (SERCOM0_I2C_IsBusy()); // wait for write operation to complete
}

/*============================================================================
void led_gpio_update(uint8_t data, uint8_t type)
------------------------------------------------------------------------------
Purpose: update I/O state to turn on or off LEDs
Input  : 
 * data - bit mask of the I/O states to write
 * type - which LEDs to control, button vs scroller
Output : none
Notes  : leds are driven on anode side; 0 = off, 1 = on
============================================================================*/
void led_gpio_update(uint8_t data, uint8_t type) {
    uint8_t i2c_clientAddr = I2C_CLIENT_ADDR_LED_DRIVER; // 7b client address
    uint8_t i2c_writeBuffer[4];
    
    if (type == LED_BUTTON) {
        i2c_writeBuffer[0] = 0x14; // latch register index
        i2c_writeBuffer[1] = data; // 0 = low, 1 = high
        SERCOM0_I2C_Write(i2c_clientAddr, i2c_writeBuffer, 2); // write to i2c via SERCOM0
        while (SERCOM0_I2C_IsBusy()); // wait for write operation to complete
    } else if (type == LED_SCROLLER) {
        i2c_writeBuffer[0] = 0x15; // latch register index
        i2c_writeBuffer[1] = data; // 0 = low, 1 = high
        SERCOM0_I2C_Write(i2c_clientAddr, i2c_writeBuffer, 2); // write to i2c via SERCOM0
        while (SERCOM0_I2C_IsBusy()); // wait for write operation to complete
    }
}

/*============================================================================
void led_reset(void)
------------------------------------------------------------------------------
Purpose: turn off all LEDs
Input  : none
Output : none
Notes  : leds are driven on anode side; 0 = off, 1 = on
============================================================================*/
void led_reset(void)
{
	led_gpio_update(0, LED_BUTTON);
	led_gpio_update(0, LED_SCROLLER);
}

// status registers for previous button and scroller LED states
volatile uint8_t lastButtonMask = 0;
volatile uint8_t lastScrollerMask = 0;
/*============================================================================
void touch_update_leds(void)
------------------------------------------------------------------------------
Purpose: Controls LEDs via MCP23017 I/O expander chip
Input  : none
Output : none
Notes  : requires status registers for last button and scroller LED states
============================================================================*/
void touch_update_leds(void) {
    uint8_t key_status = 0u;
    uint8_t scroller_status = 0u;
    uint16_t scroller_position = 0u;
    uint8_t buttonMask = 0;
    uint8_t scrollerMask = 0;

    // process button status
    key_status = get_sensor_state(0) & KEY_TOUCHED_MASK;
    if (0u != key_status) {
        buttonMask |= 0b00000001;
    } else {
        buttonMask &= 0b11111110;
    }

    key_status = get_sensor_state(1) & KEY_TOUCHED_MASK;
    if (0u != key_status) {
        buttonMask |= 0b00000010;
    } else {
        buttonMask &= 0b11111101;
    }

    key_status = get_sensor_state(2) & KEY_TOUCHED_MASK;
    if (0u != key_status) {
        buttonMask |= 0b00000100;
    } else {
        buttonMask &= 0b11111011;
    }

    key_status = get_sensor_state(3) & KEY_TOUCHED_MASK;
    if (0u != key_status) {
        buttonMask |= 0b00001000;
    } else {
        buttonMask &= 0b11110111;
    }
    // output to LED driver chip
    if (buttonMask != lastButtonMask) {
        lastButtonMask = buttonMask;
        led_gpio_update(reverseBitsInByte(buttonMask), LED_BUTTON);
    }
    
    // process scroller status
    scroller_status = get_scroller_state(0);
    scroller_position = get_scroller_position(0);
    //Example: 8 bit scroller resolution. Modify as per requirement.
    scroller_position = scroller_position >> 5;
    //LED_OFF
    if (0u != scroller_status) {
        switch (scroller_position) {
            case 0:
                scrollerMask = 1 << 0;
                break;
            case 1:
                scrollerMask = 1 << 1;
                break;
            case 2:
                scrollerMask = 1 << 2;
                break;
            case 3:
                scrollerMask = 1 << 3;
                break;
            case 4:
                scrollerMask = 1 << 4;
                break;
            case 5:
                scrollerMask = 1 << 5;
                break;
            case 6:
                scrollerMask = 1 << 6;
                break;
            case 7:
                scrollerMask = 1 << 7;
                break;
            default:
                scrollerMask = 0;
                break;
        }
        // output to LED driver chip
        if (scrollerMask != lastScrollerMask) {
            lastScrollerMask = scrollerMask;
            led_gpio_update(reverseBitsInByte(scrollerMask), LED_SCROLLER);
        }
    } else
    {
        scrollerMask = 0;
        if (scrollerMask != lastScrollerMask) {
            lastScrollerMask = scrollerMask;
            led_gpio_update(reverseBitsInByte(scrollerMask), LED_SCROLLER);
        }
    }

    // control LED0 on CNANO board (PA14, active low) for any sensor touch
    if ((buttonMask > 0) || (scrollerMask > 0)) {
        PORT_PinClear(PORT_PIN_PA14);
    } else {
        PORT_PinSet(PORT_PIN_PA14);
    }
}

/*============================================================================
uint8_t reverseBitsInByte(uint8_t b)
------------------------------------------------------------------------------
Purpose: Reverses all bits in 8b data type
Input  : uint8_t
Output : uint8_t
Notes  : none
============================================================================*/
uint8_t reverseBitsInByte(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}
#endif
