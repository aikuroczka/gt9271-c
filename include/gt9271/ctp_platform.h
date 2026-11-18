/**
 * @file ctp_platform.h
 * @brief Hardware abstraction interface for GT9271 transport.
 */

#ifndef CTP_PLATFORM_H
#define CTP_PLATFORM_H

#include <gt9271/ctp_types.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Function pointer type for configuring GPIO mode to input
 * @param pin Pin identifier
 * @return void
 */
typedef void (*ctp_gpio_config_input_t)(uint16_t pin);

/**
 * @brief Function pointer type for configuring GPIO mode to output
 * @param pin Pin identifier
 * @return void
 */
typedef void (*ctp_gpio_config_output_t)(uint16_t pin);

/**
 * @brief I2C Write: Sends two buffers sequentially without a STOP bit between
 * them.
 */
typedef bool (*ctp_i2c_write_t)(uint16_t addr,
                                const uint_least8_t *const reg_buf,
                                const uint16_t reg_bug_len,
                                const uint_least8_t *const data_buf,
                                const uint16_t data_but_len);

/**
 * @brief I2C Read: Sends a header (reg address), then performs a repeated start
 * to read data.
 */
typedef bool (*ctp_i2c_read_t)(uint16_t addr,
                               const uint_least8_t *const tx_reg_buf,
                               const uint16_t tx_reg_buf_len,
                               uint_least8_t *const rx_buf,
                               const uint16_t rx_len);

/**
 * @brief Function pointer type for GPIO control.
 * @param pin Pin identifier (user-defined).
 * @param state High or Low.
 * @return void
 */
typedef void (*ctp_gpio_write_t)(uint16_t pin, bool state);

/**
 * @brief Function pointer type for GPIO reading.
 * @param pin Pin identifier.
 * @return Current state of the pin.
 */
typedef bool (*ctp_gpio_read_t)(uint16_t pin);

/**
 * @brief Function pointer type for microsecond delays.
 * @param msec Delay in milliseconds.
 */
typedef void (*ctp_delay_msec_t)(uint32_t msec);

/**
 * @brief Platform interface structure containing hardware-specific callbacks.
 */
typedef struct ctp_platform_interface_t {
  ctp_gpio_config_input_t gpio_set_input;
  ctp_gpio_config_output_t gpio_set_output;
  ctp_i2c_write_t i2c_write;
  ctp_i2c_read_t i2c_read;
  ctp_gpio_write_t gpio_write;
  ctp_gpio_read_t gpio_read;
  ctp_delay_msec_t delay_msec;
  uint16_t pin_reset;
  uint16_t pin_int;
  uint16_t pin_wake;
} ctp_platform_interface_t;

/**
 * @brief Device context contains platform interface and cached device states.
 * Used to address specific device
 */
typedef struct ctp_device_t {
  const ctp_platform_interface_t *const plat; // Hardware callbacks
  const uint_least8_t *const config_array;
  const uint_least8_t config_array_length;
  volatile bool *const isr_flag;  ///< pointer to users ISR flag
  bool attention;                 ///< There has been a change a touch
  bool is_initialized;            ///< @ref ctp_start has been called
  uint_least8_t max_touch_points; ///< maximum number of touch points allowed
  uint_least8_t i2c_addr;         ///< i2c write address, set on initialize
  ctp_touch_list_t touch_list;    ///< All touch event data

  /*
   * Keep the receive buffer inside the object to avoid large stack usage
   * in the polling path.
   */
  uint_least8_t rx_buf[CTP_MAX_TOUCH_POINTS];
} ctp_device_t;

#endif // CTP_PLATFORM_H
