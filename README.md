# GT9271 High-Integrity Touch Driver

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/Language-C11-orange.svg)]()
[![Build](https://img.shields.io/badge/Build-CMake-green.svg)]()

A professional, hardware-agnostic driver for the Goodix GT9271 10-point capacitive touch controller. This library is designed for embedded systems where reliability, predictability, and portability are critical.

## Overview

The `gt9271_driver` provides a robust interface for interacting with the GT9271 touch panel. This implementation focuses on **high-integrity software engineering**, ensuring that the driver is resilient to hardware noise, portable across different MCUs, and easy to integrate into professional firmware.

## Engineering Philosophy

This project is built upon a set of strict design pillars to ensure production-grade reliability:

*   **Zero Dynamic Allocation:** No `malloc` or `free` is used. All memory is statically allocated at compile time, eliminating heap fragmentation and ensuring deterministic memory usage.
*   **Immutability & Memory Safety:** Extensive use of `const` correctness and a strict avoidance of pointer arithmetic to prevent accidental state corruption and buffer overflows.
*   **Strict Typing:** Use of fixed-width types (uint16_t, etc.) and "least" types (uint_least8_t) ensures predictable memory footprints across different CPU architectures.
*   **Hardware Agnosticism (PAL):** The driver is decoupled from the hardware via a **Platform Abstraction Layer (PAL)**. Porting to a new MCU requires only implementing a few simple callbacks for I2C and GPIO.
*   **Event-Driven Architecture:** Instead of manual register polling, the driver implements an **Event Iterator**. Users are notified of `START`, `MOVE`, and `RELEASE` events, reducing CPU overhead and simplifying application logic.
*   **Defensive State Management:** Validates its inputs and internal state at every API entry point to ensure resilience against undefined behavior.
*   **Reentrant:** No static or global data is used granting ability for concurrant device operation without state corruption.

## Quick Start

### Integration

* Add the `include` and `src` directories to your project. 
* If using CMake, you can simply use `add_subdirectory()`.
* If using PlatformIO, add to lib_deps `GT9271 = git+URL_TO_REPO`

### Basic Usage
```c
// 1. Add includes
#include <gt9271/ctp.h>
#include <gt9271/ctp_default_config.h> // Required only if using g_ctp_default_config and g_ctp_default_config_len as device config array

// Optional. Setup ISR routine and interrupt variable
volatile bool g_has_ctp_isr_triggered = false;
void ctp_interrupt() { 
    g_has_ctp_isr_triggered = true; 
}

// 2. Define the platform callbacks (PAL)
const ctp_platform_interface_t my_plat = {
    .i2c_read = my_platform_i2c_read,
    .i2c_write = my_platform_i2c_write,
    // ... other callbacks ...
};

// 3. Configure the device
ctp_config_t config = {
    .plat = &my_plat,
    .config_array = g_ctp_default_config,
    .config_array_length = g_ctp_default_config_len,
    .isr_flag = &my_interrupt_flag, // Set to NULL if not using
    .default_max_touch_points = 10,
};

ctp_device_t ctp;
ctp_error_t err = ctp_initialize(&ctp, &config);
if(err != CTP_ERROR_NONE) {
    // If device fails to initialize
    // Check return value to determine if hardware or software related
    // This must be resolved before usage
}
// Otherwise device is now ready

// Host can now setup interrupt pin to trigger and ISR after initialize

// 4. Main Loop
while (1) {
    bool changed = false;
    if (ctp_check_for_change(&ctp, &changed)) {
        // Handle errors
    }

    if (changed) {
        ctp_touch_t pt;
        while (ctp_get_next_event(&ctp, &pt)) {
            // Process event: pt.event (START/MOVE/RELEASE), pt.x, pt.y
        }
    }
}

```

## Build System

The project uses CMake for a portable build process. A build script has been provided to leverage multi-core building. To verify the library on your host machine:

```bash
chmod +x build.sh
./build.sh
```
*Note: The `build.sh` script performs a full-clean build every time*

## Documentation
Full API documentation is generated via Doxygen. 

## License
This project is licensed under the **Apache License 2.0**. See the `LICENSE` file for details.