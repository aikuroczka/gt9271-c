/**
 * @file arduino_basic.cpp
 * @brief Basic integration example for the GT9271 driver on Arduino.
 * @details This example demonstrates how to implement the PAL and use the
 * event-driven API to track touch points in real-time.
 */

#include <Arduino.h>
#include <Wire.h>
#include <gt9271/ctp.h>
#include <gt9271/ctp_default_config.h>
#include <gt9271/ctp_version.h>

/* --- Hardware Configuration --- */
#define CTP_INTERRUPT_PIN (3U)  // Do NOT place external pull-up
#define CTP_RESET_PIN (2U)      // Requires 10kOhm external pull-up

/* ==========================================================================
   PLATFORM ABSTRACTION LAYER (PAL) IMPLEMENTATION
   ========================================================================== */

void platform_delay_us(const uint32_t delay_us) { delayMicroseconds(delay_us); }

bool platform_read(const uint16_t pin) { return (digitalRead(pin) == HIGH); }

void set_pin_input(const uint16_t pin) { pinMode(pin, INPUT); }

void set_pin_output(const uint16_t pin) { pinMode(pin, OUTPUT); }

void platform_write(const uint16_t pin, const bool is_high) {
  digitalWrite(pin, is_high ? HIGH : LOW);
}

bool i2c_write(const uint16_t addr, const uint_least8_t* const reg_buf,
               const uint16_t reg_bug_len, const uint_least8_t* const data_buf,
               const uint16_t data_but_len) {
  Wire.beginTransmission((int)addr);
  for (uint16_t i = 0; i < reg_bug_len; i++) {
    Wire.write(reg_buf[i]);
  }
  for (uint16_t i = 0; i < data_but_len; i++) {
    Wire.write(data_buf[i]);
  }

  int error = Wire.endTransmission();
  if (error == 0) return true;

  // Error logging for debugging hardware issues
  if (error == 2)
    Serial.println("I2C: NACK on Address");
  else if (error == 3)
    Serial.println("I2C: NACK on Data");
  else
    Serial.print("I2C Error: ");
  Serial.println(error);

  return false;
}

bool i2c_read(const uint16_t addr, const uint_least8_t* const tx_reg_buf,
              const uint16_t tx_reg_buf_len, uint_least8_t* const rx_buf,
              const uint16_t rx_len) {
  Wire.beginTransmission((int)addr);
  for (uint16_t i = 0; i < tx_reg_buf_len; i++) {
    Wire.write(tx_reg_buf[i]);
  }
  if (Wire.endTransmission() != 0) return false;

  const uint8_t MAX_WIRE_BUFFER = 32;
  uint8_t bytes_read_total = 0;

  while (bytes_read_total < rx_len) {
    uint8_t remaining = rx_len - bytes_read_total;
    int chunk_size =
        (remaining > MAX_WIRE_BUFFER) ? MAX_WIRE_BUFFER : (int)remaining;

    uint8_t bytes_received = Wire.requestFrom((int)addr, chunk_size);
    if (bytes_received == 0) return false;

    for (uint8_t i = 0; i < bytes_received; i++) {
      rx_buf[bytes_read_total + i] = Wire.read();
    }
    bytes_read_total += bytes_received;
  }
  return true;
}

/* ==========================================================================
   DRIVER CONFIGURATION
   ========================================================================== */

volatile bool g_has_ctp_isr_triggered = false;
ctp_device_t g_ctp;

const ctp_platform_interface_t g_ctp_plat = {.gpio_set_input = set_pin_input,
                                             .gpio_set_output = set_pin_output,
                                             .i2c_write = i2c_write,
                                             .i2c_read = i2c_read,
                                             .gpio_write = platform_write,
                                             .gpio_read = platform_read,
                                             .delay_usec = platform_delay_us,
                                             .pin_reset = CTP_RESET_PIN,
                                             .pin_int = CTP_INTERRUPT_PIN};

const ctp_config_t g_ctp_config = {
    .plat = &g_ctp_plat,
    .config_array = g_ctp_default_config,
    .config_array_length = g_ctp_default_config_len,
    .isr_flag = &g_has_ctp_isr_triggered,
    .default_max_touch_points = 10};

void ctp_interrupt() { g_has_ctp_isr_triggered = true; }

/* ==========================================================================
   MAIN APPLICATION
   ========================================================================== */

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(400000);  // High-speed I2C for reduced latency

  Serial.print("CTP Driver Version: ");
  Serial.println(CTP_VERSION);

  ctp_error_t err = ctp_initialize(&g_ctp, &g_ctp_config);
  while (err != CTP_ERROR_NONE) {
    Serial.print("Init Fail: ");
    Serial.println(err, DEC);
    delay(1000);
    err = ctp_initialize(&g_ctp, &g_ctp_config);
  }

  attachInterrupt(digitalPinToInterrupt(CTP_INTERRUPT_PIN), ctp_interrupt,
                  RISING);
  Serial.print("Device Address: 0x");
  Serial.println(g_ctp.i2c_addr, HEX);
}

void ctp_service() {
  bool has_change = false;
  if (ctp_check_for_change(&g_ctp, &has_change) != CTP_ERROR_NONE) return;

  if (has_change) {
    Serial.print("Active Touches: ");
    Serial.println(g_ctp.touch_list.count);

    ctp_touch_t pt;
    while (ctp_get_next_event(&g_ctp, &pt)) {
      Serial.print("ID ");
      Serial.print(pt.track_id);
      Serial.print(" [");
      Serial.print(pt.x);
      Serial.print(",");
      Serial.print(pt.y);
      Serial.print("] ");

      switch (pt.event) {
        case CTP_EVENT_START:
          Serial.print("TOUCHED");
          break;
        case CTP_EVENT_MOVE:
          Serial.print("MOVED");
          break;
        case CTP_EVENT_RELEASE:
          Serial.print("RELEASED");
          break;
        default:
          Serial.print("UNKNOWN");
          break;
      }
      Serial.print(" Size: ");
      Serial.println(pt.size);
    }
  }
}

uint64_t time_sum = 0;
uint32_t counter = 0;

void loop() {
  uint64_t start_time = micros();
  ctp_service();
  time_sum += (micros() - start_time);

  if (++counter >= 1000) {
    // Cast to double to ensure floating point division for precision
    double time_avg = (double)time_sum / 1000.0;
    Serial.print("Avg Processing Time: ");
    Serial.print(time_avg, 2);
    Serial.println(" us");
    time_sum = 0;
    counter = 0;
  }
  delay(1);
}
