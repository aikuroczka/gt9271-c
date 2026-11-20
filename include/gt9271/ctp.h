/**
 * @file ctp.h
 * @brief GT9271 capacitive touch panel controller driver.
 * @details This driver provides a high-level interface for the Goodix GT9271
 * 10-point capacitive touch controller. It utilizes an event-driven
 * architecture: users poll for changes and then iterate through a stream of
 * touch events.
 */

#ifndef CTP_H
#define CTP_H

#include <gt9271/ctp_platform.h>
#include <gt9271/ctp_types.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================================
   LIFECYCLE & CONFIGURATION
   ========================================================================== */

/**
 * @brief Initializes the device, sets address, verifies communication, and
 * uploads configuration.
 *
 * This function must be called before any other API calls. It performs the
 * hardware reset sequence, selects the I2C address, and uploads the
 * manufacturer config blob.
 *
 * @param[in] ctp Pointer to the device context handler.
 * @param[in] config Pointer to the configuration structure containing PAL and
 * config array.
 * @return CTP_ERROR_NONE on success, or a specific error code on failure.
 * @post The device is initialized and ready for polling.
 */
ctp_error_t ctp_initialize(ctp_device_t* const ctp,
                           const ctp_config_t* const config);

/**
 * @brief Polls the controller for new touch data and updates internal state.
 *
 * This function checks the hardware status register. If new data is available,
 * it reads the coordinate block and updates the internal event list. If an
 * interrupt has been defined then this function will use the state of that
 * boolean to trigger reads and will reset the ISR variable.
 *
 * @param[in,out] ctp Pointer to device handler.
 * @param[out] has_change_occurred Set to true if at least one touch event
 * (START, MOVE, or RELEASE) was detected.
 * @return CTP_ERROR_NONE on success, or a specific error code on failure.
 * @pre @ref ctp_initialize() must have been called and returned CTP_ERROR_NONE
 * @note This function must be called before ctp_get_next_event().
 * @note this function is blocking during I2C reads, expect 0.1-6 milliseconds
 * per call depending on use of ISR, freq of I2C communication, and set max
 * number of touches.
 * @warning calling this function before clearing all pending events with @ref
 * ctp_get_next_event() will result in the clearing of those pending events
 */
ctp_error_t ctp_check_for_change(ctp_device_t* const ctp,
                                 bool* const has_change_occurred);

/**
 * @brief Iterates through the pending touch events.
 *
 * This function provides a stream of events that occurred since the last call
 * to ctp_check_for_change(). Each call returns the next available event and
 * automatically clears it from the internal queue.
 *
 * @param[in,out] ctp Pointer to device handler.
 * @param[out] pt Pointer to a touch structure where the event data will be
 * copied.
 * @return true if an event was returned; false if all pending events have been
 * consumed.
 * @pre @ref ctp_check_for_change() must have been called and returned
 * has_change_occurred = true.
 */
bool ctp_get_next_event(ctp_device_t* const ctp, ctp_touch_t* const pt);

/* ==========================================================================
   DEVICE INFORMATION (Read-Only)
   ========================================================================== */

/** @brief Reads the 32-bit unique product identification number from device
 * memory. */
ctp_error_t ctp_get_product_id(const ctp_device_t* const ctp,
                               uint32_t* const product_id);

/** @brief Reads the 16-bit firmware version from device memory. */
ctp_error_t ctp_get_firmware_version(const ctp_device_t* const ctp,
                                     uint16_t* const firmware_version);

/** @brief Reads the X coordinate resolution (pixels per mm) from device memory.
 */
ctp_error_t ctp_get_x_cord_resolution(const ctp_device_t* const ctp,
                                      uint16_t* const x_cord_res);

/** @brief Reads the Y coordinate resolution (pixels per mm) from device memory.
 */
ctp_error_t ctp_get_y_cord_resolution(const ctp_device_t* const ctp,
                                      uint16_t* const y_cord_res);

/* ==========================================================================
   POWER MANAGEMENT
   ========================================================================== */

/**
 * @brief Enters low-power sleep mode.
 * @details Drives the INT pin low and sends the OFF_SCREEN command to the
 * device.
 * @param ctp Pointer to device handler.
 * @return CTP_ERROR_NONE on success, or error code.
 */
ctp_error_t ctp_enter_sleep(const ctp_device_t* const ctp);

/**
 * @brief Exits sleep mode and returns the device to Normal Mode.
 * @details Drives a high-level wake pulse on the INT pin as required by the
 * datasheet.
 * @param ctp Pointer to device handler.
 * @return CTP_ERROR_NONE on success, or error code.
 * @warning this function is blocking for at least 58 milliseconds.
 */
ctp_error_t ctp_exit_sleep(const ctp_device_t* const ctp);

/* ==========================================================================
CONFIGURATION & STATUS
========================================================================== */

/**
 * @brief Sets the maximum number of touch points to be tracked by the driver.
 *
 * By limiting the number of tracked points, the user can reduce the processing
 * overhead in ctp_check_for_change(). For example, setting this to 2 for a
 * simple button-like interface instead of the hardware maximum of 10.
 *
 * @param[in] ctp Pointer to device handler.
 * @param[in] touch_point_count Number of points to track (Range: 1 to
 * CTP_MAX_TOUCH_POINTS).
 * @return CTP_ERROR_NONE on success, or an error code if the count is out of
 * bounds.
 */
ctp_error_t ctp_set_max_touch_points(ctp_device_t* const ctp,
                                     uint_least8_t touch_point_count);

/**
 * @brief Returns the current limit of tracked touch points.
 * @param[in] ctp Pointer to device handler.
 * @return The maximum number of touch points currently configured for tracking.
 */
uint_least8_t ctp_get_max_touch_points(const ctp_device_t* const ctp);

/**
 * @brief Returns the number of active touches currently detected on the screen.
 * @details This returns the count from the most recent poll. It is useful for
 * quick checks to see if any interaction is occurring without iterating through
 * events.
 * @param[in] ctp Pointer to device handler.
 * @return Number of active touch points (0 to max_touch_points).
 */
uint_least8_t ctp_get_touch_count(const ctp_device_t* const ctp);

#ifdef __cplusplus
}
#endif

#endif /* CTP_H */
