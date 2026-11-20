/**
 * @file ctp.c
 * @brief Core logic implementation for the GT9271 capacitive touch panel
 * driver.
 * @details This file implements the "Logic Layer" of the driver. It is
 * responsible for:
 * 1. Orchestrating the device lifecycle (Initialization, Sleep/Wake).
 * 2. Managing the touch state machine (detecting START, MOVE, and RELEASE
 * events).
 * 3. Implementing an event-driven iterator to provide a clean API for the user.
 */

#include <gt9271/ctp.h>
#include <stddef.h>

#include "include/ctp_hal.h"
#include "include/ctp_registers.h"
#include "include/ctp_touch.h"

static inline ctp_error_t validate_context(const ctp_device_t* const ctp) {
  if (ctp == NULL) {
    return CTP_ERROR_HANDLER_IS_NULL;
  }
  if (ctp->config == NULL) {
    return CTP_ERROR_CONFIG_IS_NULL;
  }
  if (ctp->config->plat == NULL) {
    return CTP_ERROR_PLAT_IS_NULL;
  }
  if (ctp->is_initialized == false) {
    return CTP_ERROR_NOT_INITIALIZED;
  }
  return CTP_ERROR_NONE;
}

ctp_error_t ctp_get_firmware_version(const ctp_device_t* const ctp,
                                     uint16_t* const firmware_version) {
  ctp_error_t err = validate_context(ctp);
  if (err != CTP_ERROR_NONE) {
    return err;
  }
  if (firmware_version == NULL) {
    return CTP_ERROR_NULL_PTR;
  }
  return (get_two_byte_reg_data(ctp, CTP_REG_FW_VERSION, firmware_version));
}

ctp_error_t ctp_get_x_cord_resolution(const ctp_device_t* const ctp,
                                      uint16_t* const x_cord_res) {
  ctp_error_t err = validate_context(ctp);
  if (err != CTP_ERROR_NONE) {
    return err;
  }
  if (x_cord_res == NULL) {
    return CTP_ERROR_NULL_PTR;
  }
  return (get_two_byte_reg_data(ctp, CTP_REG_X_CORD_RES, x_cord_res));
}

ctp_error_t ctp_get_y_cord_resolution(const ctp_device_t* const ctp,
                                      uint16_t* const y_cord_res) {
  ctp_error_t err = validate_context(ctp);
  if (err != CTP_ERROR_NONE) {
    return err;
  }
  if (y_cord_res == NULL) {
    return CTP_ERROR_NULL_PTR;
  }
  return (get_two_byte_reg_data(ctp, CTP_REG_Y_CORD_RES, y_cord_res));
}

ctp_error_t ctp_get_product_id(const ctp_device_t* const ctp,
                               uint32_t* const product_id) {
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

ctp_error_t ctp_set_number_of_touch_points(ctp_device_t* const ctp,
                                           uint_least8_t touch_point_count) {
  if (ctp == NULL) {
    return CTP_ERROR_HANDLER_IS_NULL;
  }
  if (touch_point_count > CTP_MAX_TOUCH_POINTS) {
    return CTP_ERROR_EXCEED_MAX_TOUCH_COUNT;
  }
  if (touch_point_count == 0) {
    return CTP_ERROR_TOUCH_COUNT_ZERO;
  }

  ctp->max_touch_points = touch_point_count;
  return CTP_ERROR_NONE;
}

uint_least8_t ctp_get_set_number_of_touch_points(
    const ctp_device_t* const ctp) {
  if (ctp == NULL) {
    return 0;
  }
  return ctp->max_touch_points;
}

uint_least8_t ctp_get_touch_count(const ctp_device_t* const ctp) {
  if (ctp == NULL) {
    return 0;
  }

  return ctp->touch_list.count;
}

bool ctp_get_next_event(ctp_device_t* const ctp, ctp_touch_t* const pt) {
  if (ctp == NULL || pt == NULL) return false;

  // Loop through the touch list starting from the last saved index
  for (uint_least8_t i = ctp->event_iterator_idx; i < CTP_MAX_TOUCH_POINTS;
       i++) {
    ctp_touch_t* const slot = &ctp->touch_list.touch[i];

    if (slot->event != CTP_EVENT_NONE) {
      // 1. Copy the data to the user's buffer
      copy_touch(pt, slot);
      pt->event = slot->event;

      // 2. Clear the event in the internal list so it isn't returned again
      slot->event = CTP_EVENT_NONE;

      // 3. Update iterator for the next call
      ctp->event_iterator_idx = i + 1;
      return true;
    }
  }

  // Reset iterator for the next polling cycle
  ctp->event_iterator_idx = 0;
  return false;
}

ctp_error_t ctp_check_for_change(ctp_device_t* const ctp,
                                 bool* const has_change_occurred) {
  ctp_error_t err = validate_context(ctp);
  if (err != CTP_ERROR_NONE) {
    return err;
  }
  if (has_change_occurred == NULL) {
    return CTP_ERROR_NULL_PTR;
  }
  *has_change_occurred = false;
  ctp->event_iterator_idx = 0;

  // clear all previous events
  for (uint16_t i = 0; i < CTP_MAX_TOUCH_POINTS; i++) {
    if (ctp->touch_list.touch[i].event == CTP_EVENT_RELEASE) {
      ctp_clear_touch(&ctp->touch_list.touch[i]);
    } else {
      ctp->touch_list.touch[i].event = CTP_EVENT_NONE;
    }
  }

  if (has_isr_event_occurred(ctp) == false) {
    return CTP_ERROR_NONE;
  }

  bool has_new_data = 0;
  uint_least8_t touch_point_count = 0;
  err = ctp_read_status(ctp, &has_new_data, &touch_point_count);
  if ((err != CTP_ERROR_NONE) || (has_new_data == false)) {
    return err;
  }

  if (touch_point_count > CTP_MAX_TOUCH_POINTS) {
    touch_point_count = CTP_MAX_TOUCH_POINTS;
  }

  // device shifts all active touches down the registers block
  // so one only needs to read the first touch_point_count regs
  if (touch_point_count != 0) {
    err = i2c_read_reg(ctp, CTP_REG_TOUCH_DATA, ctp->rx_buf,
                       touch_point_count * CTP_BYTES_PER_TOUCH);
    if (err != CTP_ERROR_NONE) {
      return err;
    }
  }

  ctp->touch_list.count = touch_point_count;
  uint16_t touch_id_located_record = 0;
  for (uint16_t i = 0; i < touch_point_count; i++) {
    ctp_touch_t pt;
    parse_data_into_touch(&pt, &ctp->rx_buf[i * CTP_BYTES_PER_TOUCH]);

    uint_least8_t idx = pt.track_id;
    if ((idx >= ctp->max_touch_points) || !is_touch_valid(&pt) ||
        (idx >= CTP_MAX_TOUCH_POINTS)) {
      continue;
    }

    // make note of the track id that was read
    // this will be used later to clear released touches
    touch_id_located_record |= (1UL << idx);  // Use 1UL for unsigned long shift
    ctp_touch_t* const slot = &ctp->touch_list.touch[idx];

    if (!slot->is_touched) {
      // New Touch detected
      copy_touch(slot, &pt);
      slot->is_touched = true;
      slot->event = CTP_EVENT_START;
      *has_change_occurred = true;
    } else if (!are_touches_equal(slot, &pt)) {
      // Existing touch moved or changed
      copy_touch(slot, &pt);
      slot->event = CTP_EVENT_MOVE;
      *has_change_occurred = true;
    }
  }

  for (uint16_t i = 0; i < CTP_MAX_TOUCH_POINTS; i++) {
    ctp_touch_t* const slot = &ctp->touch_list.touch[i];
    // If the ID was not found in the current hardware record, it's a release
    if ((slot->is_touched) && ((touch_id_located_record & (1UL << i)) == 0)) {
      slot->is_touched = false;
      slot->event = CTP_EVENT_RELEASE;
      *has_change_occurred = true;
    }
  }

  return CTP_ERROR_NONE;
}

ctp_error_t ctp_enter_sleep(const ctp_device_t* const ctp) {
  ctp_error_t err = validate_context(ctp);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  hal_prepare_for_sleep(ctp->config->plat);
  uint_least8_t write_data = CTP_COMMAND_OFF_SCREEN;
  err = i2c_write_reg(ctp, CTP_REG_COMMAND, &write_data, 1);
  ctp->config->plat->gpio_set_input(ctp->config->plat->pin_int);

  return err;
}

ctp_error_t ctp_exit_sleep(const ctp_device_t* const ctp) {
  ctp_error_t err = validate_context(ctp);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  hal_wake_from_sleep(ctp->config->plat);
  return CTP_ERROR_NONE;
}

ctp_error_t ctp_initialize(ctp_device_t* const ctp,
                           const ctp_config_t* const config) {
  if (ctp == NULL) {
    return CTP_ERROR_HANDLER_IS_NULL;
  }
  if ((config == NULL) || (config->config_array == NULL) ||
      (config->config_array_length != CTP_CONFIG_REQUIRED_LEN)) {
    return CTP_ERROR_CONFIG_INVALID;
  }
  if (config->plat == NULL) {
    return CTP_ERROR_PLAT_IS_NULL;
  }

  ctp->config = config;
  ctp_error_t err =
      ctp_set_number_of_touch_points(ctp, config->default_max_touch_points);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  set_i2c_address(ctp, config->use_alt_i2c_addr);

  err = ctp_send_config(ctp);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  err = verify_config_data_chunk(ctp);
  if (err != CTP_ERROR_NONE) {
    return err;
  }

  ctp_clear_touch_event_list(&ctp->touch_list);
  ctp->is_initialized = true;
  return CTP_ERROR_NONE;
}
