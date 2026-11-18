/** @file ctp.c
 *  @brief GT9271 capacitive touch panel controller driver implementation.
 */

#include "include/ctp_registers.h"
#include <gt9271/ctp.h>
#include <stddef.h>

/** @brief Mask to extract the least significant byte from a 16-bit word. */
#define CTP_LSB_MASK 0xFFU

/** @brief Bit shift required to align the MSB of a 16-bit word. */
#define CTP_MSB_SHIFT (8U)

/** @brief number of bytes for register, must be sent as header for all i2c
 * commands */
#define CTP_REGISTER_BYTE_COUNT (2U)

/** Timing constants */
#define CTP_SLEEP_WAKE_PULSE_MS 5U
#define CTP_SLEEP_ENTER_PULSE_MS 2U
#define CTP_SLEEP_TO_WAKE_MIN_MS 58U

static inline ctp_error_t validate_context(const ctp_device_t *const ctp) {
  if (ctp == NULL) {
    return CTP_ERROR_HANDLER_IS_NULL;
  }
  if (ctp->plat == NULL) {
    return CTP_ERROR_PLAT_IS_NULL;
  }
  if (ctp->is_initialized == false) {
    return CTP_ERROR_NOT_INITIALIZED;
  }
  return CTP_ERROR_NONE;
}

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

static inline bool is_register_readable(const uint16_t reg) {
  if ((reg >= CTP_REG_PRODUCT_ID) && (reg <= CTP_REG_KEY_VALUE)) {
    return true;
  }
  if ((reg >= CTP_REG_CONFIG_DATA) && (reg <= CTP_REG_CONFIG_FRESH)) {
    return true;
  }
  return false;
}

static ctp_error_t i2c_read_reg(const ctp_device_t *const ctp,
                                const uint16_t reg, uint_least8_t *const data,
                                const uint16_t len) {
  if (len == 0) {
    return CTP_WARNING_LEN_IS_ZERO;
  }
  if (is_register_readable(reg) == false) {
    return CTP_ERROR_ATTEMPT_READ_WRITE_ONLY_REG;
  }

  const uint_least8_t reg_msb = (uint_least8_t)(reg >> CTP_MSB_SHIFT);
  const uint_least8_t reg_lsb = (uint_least8_t)(reg & CTP_LSB_MASK);
  uint_least8_t reg_buf[CTP_REGISTER_BYTE_COUNT] = {reg_msb, reg_lsb};
  bool has_error_occurred = ctp->plat->i2c_read(
      ctp->i2c_addr, reg_buf, CTP_REGISTER_BYTE_COUNT, data, len);
  if (has_error_occurred) {
    return CTP_ERROR_I2C;
  }

  return CTP_ERROR_NONE;
}

static ctp_error_t i2c_write_reg(const ctp_device_t *const ctp,
                                 const uint16_t reg,
                                 const uint_least8_t *const data,
                                 const uint16_t len) {
  if (len == 0) {
    return CTP_WARNING_LEN_IS_ZERO;
  }
  if (is_register_writable(reg) == false) {
    return CTP_ERROR_ATTEMPT_WRITE_READ_ONLY_REG;
  }

  const uint_least8_t reg_msb = (uint_least8_t)(reg >> CTP_MSB_SHIFT);
  const uint_least8_t reg_lsb = (uint_least8_t)(reg & CTP_LSB_MASK);
  uint_least8_t reg_buf[CTP_REGISTER_BYTE_COUNT] = {reg_msb, reg_lsb};
  bool has_error_occurred = ctp->plat->i2c_write(
      ctp->i2c_addr, reg_buf, CTP_REGISTER_BYTE_COUNT, data, len);
  if (has_error_occurred) {
    return CTP_ERROR_I2C;
  }

  return CTP_ERROR_NONE;
}

static inline uint16_t get_uint16_from_bytes(const uint_least8_t msb,
                                             const uint_least8_t lsb) {
  uint16_t value = ((uint16_t)lsb) | ((uint16_t)msb << CTP_MSB_SHIFT);
  return value;
}

static ctp_error_t get_two_byte_reg_data(const ctp_device_t *const ctp,
                                         const uint16_t reg,
                                         uint16_t *const ret_word) {
  uint_least8_t bytes_lsb_first[2] = {0};
  ctp_error_t err = i2c_read_reg(ctp, reg, bytes_lsb_first, 2);
  if (err != CTP_ERROR_NONE) {
    return CTP_ERROR_I2C;
  }

  *ret_word = get_uint16_from_bytes(bytes_lsb_first[1], bytes_lsb_first[0]);

  return CTP_ERROR_NONE;
}

//
ctp_error_t ctp_get_firmware_version(const ctp_device_t *const ctp,
                                     uint16_t *const firmware_version) {
  ctp_error_t err = validate_context(ctp);
  if (err != CTP_ERROR_NONE) {
    return err;
  }
  if (firmware_version == NULL) {
    return CTP_ERROR_NULL_PTR;
  }
  return (get_two_byte_reg_data(ctp, CTP_REG_FW_VERSION, firmware_version));
}

//
ctp_error_t ctp_get_x_cord_resolution(const ctp_device_t *const ctp,
                                      uint16_t *const x_cord_res) {
  ctp_error_t err = validate_context(ctp);
  if (err != CTP_ERROR_NONE) {
    return err;
  }
  if (x_cord_res == NULL) {
    return CTP_ERROR_NULL_PTR;
  }
  return (get_two_byte_reg_data(ctp, CTP_REG_X_CORD_RES, x_cord_res));
}

//
ctp_error_t ctp_get_y_cord_resolution(const ctp_device_t *const ctp,
                                      uint16_t *const y_cord_res) {
  ctp_error_t err = validate_context(ctp);
  if (err != CTP_ERROR_NONE) {
    return err;
  }
  if (y_cord_res == NULL) {
    return CTP_ERROR_NULL_PTR;
  }
  return (get_two_byte_reg_data(ctp, CTP_REG_Y_CORD_RES, y_cord_res));
}

//
ctp_error_t ctp_get_product_id(const ctp_device_t *const ctp,
                               uint32_t *const product_id) {
  ctp_error_t err = validate_context(ctp);
  if (err != CTP_ERROR_NONE) {
    return err;
  }
  if (product_id == NULL) {
    return CTP_ERROR_NULL_PTR;
  }
  uint_least8_t bytes_lsb_first[CTP_REG_PRODUCT_ID_SIZE] = {0};

  err = i2c_read_reg(ctp, CTP_REG_PRODUCT_ID, bytes_lsb_first,
                     CTP_REG_PRODUCT_ID_SIZE);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  *product_id = 0;
  for (uint_least8_t i = 0; i < CTP_REG_PRODUCT_ID_SIZE; i++) {
    *product_id |= (((uint32_t)bytes_lsb_first[i]) << (i * 8));
  }

  return CTP_ERROR_NONE;
}

static ctp_error_t test_comms(const ctp_device_t *const ctp) {
  // Simple test to verify i2c communication. Will check if the product id
  // `seems` valid
  uint32_t product_id = 0;
  ctp_error_t err = ctp_get_product_id(ctp, &product_id);
  // Most communication errors will usually return as all binary 1s or 0s
  if (product_id == 0 || product_id == 0xFFFF) {
    return CTP_ERROR_I2C;
  }
  return err;
}

static void parse_data_into_touch(ctp_touch_t *const touch,
                                  const uint_least8_t *const data) {
  touch->track_id = data[CTP_TOUCH_DATA_TRACK_ID_OFFSET];

  touch->x = get_uint16_from_bytes(data[CTP_TOUCH_DATA_X_HIGH_OFFSET],
                                   data[CTP_TOUCH_DATA_X_LOW_OFFSET]);

  touch->y = get_uint16_from_bytes(data[CTP_TOUCH_DATA_Y_HIGH_OFFSET],
                                   data[CTP_TOUCH_DATA_Y_LOW_OFFSET]);

  touch->size = get_uint16_from_bytes(data[CTP_TOUCH_DATA_SIZE_HIGH_OFFSET],
                                      data[CTP_TOUCH_DATA_SIZE_LOW_OFFSET]);

  touch->is_touched = false;
  touch->event = CTP_EVENT_NONE;
}

static void set_reset_state(const ctp_platform_interface_t *const plat,
                            const bool is_in_reset) {
  // reset pin is active low, this inverse of is_in_reset
  plat->gpio_write(plat->pin_reset, !is_in_reset);
}

/**
 * @brief Executes the hardware sequence to set the I2C address.
 * @param ctp Pointer to device context.
 * @param use_alt_address Boolean to select between default and alternate
 * address.
 * @return CTP_ERROR_NONE on success, or error code.
 */
static ctp_error_t set_i2c_address(ctp_device_t *const ctp,
                                   const bool use_alt_address) {
  if (ctp == NULL || ctp->plat == NULL) {
    return CTP_ERROR_NULL_PTR;
  }

  const ctp_platform_interface_t *const plat = ctp->plat;

  /* Set pins to output for address selection sequence */
  plat->gpio_set_output(plat->pin_reset);
  plat->gpio_set_output(plat->pin_int);

  set_reset_state(plat, true);
  plat->gpio_write(plat->pin_int, use_alt_address);

  plat->delay_msec(CTP_RESET_LOW_DELAY_MS);

  set_reset_state(plat, false);
  plat->delay_msec(CTP_ADDRESS_HOLD_DELAY_MS);

  /* Return INT to input for interrupt sensing */
  plat->gpio_set_input(plat->pin_int);
  plat->delay_msec(CTP_POST_RESET_DELAY_MS);

  ctp->i2c_addr = use_alt_address ? CTP_I2C_ADDR_ALT : CTP_I2C_ADDR_DEFAULT;

  return CTP_ERROR_NONE;
}

static bool is_touch_valid(const ctp_touch_t *const touch) {
  if (touch == NULL) {
    return false;
  }

  // ids range from 1 to CTP_MAX_TOUCH_POINTS
  if ((touch->track_id == 0) || (touch->track_id > CTP_MAX_TOUCH_POINTS)) {
    return false;
  }

  if ((touch->size < CTP_MIN_TOUCH_SIZE) ||
      (touch->size > CTP_MAX_TOUCH_SIZE)) {
    return false;
  }

  if ((touch->x > CTP_MAX_X) || (touch->y > CTP_MAX_Y)) {
    return false;
  }

  // 0 reads are a common error due to noise induced `ghost touches`
  // It is very unlikely to be able to actually touch the single top left pixel
  // so we can simply reject these touches
  if ((touch->x == 0) && (touch->y == 0)) {
    return false;
  }

  return true;
}

static bool are_touches_equal(const ctp_touch_t *const a,
                              const ctp_touch_t *const b) {
  if (a->track_id != b->track_id) {
    return false;
  }

  if (a->x != b->x) {
    return false;
  }

  if (a->y != b->y) {
    return false;
  }

  if (a->size != b->size) {
    return false;
  }

  return true;
}

static void copy_touch(ctp_touch_t *const dest, const ctp_touch_t *const src) {
  dest->track_id = src->track_id;
  dest->x = src->x;
  dest->y = src->y;
  dest->size = src->size;
}

static ctp_error_t ctp_send_config(const ctp_device_t *const ctp) {
  ctp_error_t err = CTP_ERROR_CONFIG_NOT_SENT;
  for (uint_least8_t retry = 0; retry < CTP_CONFIG_RETRY_COUNT; retry++) {
    err = i2c_write_reg(ctp, CTP_REG_CONFIG_DATA, ctp->config_array,
                        ctp->config_array_length);

    if (err == CTP_ERROR_NONE) {
      return CTP_ERROR_NONE;
    }

    ctp->plat->delay_msec(CTP_I2C_RETRY_DELAY_MS);
  }
  return err;
}

void ctp_set_number_of_touch_points(ctp_device_t *const ctp,
                                    uint_least8_t touch_point_count) {
  if (touch_point_count > CTP_MAX_TOUCH_POINTS) {
    touch_point_count = CTP_MAX_TOUCH_POINTS;
  }
  if (touch_point_count == 0) {
    touch_point_count = 1;
  }

  ctp->max_touch_points = touch_point_count;
}

uint_least8_t
ctp_get_set_number_of_touch_points(const ctp_device_t *const ctp) {
  return ctp->max_touch_points;
}

ctp_error_t ctp_read_status(const ctp_device_t *const ctp,
                            bool *const has_new_data,
                            uint_least8_t *const touch_points) {
  ctp_error_t err = validate_context(ctp);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  uint_least8_t status = 0;
  err = i2c_read_reg(ctp, CTP_REG_STATUS, &status, 1);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  *has_new_data = (status & CTP_STATUS_DATA_FLAG) != 0;
  *touch_points = (status & CTP_STATUS_TOUCH_MASK);

  return CTP_ERROR_NONE;
}

ctp_error_t ctp_check_for_change(ctp_device_t *const ctp,
                                 bool *const has_change_occurred) {
  ctp_error_t err = validate_context(ctp);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  bool has_new_data = 0;
  uint_least8_t touch_point_count = 0;
  err = ctp_read_status(ctp, &has_new_data, &touch_point_count);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  if ((has_new_data == false) || (ctp->touch_list.count == touch_point_count)) {
    *has_change_occurred = false;
    return CTP_ERROR_NONE;
  }

  uint_least8_t max_touch = ctp->max_touch_points > CTP_MAX_TOUCH_POINTS
                                ? CTP_MAX_TOUCH_POINTS
                                : ctp->max_touch_points;

  /*
   * Read up to the hardware maximum.
   *
   * Do not cap the read to CTP_MAX_TOUCH_POINTS here, because a lower
   * application limit could otherwise miss a lower track ID that appears later
   * in the reported point list.
   */
  if (touch_point_count > CTP_MAX_TOUCH_POINTS) {
    touch_point_count = CTP_MAX_TOUCH_POINTS;
  }

  err = i2c_read_reg(ctp, CTP_REG_TOUCH_DATA, ctp->rx_buf,
                     touch_point_count * CTP_BYTES_PER_TOUCH);
  if (err != CTP_ERROR_NONE) {
    // Do not clear the data flag if the coordinate read failed.
    return err;
  }

  uint_least8_t write_data = CTP_STATUS_REG_CLEAR;
  err = i2c_write_reg(ctp, CTP_REG_STATUS, &write_data, 1);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  bool changed = false;
  uint16_t present_mask = 0;
  uint_least8_t active = 0;

  for (uint_least8_t i = 0; i < touch_point_count; i++) {
    ctp_touch_t pt;

    parse_data_into_touch(&pt, &ctp->rx_buf[i * CTP_BYTES_PER_TOUCH]);

    if (!is_touch_valid(&pt)) {
      continue;
    }

    /*
     * GT9271 track IDs are 1-based:
     *   track ID 1 -> slot 0
     *   track ID 2 -> slot 1
     *   ...
     *   track ID 10 -> slot 9
     */
    uint_least8_t slot = (uint_least8_t)(pt.track_id - 1);

    if (slot >= max_touch) {
      continue;
    }

    present_mask |= (uint16_t)(1u << slot);

    ctp_touch_t *const touch = &ctp->touch_list.touch[slot];
    ctp_event_t event;

    if (!touch->is_touched) {
      event = CTP_EVENT_START;
    } else if (!are_touches_equal(touch, &pt)) {
      event = CTP_EVENT_MOVE;
    } else {
      event = CTP_EVENT_NONE;
    }

    copy_touch(touch, &pt);
    touch->is_touched = true;
    touch->event = event;

    if (event != CTP_EVENT_NONE) {
      changed = true;
    }

    active++;
  }

  /*
   * Release any previously tracked IDs that are no longer present.
   */
  for (uint_least8_t i = 0; i < max_touch; i++) {
    ctp_touch_t *const touch = &ctp->touch_list.touch[i];
    if (touch->is_touched && ((present_mask & (uint16_t)(1u << i)) == 0)) {
      touch->is_touched = false;
      touch->event = CTP_EVENT_RELEASE;
      changed = true;
    }
  }

  /*
   * If CTP_MAX_TOUCH_POINTS is less than CTP_ABSOLUTE_MAX_TOUCH,
   * release any stale slots above the application limit.
   */
  for (uint_least8_t i = max_touch; i < CTP_MAX_TOUCH_POINTS; i++) {
    ctp_touch_t *const touch = &ctp->touch_list.touch[i];
    if (touch->is_touched) {
      touch->is_touched = false;
      touch->event = CTP_EVENT_RELEASE;
      changed = true;
    }
  }

  if (ctp->touch_list.count != active) {
    ctp->touch_list.count = active;
    changed = true;
  }

  ctp->attention = changed;
  *has_change_occurred = changed;
  return CTP_ERROR_NONE;
}

ctp_error_t ctp_process_isr(ctp_device_t *const ctp,
                            bool *const has_change_occurred) {
  ctp_error_t err = validate_context(ctp);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  if (ctp_has_isr_event(ctp) == false) {
    *has_change_occurred = false;
    return CTP_ERROR_NONE;
  }

  ctp_clear_isr_flag(ctp);

  err = ctp_check_for_change(ctp, has_change_occurred);
  return err;
}

void ctp_clear_touch(ctp_touch_t *const touch) {
  if (touch == NULL) {
    return;
  }

  touch->track_id = 0;
  touch->x = 0;
  touch->y = 0;
  touch->size = 0;
  touch->is_touched = false;
  touch->event = CTP_EVENT_NONE;
}

void ctp_clear_touch_event_list(ctp_touch_list_t *const touch_list) {
  if (touch_list == NULL) {
    return;
  }

  for (uint_least8_t i = 0; i < CTP_MAX_TOUCH_POINTS; i++) {
    ctp_clear_touch(&touch_list->touch[i]);
  }

  touch_list->count = 0;
  touch_list->key_status = 0;
}

bool ctp_has_isr_event(const ctp_device_t *const ctp) {
  if ((ctp == NULL) || (ctp->isr_flag == NULL)) {
    return false;
  }
  return *(ctp->isr_flag);
}

void ctp_clear_isr_flag(ctp_device_t *const ctp) {
  if ((ctp == NULL) || (ctp->isr_flag == NULL)) {
    return;
  }
  *(ctp->isr_flag) = false;
}

uint16_t ctp_get_x(const ctp_device_t *const ctp, uint_least8_t idx) {
  if ((ctp == NULL) || (idx >= CTP_MAX_TOUCH_POINTS)) {
    return 0;
  }
  return ctp->touch_list.touch[idx].x;
}

uint16_t ctp_get_y(const ctp_device_t *const ctp, uint_least8_t idx) {
  if ((ctp == NULL) || (idx >= CTP_MAX_TOUCH_POINTS)) {
    return 0;
  }
  return ctp->touch_list.touch[idx].y;
}

ctp_event_t ctp_get_event(const ctp_device_t *const ctp,
                          const uint_least8_t idx) {
  if ((ctp == NULL) || (idx >= CTP_MAX_TOUCH_POINTS)) {
    return CTP_EVENT_NONE;
  }
  return ctp->touch_list.touch[idx].event;
}

void ctp_clear_touch_event(ctp_touch_t *const touch) {
  if (touch == NULL) {
    return;
  }

  touch->event = CTP_EVENT_NONE;
}

bool ctp_is_touched(const ctp_device_t *const ctp) {
  if (ctp == NULL) {
    return false;
  }

  return ctp->touch_list.count > 0;
}

uint_least8_t ctp_get_touch_count(const ctp_device_t *const ctp) {
  if (ctp == NULL) {
    return 0;
  }

  return ctp->touch_list.count;
}

uint_least8_t ctp_get_key_status(const ctp_device_t *const ctp) {
  if (ctp == NULL) {
    return 0;
  }

  return ctp->touch_list.key_status;
}

ctp_error_t ctp_enter_sleep(const ctp_device_t *const ctp) {
  ctp_error_t err = validate_context(ctp);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  const ctp_platform_interface_t *const plat = ctp->plat;
  plat->gpio_set_output(plat->pin_int);
  plat->gpio_write(plat->pin_int, false);
  plat->delay_msec(CTP_SLEEP_ENTER_PULSE_MS);

  uint_least8_t write_data = CTP_COMMAND_OFF_SCREEN;
  err = i2c_write_reg(ctp, CTP_REG_COMMAND, &write_data, 1);

  plat->gpio_set_input(plat->pin_int);

  return err;
}

ctp_error_t ctp_exit_sleep(const ctp_device_t *const ctp) {
  ctp_error_t err = validate_context(ctp);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  const ctp_platform_interface_t *const plat = ctp->plat;
  plat->gpio_set_output(plat->pin_int);
  plat->gpio_write(plat->pin_int, true);
  plat->delay_msec(CTP_SLEEP_WAKE_PULSE_MS);

  plat->gpio_set_input(plat->pin_int);

  return CTP_ERROR_NONE;
}

ctp_error_t ctp_initialize(ctp_device_t *const ctp,
                           const bool use_alt_address) {
  if (ctp == NULL) {
    return CTP_ERROR_HANDLER_IS_NULL;
  }
  if (ctp->plat == NULL) {
    return CTP_ERROR_PLAT_IS_NULL;
  }
  if ((ctp->config_array == NULL) ||
      (ctp->config_array_length != CTP_CONFIG_REQUIRED_LEN)) {
    return CTP_ERROR_CONFIG_INVALID;
  }

  ctp_error_t err = set_i2c_address(ctp, use_alt_address);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  err = test_comms(ctp);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  err = ctp_send_config(ctp);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  ctp_clear_touch_event_list(&ctp->touch_list);
  ctp->is_initialized = true;
  ctp->attention = false;
  return CTP_ERROR_NONE;
}
