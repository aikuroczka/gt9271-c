/** @file ctp.h
 *  @brief GT9271 capacitive touch panel controller driver.
 *  @details Driver for the Goodix GT9271 10-point capacitive touch controller.
 */
#ifndef CTP_H
#define CTP_H

#include <gt9271/ctp_platform.h>
#include <gt9271/ctp_types.h>

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief Initializes the device, sets address, verifys communication, and
     * uploads configuration.
     * @param ctp Pointer to device handler.
     * @param use_alt_address true to use alternative I2C address for multi-device
     * support on one bus. False to use default address.
     * @return CTP_ERROR_NONE on success, or error code.
     * @note This function must be called first
     */
    ctp_error_t ctp_initialize(ctp_device_t *const ctp, const ctp_config_t *const config);

    /** @brief attempts to read firmware version from device memory
     * @param ctp pointer to device handler, must be initialized
     * @param firmware_version pointer to data location to store 16bit firmware
     * version
     * @return CTP_ERROR_NONE if success, otherwise specific error code
     */
    ctp_error_t ctp_get_firmware_version(const ctp_device_t *const ctp,
                                         uint16_t *const firmware_version);

    /** @brief attempts to read x cordinate resolution from device memory
     * @param ctp pointer to device handler, must be initialized
     * @param x_cord_res pointer to data location to store 16bit x res
     * @return CTP_ERROR_NONE if success, otherwise specific error code
     */
    ctp_error_t ctp_get_x_cord_resolution(const ctp_device_t *const ctp,
                                          uint16_t *const x_cord_res);

    /** @brief attempts to read y cordinate resolution from device memory
     * @param ctp pointer to device handler, must be initialized
     * @param y_cord_res pointer to data location to store 16bit y res
     * @return CTP_ERROR_NONE if success, otherwise specific error code
     */
    ctp_error_t ctp_get_y_cord_resolution(const ctp_device_t *const ctp,
                                          uint16_t *const y_cord_res);

    /** @brief attempts to read unique product identification number from device
     * memory
     * @param ctp pointer to device handler, must be initialized
     * @param product_id pointer to data location to store 32bit product id
     * @return CTP_ERROR_NONE if success, otherwise specific error code
     */
    ctp_error_t ctp_get_product_id(const ctp_device_t *const ctp,
                                   uint32_t *const product_id);

    /** @brief Enter sleep mode.
     *  @param ctp Pointer to device handler
     *  @return CTP_ERROR_OK on success
     *  @note Verify the exact sleep command sequence against the full GT9271
     * register map.
     */
    ctp_error_t ctp_enter_sleep(const ctp_device_t *const ctp);

    /** @brief Exit sleep mode by driving INT high for the required wake pulse.
     *  @param ctp Pointer to device handler
     *  @return CTP_ERROR_OK on success
     */
    ctp_error_t ctp_exit_sleep(const ctp_device_t *const ctp);

    /** @brief Poll the controller and update touch state.
     *  @param ctp Pointer to device handler
     *  @return true if any touch state changed
     */
    ctp_error_t ctp_check_for_change(ctp_device_t *ctp,
                                     bool *const has_change_occurred);

    //////////////

    /** @brief Clear one touch event.
     *  @param touch Pointer to touch event
     */
    void ctp_clear_touch_event(ctp_touch_t *touch);

    /** @brief Clear the full touch list.
     *  @param touch Pointer to touch list
     */
    void ctp_clear_touch_event_list(ctp_touch_list_t *touch);

    /** @brief Get X coordinate for a touch slot.
     *  @param ctp Pointer to device handler
     *  @param idx Touch slot index
     *  @return X coordinate, or 0 if invalid
     */
    uint16_t ctp_get_x(const ctp_device_t *const ctp, uint_least8_t idx);

    /** @brief Get Y coordinate for a touch slot.
     *  @param ctp Pointer to device handler
     *  @param idx Touch slot index
     *  @return Y coordinate, or 0 if invalid
     */
    uint16_t ctp_get_y(const ctp_device_t *const ctp, uint_least8_t idx);

    /** @brief Get event state for a touch slot.
     *  @param ctp Pointer to device handler
     *  @param idx Touch slot index
     *  @return Current event state
     */
    ctp_event_t ctp_get_event(const ctp_device_t *const ctp, uint_least8_t idx);

    /** @brief Clear the event state for one touch.
     *  @param touch Pointer to touch event
     */
    void ctp_clear_event(ctp_touch_t *touch);

    /** @brief Check whether any touch is active.
     *  @param ctp Pointer to device handler
     *  @return true if touched
     */
    bool ctp_is_touched(const ctp_device_t *const ctp);

    /** @brief Get active touch count.
     *  @param ctp Pointer to device handler
     *  @return Number of active touches
     */
    uint_least8_t ctp_get_touch_count(const ctp_device_t *const ctp);

    /** @brief Get raw key/status bits from the status register.
     *  @param ctp Pointer to device handler
     *  @return Key/status bits
     */
    uint_least8_t ctp_get_key_status(const ctp_device_t *const ctp);

#ifdef __cplusplus
}
#endif

#endif /* CTP_H */
