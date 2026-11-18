#ifndef CTP_REGISTERS_H
#define CTP_REGISTERS_H

/* Registers from GT9271 Rev.00 */
#define CTP_REG_COMMAND 0x8040
#define CTP_REG_ESD_CHECK 0x8041
#define CTP_REG_CONFIG_DATA 0x8047  // Start of config block
#define CTP_REG_CONFIG_FRESH 0x8100 // End of config block

#define CTP_REG_PRODUCT_ID 0x8140
#define CTP_REG_FW_VERSION 0x8144
#define CTP_REG_X_CORD_RES 0x8146
#define CTP_REG_Y_CORD_RES 0x8148
#define CTP_REG_VENDOR_ID 0x814A

#define CTP_REG_STATUS 0x814E
#define CTP_REG_TOUCH_DATA 0x814F
#define CTP_REG_KEY_VALUE 0x819F

/* Register byte counts */
#define CTP_REG_PRODUCT_ID_SIZE 4
#define CTP_REG_FW_VERSION_SIZE 2
#define CTP_REG_X_CORD_RES_SIZE 2
#define CTP_REG_Y_CORD_RES_SIZE 2

/* 0x814E status bits */
#define CTP_STATUS_DATA_FLAG 0x80
#define CTP_STATUS_LARGE_DETECT 0x40
#define CTP_STATUS_HAVE_KEY 0x10
#define CTP_STATUS_TOUCH_MASK 0x0F

/* 0x8040 command values */
#define CTP_COMMAND_OFF_SCREEN (0x05U)

/** 0x814E command values */
#define CTP_STATUS_REG_CLEAR (0x00U)

/*
 * GT9271 touch point layout at 0x814F.
 *
 * 0-based offsets:
 *   0 = track id
 *   1 = x low
 *   2 = x high
 *   3 = y low
 *   4 = y high
 *   5 = size low
 *   6 = size high
 *   7 = reserved
 */
#define CTP_TOUCH_DATA_TRACK_ID_OFFSET 0
#define CTP_TOUCH_DATA_X_LOW_OFFSET 1
#define CTP_TOUCH_DATA_X_HIGH_OFFSET 2
#define CTP_TOUCH_DATA_Y_LOW_OFFSET 3
#define CTP_TOUCH_DATA_Y_HIGH_OFFSET 4
#define CTP_TOUCH_DATA_SIZE_LOW_OFFSET 5
#define CTP_TOUCH_DATA_SIZE_HIGH_OFFSET 6
#define CTP_TOUCH_DATA_RESERVED_OFFSET 7

/* Status register bits */
#define CTP_STATUS_DATA_FLAG 0x80
#define CTP_STATUS_TOUCH_MASK 0x0F
#define CTP_STATUS_KEY_MASK 0x70

#endif // CTP_REGISTERS_H
