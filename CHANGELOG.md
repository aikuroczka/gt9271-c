## 0.1.0 (2026-11-20)

### Feat

- **api**: Remodeled API to use iterator for event retrieval instead of state probing
- **core**: move from init sanity check using product id to functional verification using RAW for config
- **api**: added CTP_ERROR_COMM_TEST_FAIL as error return to deliniated between I2C failure and test failure

### Fix

- **core**: failure to set is_initilized early enough was resulting in always fail for initialize
- **core**: test_comms will now return exact i2c error before product id value check
- **api**: corrected type mismatch between config_array_length and g_ctp_default_config_len

### Refactor

- **core**: significant changed to primary detection algorithm, addition of private hal module, and documentation
- **core**: touch related functions moved to their own private module for proper deliniation
- **pal**: increased const correctness, documentation, and swap of delay_msec for delay_usec
- **api**: default config type promoted to uint16_t to accomodate edge case config array exceeds 255 bytes
- **api**: platform is moved to bottom of include hierarchy and types will now include it
- **api**: addition of config and adjustments such that ctp does not require any member definitions
- **pal**: remove wake pin definition from platform interface
- **pal**: remove I2C pin definitions from device handler to enforce PAL encapsulation
