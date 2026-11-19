#ifndef CTP_DEFAULT_CONFIG_H
#define CTP_DEFAULT_CONFIG_H

#include <stdint.h>

/** @brief Default manufacturer configuration array stored in Flash.
 * @details The configuration array is typically provided by the manufacturer for a specific LCD.
 * This default config is for an 800x480 array with a slight increase in noise resistance.
 * By referencing the datasheet, it is possible to adjust the array to suite your needs if the manufacurer's is insufficient.
 */
extern const uint_least8_t g_ctp_default_config[];

/** @brief Length of the default manufacturer configuration array in bytes. */
extern const uint16_t g_ctp_default_config_len;

#endif // CTP_DEFAULT_CONFIG_H
