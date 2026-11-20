/**
 * @file ctp_hal.h
 * @brief Internal Hardware Abstraction Layer (HAL) for GT9271.
 * @details This header defines the internal helper functions that bridge the
 * gap between the public API and the platform-specific callbacks. These
 * functions handle the specific I2C protocol requirements of the GT9271, such
 * as register address formatting and status flag management.
 */

#ifndef CTP_HAL_H
#define CTP_HAL_H

#include <gt9271/ctp_types.h>
#include <stdint.h>

/**
 * @brief Reads a block of data from a specific device register.
 * @details Wraps the platform I2C read to handle the 16-bit register address
 * formatting required by the GT9271.
 *
 * @param[in] ctp Pointer to device context.
 * @param[in] reg The 16-bit register address to read from.
 * @param[out] data Buffer to store the received bytes.
 * @param[in] len Number of bytes to read.
 * @return CTP_ERROR_NONE on success, otherwise error code
 */
ctp_error_t i2c_read_reg(const ctp_device_t* const ctp, const uint16_t reg,
                         uint_least8_t* const data, const uint16_t len);

/**
 * @brief Writes a block of data to a specific device register.
 * @details Wraps the platform I2C write to handle the 16-bit register address
 * formatting required by the GT9271.
 *
 * @param[in] ctp Pointer to device context.
 * @param[in] reg The 16-bit register address to write to.
 * @param[in] data Buffer containing the data to be written.
 * @param[in] len Length of the data buffer.
 * @return CTP_ERROR_NONE on success, otherwise error code
 */
ctp_error_t i2c_write_reg(const ctp_device_t* const ctp, const uint16_t reg,
                          const uint_least8_t* const data, const uint16_t len);

/**
 * @brief Checks if the ISR flag is currently set.
 * @return true if an interrupt event is pending; false otherwise.
 */
bool has_isr_event(const ctp_device_t* const ctp);

/**
 * @brief Clears the user-provided ISR flag to acknowledge the interrupt.
 * @param ctp Pointer to device context.
 */
void clear_isr_flag(ctp_device_t* const ctp);

/**
 * @brief High-level check for an ISR event including synchronization delay.
 * @details This function checks for an ISR event, clears it, and applies the
 * required hardware synchronization delay (50us) to ensure registers are
 * updated.
 * @return true if a valid interrupt occurred; false otherwise.
 */
bool has_isr_event_occurred(ctp_device_t* const ctp);

/**
 * @brief Helper to read a 16-bit word from a register.
 * @param[in] ctp Pointer to device context.
 * @param[in] reg The 16-bit register address.
 * @param[out] ret_word Pointer to store the resulting 16-bit value.
 * @return CTP_ERROR_NONE on success, otherwise error code
 */
ctp_error_t get_two_byte_reg_data(const ctp_device_t* const ctp,
                                  const uint16_t reg, uint16_t* const ret_word);

/**
 * @brief Reads the device status register and clears the data flag.
 * @details This function reads the status register to determine if new touch
 * data is available and how many points are active. It then writes a clear
 * command to the status register as required by the GT9271 protocol.
 *
 * @param[in] ctp Pointer to device context.
 * @param[out] has_new_data Set to true if CTP_STATUS_DATA_FLAG is set.
 * @param[out] touch_points Number of active touch points reported by the chip.
 * @return CTP_ERROR_NONE on success,otherwise error code
 */
ctp_error_t ctp_read_status(const ctp_device_t* const ctp,
                            bool* const has_new_data,
                            uint_least8_t* const touch_points);

/**
 * @brief Sets the physical state of the RESET pin.
 * @param[in] plat Pointer to platform interface.
 * @param[in] is_in_reset true to pull RESET low (active), false to release.
 */
void set_reset_state(const ctp_platform_interface_t* const plat,
                     const bool is_in_reset);

/**
 * @brief Executes the hardware sequence to set the I2C address.
 * @details Performs the specific timing sequence of RESET and INT pin
 * manipulation required by the GT9271 to select between default (0x5D)
 * and alternative (0x14) addresses.
 *
 * @param[in] ctp Pointer to device context. Pointers must already be checked.
 * @param[in] use_alt_address true for 0x14, false for 0x5D.
 * @note should only be called once during initilization after pointer checks
 */
void set_i2c_address(ctp_device_t* const ctp, const bool use_alt_address);

/**
 * @brief Verifies that the configuration was correctly written to the device.
 * @details Reads a small chunk of the config memory and compares it against
 * the local config array to ensure communication integrity.
 *
 * @param[in] ctp Pointer to device context.
 * @return CTP_ERROR_NONE if data matches; otherwise error code
 */
ctp_error_t verify_config_data_chunk(const ctp_device_t* const ctp);

/**
 * @brief Uploads the configuration array to the device registers.
 * @param[in] ctp Pointer to device context.
 * @return CTP_ERROR_NONE on success, otherwise error code
 */
ctp_error_t ctp_send_config(const ctp_device_t* const ctp);

/**
 * @brief Parses raw I2C bytes into a structured touch point.
 * @details Extracts track ID, X/Y coordinates, and size from the 8-byte
 * hardware record format.
 *
 * @param[out] touch Pointer to the structure to be populated.
 * @param[in] data Pointer to the raw byte array read from the device.
 */
void parse_data_into_touch(ctp_touch_t* const touch,
                           const uint_least8_t* const data);

/**
 * @brief Prepares the hardware for sleep mode by driving the INT pin low.
 * @param plat Pointer to platform interface.
 */
void hal_prepare_for_sleep(const ctp_platform_interface_t* const plat);

/**
 * @brief Wakes the device from sleep mode by pulsing the INT pin high.
 * @param plat Pointer to platform interface.
 */
void hal_wake_from_sleep(const ctp_platform_interface_t* const plat);

#endif  // CTP_HAL_H
