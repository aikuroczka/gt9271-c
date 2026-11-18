#ifndef CTP_DEFS_H
#define CTP_DEFS_H

/** @brief Required configuration length based on register range 0x8047 to
 * 0x8100 */
#define CTP_CONFIG_REQUIRED_LEN ((uint16_t)186)

/* Screen geometry */
#define CTP_MAX_X 800
#define CTP_MAX_Y 480

/* timing constants */
#define CTP_CONFIG_RETRY_COUNT 5
#define CTP_I2C_RETRY_DELAY_MS 10
#define CTP_RESET_LOW_DELAY_MS 10
#define CTP_ADDRESS_HOLD_DELAY_MS 75
#define CTP_POST_RESET_DELAY_MS 25

/* Touch limits */
#define CTP_MAX_TOUCH_POINTS                                                   \
  (10U) ///< Maximum number of touch points supported by device
#define CTP_BYTES_PER_TOUCH (8U)
#define CTP_MAX_TOUCH_DATA_BYTES (CTP_ABSOLUTE_MAX_TOUCH * CTP_BYTES_PER_TOUCH)

#define CTP_MIN_TOUCH_SIZE 5
#define CTP_MAX_TOUCH_SIZE 100

/* I2C addresses */
#define CTP_I2C_ADDR_DEFAULT 0xBA
#define CTP_I2C_ADDR_ALT 0x28

#endif // CTP_DEFS_H
