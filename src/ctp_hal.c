/**
 * @file ctp_hal.c
 * @brief Implementation of the Internal Hardware Abstraction Layer for GT9271.
 * @details This file implements the "glue logic" that translates high-level
 * driver requests into specific I2C and GPIO sequences. It handles register
 * address formatting, hardware synchronization delays, and the critical device
 * initialization sequence.
 */

#include "include/ctp_hal.h"

#include <stdbool.h>
#include <stddef.h>

#include "include/ctp_registers.h"

/* I2C addresses (7-bit) */
#define CTP_I2C_ADDR_DEFAULT (0x5D)  ///< Default 7-bit address
#define CTP_I2C_ADDR_ALT (0x14)      ///< Alternative 7-bit address

/* Configuration verification constants */
#define CTP_CONFIG_RAW_CHECK_BYTE_COUNT (8U)

/** @brief Mask to extract the least significant byte from a 16-bit word. */
#define CTP_LSB_MASK 0xFFU

/** @brief Bit shift required to align the MSB of a 16-bit word. */
#define CTP_MSB_SHIFT (8U)

/** @brief Number of bytes for register address header in I2C commands. */
#define CTP_REGISTER_BYTE_COUNT (2U)

/** Timing constants based on GT9271 Datasheet Rev 0.0 */
#define CTP_SLEEP_ENTER_PULSE_US (2000U)
#define CTP_RESET_DELAY_AFTER_INT_US (1000U)
#define CTP_HOLD_INT_DUR_US (10000U)
#define CTP_SLEEP_WAKE_PULSE_US (5000U)

/**
 * @brief Assembles a 16-bit word from two bytes.
 * @note GT9271 uses Little-Endian format for multi-byte register data.
 */
static inline uint16_t get_uint16_from_bytes(const uint_least8_t msb,
                                             const uint_least8_t lsb) {
  return ((uint16_t)lsb) | ((uint16_t)msb << CTP_MSB_SHIFT);
}

/**
 * @brief Validates if a register is writable to prevent invalid I2C
 * transactions.
 */
static inline bool is_register_writable(const uint16_t reg) {
  if ((reg == CTP_REG_STATUS) || (reg == CTP_REG_COMMAND) ||
      (reg == CTP_REG_ESD_CHECK)) {
    return true;
  }
  if ((reg >= CTP_REG_CONFIG_DATA) && (reg <= CTP_REG_CONFIG_FRESH)) {
    return true;
  }
  return false;
}

/**
 * @brief Validates if a register is readable to prevent I2C bus hangs.
 */
static inline bool is_register_readable(const uint16_t reg) {
  if ((reg >= CTP_REG_PRODUCT_ID) && (reg <= CTP_REG_KEY_VALUE)) {
    return true;
  }
  if ((reg >= CTP_REG_CONFIG_DATA) && (reg <= CTP_REG_CONFIG_FRESH)) {
    return true;
  }
  return false;
}

ctp_error_t i2c_read_reg(const ctp_device_t* const ctp, const uint16_t reg,
                         uint_least8_t* const data, const uint16_t len) {
  if (len == 0) {
    return CTP_WARNING_LEN_IS_ZERO;
  }
  if (is_register_readable(reg) == false) {
    return CTP_ERROR_ATTEMPT_READ_WRITE_ONLY_REG;
  }

  /* Format the 16-bit register address into a 2-byte buffer for I2C
   * transmission */
  const uint_least8_t reg_msb = (uint_least8_t)(reg >> CTP_MSB_SHIFT);
  const uint_least8_t reg_lsb = (uint_least8_t)(reg & CTP_LSB_MASK);
  uint_least8_t reg_buf[CTP_REGISTER_BYTE_COUNT] = {reg_msb, reg_lsb};

  bool is_success = ctp->config->plat->i2c_read(
      ctp->i2c_addr, reg_buf, CTP_REGISTER_BYTE_COUNT, data, len);

  return (is_success == false) ? CTP_ERROR_I2C : CTP_ERROR_NONE;
}

ctp_error_t i2c_write_reg(const ctp_device_t* const ctp, const uint16_t reg,
                          const uint_least8_t* const data, const uint16_t len) {
  if (len == 0) {
    return CTP_WARNING_LEN_IS_ZERO;
  }
  if (is_register_writable(reg) == false) {
    return CTP_ERROR_ATTEMPT_WRITE_READ_ONLY_REG;
  }

  const uint_least8_t reg_msb = (uint_least8_t)(reg >> CTP_MSB_SHIFT);
  const uint_least8_t reg_lsb = (uint_least8_t)(reg & CTP_LSB_MASK);
  uint_least8_t reg_buf[CTP_REGISTER_BYTE_COUNT] = {reg_msb, reg_lsb};

  bool is_success = ctp->config->plat->i2c_write(
      ctp->i2c_addr, reg_buf, CTP_REGISTER_BYTE_COUNT, data, len);

  return (is_success == false) ? CTP_ERROR_I2C : CTP_ERROR_NONE;
}

bool has_isr_event(const ctp_device_t* const ctp) {
  if ((ctp == NULL) || (ctp->config->isr_flag == NULL)) {
    return false;
  }
  return *(ctp->config->isr_flag);
}

void clear_isr_flag(ctp_device_t* const ctp) {
  if ((ctp == NULL) || (ctp->config->isr_flag == NULL)) {
    return;
  }
  *(ctp->config->isr_flag) = false;
}

bool has_isr_event_occurred(ctp_device_t* const ctp) {
  // if ISR flag is not configured, we default to polling mode
  if (ctp->config->isr_flag == NULL) {
    return true;
  }

  if (has_isr_event(ctp) == false) {
    return false;
  }

  clear_isr_flag(ctp);
  /* Hardware Sync: Give the GT9271 time to update internal registers after INT
   * pulse */
  ctp->config->plat->delay_usec(50);
  return true;
}

ctp_error_t get_two_byte_reg_data(const ctp_device_t* const ctp,
                                  const uint16_t reg,
                                  uint16_t* const ret_word) {
  uint_least8_t bytes_lsb_first[2] = {0};
  ctp_error_t err = i2c_read_reg(ctp, reg, bytes_lsb_first, 2);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  *ret_word = get_uint16_from_bytes(bytes_lsb_first[1], bytes_lsb_first[0]);
  return CTP_ERROR_NONE;
}

ctp_error_t ctp_read_status(const ctp_device_t* const ctp,
                            bool* const has_new_data,
                            uint_least8_t* const touch_points) {
  uint_least8_t status = 0;
  ctp_error_t err = i2c_read_reg(ctp, CTP_REG_STATUS, &status, 1);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  *has_new_data = (status & CTP_STATUS_DATA_FLAG) != 0;
  *touch_points = (status & CTP_STATUS_TOUCH_MASK);

  /* GT9271 Protocol: The status flag must be cleared by writing 0x00 to the
   * register */
  uint_least8_t write_data = CTP_STATUS_REG_CLEAR;
  err = i2c_write_reg(ctp, CTP_REG_STATUS, &write_data, 1);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  return CTP_ERROR_NONE;
}

void set_reset_state(const ctp_platform_interface_t* const plat,
                     const bool is_in_reset) {
  /* Reset pin is active low: true (in reset) -> LOW, false (out of reset) ->
   * HIGH */
  plat->gpio_write(plat->pin_reset, !is_in_reset);
}

void set_i2c_address(ctp_device_t* const ctp, const bool use_alt_address) {
  const ctp_platform_interface_t* const plat = ctp->config->plat;

  /* Address Selection Sequence: Requires specific timing on RESET and INT pins
   */
  plat->gpio_set_output(plat->pin_reset);
  plat->gpio_set_output(plat->pin_int);

  set_reset_state(plat, true);
  plat->gpio_write(plat->pin_int, use_alt_address);  // HIGH = 0x14, LOW = 0x5D

  plat->delay_usec(CTP_RESET_DELAY_AFTER_INT_US);
  set_reset_state(plat, false);
  plat->delay_usec(CTP_HOLD_INT_DUR_US);

  /* Restore INT pin to input mode for interrupt sensing */
  plat->gpio_set_input(plat->pin_int);

  ctp->i2c_addr = use_alt_address ? CTP_I2C_ADDR_ALT : CTP_I2C_ADDR_DEFAULT;
}

ctp_error_t verify_config_data_chunk(const ctp_device_t* const ctp) {
  if (CTP_CONFIG_RAW_CHECK_BYTE_COUNT > ctp->config->config_array_length) {
    return CTP_ERROR_DEFS_INVALID;
  }

  uint_least8_t config_read_data[CTP_CONFIG_RAW_CHECK_BYTE_COUNT] = {0};
  ctp_error_t err = i2c_read_reg(ctp, CTP_REG_CONFIG_DATA, config_read_data,
                                 CTP_CONFIG_RAW_CHECK_BYTE_COUNT);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  for (uint_least8_t i = 0; i < CTP_CONFIG_RAW_CHECK_BYTE_COUNT; i++) {
    if (config_read_data[i] != ctp->config->config_array[i]) {
      return CTP_ERROR_CONFIG_RAW_CHECK_FAIL;
    }
  }

  return CTP_ERROR_NONE;
}

ctp_error_t ctp_send_config(const ctp_device_t* const ctp) {
  /* Upload the full configuration block to the device */
  ctp_error_t err =
      i2c_write_reg(ctp, CTP_REG_CONFIG_DATA, ctp->config->config_array,
                    ctp->config->config_array_length);

  return (err == CTP_ERROR_NONE) ? CTP_ERROR_NONE : err;
}

void parse_data_into_touch(ctp_touch_t* const touch,
                           const uint_least8_t* const data) {
  /* Extract fields based on the GT9271 coordinate register layout */
  touch->track_id = data[CTP_TOUCH_DATA_TRACK_ID_OFFSET];

  touch->x = get_uint16_from_bytes(data[CTP_TOUCH_DATA_X_HIGH_OFFSET],
                                   data[CTP_TOUCH_DATA_X_LOW_OFFSET]);

  touch->y = get_uint16_from_bytes(data[CTP_TOUCH_DATA_Y_HIGH_OFFSET],
                                   data[CTP_TOUCH_DATA_Y_LOW_OFFSET]);

  touch->size = get_uint16_from_bytes(data[CTP_TOUCH_DATA_SIZE_HIGH_OFFSET],
                                      data[CTP_TOUCH_DATA_SIZE_LOW_OFFSET]);

  /* Initialize state flags; these are updated by the logic layer in ctp.c */
  touch->is_touched = false;
  touch->event = CTP_EVENT_NONE;
}

void hal_prepare_for_sleep(const ctp_platform_interface_t* const plat) {
  if (plat == NULL) {
    return;
  }
  plat->gpio_set_output(plat->pin_int);
  plat->gpio_write(plat->pin_int, false);
  plat->delay_usec(CTP_SLEEP_ENTER_PULSE_US);
}

void hal_wake_from_sleep(const ctp_platform_interface_t* const plat) {
  if (plat == NULL) {
    return;
  }
  plat->gpio_set_output(plat->pin_int);
  plat->gpio_write(plat->pin_int, true);
  plat->delay_usec(CTP_SLEEP_WAKE_PULSE_US);
  plat->gpio_set_input(plat->pin_int);
}
