/**
 * @file ctp_platform.h
 * @brief Hardware abstraction interface for GT9271 transport.
 * @details This file defines the Platform Abstraction Layer (PAL). To port this
 * driver to a new MCU, the user must implement these callbacks. The library
 * relies on these functions to remain agnostic of the specific hardware
 * registers or SDKs used.
 */

#ifndef CTP_PLATFORM_H
#define CTP_PLATFORM_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Function pointer type for configuring GPIO mode to input.
 * @param[in] pin Pin identifier (platform-specific).
 * @warning This should set the pin to Floating/High-Impedance mode;
 * do NOT enable internal PULLUP resistors.
 */
typedef void (*ctp_gpio_config_input_t)(const uint16_t pin);

/**
 * @brief Function pointer type for configuring GPIO mode to output.
 * @param[in] pin Pin identifier (platform-specific).
 */
typedef void (*ctp_gpio_config_output_t)(const uint16_t pin);

/**
 * @brief I2C Write: Sends two buffers sequentially without a STOP bit between
 * them.
 *
 * The implementation must send the reg_buf, then immediately send the data_buf
 * before issuing the I2C STOP condition.
 *
 * @param[in] addr The 7-bit I2C slave address.
 * @param[in] reg_buf Buffer containing the register address(es).
 * @param[in] reg_bug_len Length of the register buffer.
 * @param[in] data_buf Buffer containing the data to be written.
 * @param[in] data_but_len Length of the data buffer.
 * @return true if the write was successful (ACK received); false otherwise.
 */
typedef bool (*ctp_i2c_write_t)(const uint16_t addr,
                                const uint_least8_t* const reg_buf,
                                const uint16_t reg_bug_len,
                                const uint_least8_t* const data_buf,
                                const uint16_t data_but_len);

/**
 * @brief I2C Read: Sends a header (reg address), then performs a repeated start
 * to read data.
 *
 * The implementation must send the tx_reg_buf, issue a REPEATED START, and then
 * clock in rx_len bytes of data from the slave.
 *
 * @param[in] addr The 7-bit I2C slave address.
 * @param[in] tx_reg_buf Buffer containing the register address(es) to read
 * from.
 * @param[in] tx_reg_buf_len Length of the register buffer.
 * @param[out] rx_buf Buffer where the received data will be stored.
 * @param[in] rx_len Number of bytes to read.
 * @return true if the read was successful; false otherwise.
 */
typedef bool (*ctp_i2c_read_t)(const uint16_t addr,
                               const uint_least8_t* const tx_reg_buf,
                               const uint16_t tx_reg_buf_len,
                               uint_least8_t* const rx_buf,
                               const uint16_t rx_len);

/**
 * @brief Function pointer type for GPIO control.
 * @param[in] pin Pin identifier (platform-specific).
 * @param[in] state High or Low.
 */
typedef void (*ctp_gpio_write_t)(const uint16_t pin, const bool state);

/**
 * @brief Function pointer type for GPIO reading.
 * @param[in] pin Pin identifier (platform-specific).
 * @return Current logical state of the pin (true = HIGH, false = LOW).
 */
typedef bool (*ctp_gpio_read_t)(const uint16_t pin);

/**
 * @brief Function pointer type for microseconds delays.
 * @param[in] usec Delay duration in microseconds.
 */
typedef void (*ctp_delay_usec_t)(const uint32_t usec);

/**
 * @brief Platform interface structure containing hardware-specific callbacks.
 *
 * This structure maps the driver's generic requests to platform-specific
 * implementations. All function pointers must be valid before calling
 * ctp_initialize().
 */
typedef struct ctp_platform_interface_t {
  const ctp_gpio_config_input_t gpio_set_input;
  const ctp_gpio_config_output_t gpio_set_output;
  const ctp_i2c_write_t i2c_write;
  const ctp_i2c_read_t i2c_read;
  const ctp_gpio_write_t gpio_write;
  const ctp_gpio_read_t gpio_read;
  const ctp_delay_usec_t delay_usec;

  /** @brief Reset pin identifier. Must be capable of outputting HIGH/LOW.
   * @warning A pullup (~10kOhm) is required on this pin for proper operation
   */
  const uint16_t pin_reset;

  /**
   * @brief Interrupt pin identifier.
   * @note This pin is used as an input to detect touch events.
   * The GT9271 drives this pin during initialization
   * @warning Do NOT place external pull-up on interrupt pin
   */
  const uint16_t pin_int;
} ctp_platform_interface_t;

#endif  // CTP_PLATFORM_H
