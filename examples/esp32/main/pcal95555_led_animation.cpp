/**
 * @file pcal95555_led_animation.cpp
 * @brief LED animation demo for PCA9555/PCAL9555A GPIO expander on ESP32-S3
 *
 * This example drives 16 LEDs connected to all pins of a PCA9555/PCAL9555A
 * I/O expander through a series of visual animation patterns designed to
 * exercise and verify the driver's capabilities:
 *
 *   1. Sequential Chase     - Tests individual pin on/off control
 *   2. Bounce               - Tests bidirectional single-pin toggling
 *   3. Binary Counter       - Tests 16-bit port-wide write accuracy
 *   4. Breathing (PWM-like) - Tests rapid bit-banged toggling at high speed
 *   5. Wave                 - Tests multi-pin concurrent state management
 *   6. Random Sparkle       - Tests random-access single-pin writes
 *   7. Build-up / Teardown  - Tests cumulative pin state management
 *   8. Accelerating Scan    - Tests variable-speed I2C access from slow to fast
 *   9. All-on / All-off     - Tests bulk port writes
 *
 * Hardware setup:
 *   - PCA9555 or PCAL9555A at address 0x20 (A2=A1=A0=LOW)
 *   - LEDs with current-limiting resistors on pins IO0_0..IO1_7
 *   - LEDs are active-LOW (pin LOW = LED ON) or active-HIGH — configurable below
 *   - ESP32-S3 I2C: SDA=GPIO4, SCL=GPIO5
 *   - Address pins: A0=GPIO45, A1=GPIO48, A2=GPIO47
 *
 * @note This example uses only standard PCA9555 registers (0x00-0x07) so it
 *       works with both PCA9555 and PCAL9555A chips. The detected chip variant
 *       is logged at startup.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "esp32_pcal95555_bus.hpp"
#include "pcal95555.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdlib>
#include <memory>

#ifdef __cplusplus
extern "C" {
#endif
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#ifdef __cplusplus
}
#endif

using PCAL95555Driver = pcal95555::PCAL95555<Esp32Pcal9555I2cBus>;

static const char* g_TAG = "LED_Anim";

//=============================================================================
// CONFIGURATION
//=============================================================================

/// Set to true if LEDs are active-LOW (pin LOW = LED ON, common-anode).
/// Set to false if LEDs are active-HIGH (pin HIGH = LED ON, common-cathode).
static constexpr bool LEDS_ACTIVE_LOW = false;

/// Number of times to repeat each animation pattern
static constexpr int PATTERN_REPEATS = 2;

/// Delay (ms) between animation patterns
static constexpr int INTER_PATTERN_DELAY_MS = 500;

/// Total number of GPIO pins on the expander
static constexpr uint16_t NUM_PINS = 16;

// I2C address pin configuration
static constexpr bool A0_LEVEL = false;
static constexpr bool A1_LEVEL = false;
static constexpr bool A2_LEVEL = false;

//=============================================================================
// GLOBALS
//=============================================================================
static std::unique_ptr<Esp32Pcal9555I2cBus> g_bus;
static std::unique_ptr<PCAL95555Driver> g_driver;

//=============================================================================
// HELPERS
//=============================================================================

/// Write a 16-bit LED pattern (handles active-low inversion).
/// Writes both OUTPUT_PORT_0 and OUTPUT_PORT_1 registers in two fast I2C
/// transactions for maximum throughput.
static inline void set_leds(uint16_t pattern) {
  uint16_t hw = LEDS_ACTIVE_LOW ? static_cast<uint16_t>(~pattern) : pattern;
  uint8_t port0 = static_cast<uint8_t>(hw & 0xFF);
  uint8_t port1 = static_cast<uint8_t>((hw >> 8) & 0xFF);
  // Write OUTPUT_PORT_0 (0x02) and OUTPUT_PORT_1 (0x03) directly via bus
  uint8_t addr = g_driver->GetAddress();
  g_bus->Write(addr, 0x02, &port0, 1);
  g_bus->Write(addr, 0x03, &port1, 1);
}

/// Turn all LEDs off
static inline void all_off() {
  set_leds(0x0000);
}

/// Turn all LEDs on
static inline void all_on() {
  set_leds(0xFFFF);
}

/// Delay helper (ms)
static inline void delay_ms(uint32_t ms) {
  vTaskDelay(pdMS_TO_TICKS(ms));
}

/// Get a pseudo-random 32-bit value
static inline uint32_t random32() {
  return esp_random();
}

//=============================================================================
// ANIMATION PATTERNS
//=============================================================================

/**
 * @brief Pattern 1: Sequential Chase
 * One LED travels from pin 0 to pin 15 and back.
 * Tests: individual pin ON/OFF control in sequence.
 */
static void anim_sequential_chase(uint32_t speed_ms) {
  ESP_LOGI(g_TAG, "  Pattern: Sequential Chase (speed=%lu ms)", speed_ms);

  for (int rep = 0; rep < PATTERN_REPEATS; ++rep) {
    // Forward
    for (uint16_t i = 0; i < NUM_PINS; ++i) {
      set_leds(1U << i);
      delay_ms(speed_ms);
    }
    // Reverse
    for (int16_t i = NUM_PINS - 1; i >= 0; --i) {
      set_leds(1U << i);
      delay_ms(speed_ms);
    }
  }
  all_off();
}

/**
 * @brief Pattern 2: Bounce
 * A single lit LED bounces between pin 0 and pin 15.
 * Tests: bidirectional single-pin toggling with smooth motion.
 */
static void anim_bounce(uint32_t speed_ms) {
  ESP_LOGI(g_TAG, "  Pattern: Bounce (speed=%lu ms)", speed_ms);

  for (int rep = 0; rep < PATTERN_REPEATS * 3; ++rep) {
    // Forward
    for (uint16_t i = 0; i < NUM_PINS; ++i) {
      set_leds(1U << i);
      delay_ms(speed_ms);
    }
    // Backward (skip endpoints to avoid double-flash)
    for (int16_t i = NUM_PINS - 2; i > 0; --i) {
      set_leds(1U << i);
      delay_ms(speed_ms);
    }
  }
  all_off();
}

/**
 * @brief Pattern 3: Binary Counter
 * Count from 0 to 65535 in steps, displaying the binary value on LEDs.
 * Tests: 16-bit port-wide write accuracy across all pin combinations.
 */
static void anim_binary_counter(uint32_t speed_ms) {
  ESP_LOGI(g_TAG, "  Pattern: Binary Counter (speed=%lu ms)", speed_ms);

  // Count up in larger steps for visual effect
  uint16_t step = 1;
  if (speed_ms < 10) step = 256;
  else if (speed_ms < 30) step = 64;
  else if (speed_ms < 60) step = 16;
  else step = 4;

  for (uint32_t val = 0; val <= 0xFFFF; val += step) {
    set_leds(static_cast<uint16_t>(val));
    delay_ms(speed_ms);
  }

  // Flash all on then off
  all_on();
  delay_ms(200);
  all_off();
}

/**
 * @brief Pattern 4: Software PWM Breathing
 * Simulate brightness ramping by varying the on/off duty cycle with
 * rapid bit-bang toggling. This hammers the I2C bus at maximum speed.
 * Tests: rapid I2C write throughput, pseudo-PWM timing.
 */
static void anim_breathing(uint32_t cycle_ms) {
  ESP_LOGI(g_TAG, "  Pattern: Breathing / Software PWM (cycle=%lu ms)", cycle_ms);

  // Total steps in one fade-in or fade-out
  constexpr int PWM_STEPS = 20;
  // Number of sub-cycles per step to make the effect visible
  constexpr int SUB_CYCLES = 3;

  for (int rep = 0; rep < PATTERN_REPEATS; ++rep) {
    // Fade in (increase duty)
    for (int step = 0; step <= PWM_STEPS; ++step) {
      for (int sub = 0; sub < SUB_CYCLES; ++sub) {
        // ON phase
        all_on();
        // Proportional delay: step/PWM_STEPS of cycle time
        uint32_t on_time = (cycle_ms * step) / (PWM_STEPS * SUB_CYCLES);
        if (on_time > 0) delay_ms(on_time);

        // OFF phase
        all_off();
        uint32_t off_time = (cycle_ms * (PWM_STEPS - step)) / (PWM_STEPS * SUB_CYCLES);
        if (off_time > 0) delay_ms(off_time);
      }
    }

    // Fade out (decrease duty)
    for (int step = PWM_STEPS; step >= 0; --step) {
      for (int sub = 0; sub < SUB_CYCLES; ++sub) {
        all_on();
        uint32_t on_time = (cycle_ms * step) / (PWM_STEPS * SUB_CYCLES);
        if (on_time > 0) delay_ms(on_time);

        all_off();
        uint32_t off_time = (cycle_ms * (PWM_STEPS - step)) / (PWM_STEPS * SUB_CYCLES);
        if (off_time > 0) delay_ms(off_time);
      }
    }
  }
  all_off();
}

/**
 * @brief Pattern 5: Wave / Comet Tail
 * A 4-LED "comet" sweeps across with a fading tail.
 * Tests: multi-pin concurrent state management.
 */
static void anim_wave(uint32_t speed_ms) {
  ESP_LOGI(g_TAG, "  Pattern: Wave / Comet Tail (speed=%lu ms)", speed_ms);

  constexpr int TAIL_LENGTH = 4;

  for (int rep = 0; rep < PATTERN_REPEATS; ++rep) {
    // Forward sweep
    for (int head = 0; head < NUM_PINS + TAIL_LENGTH; ++head) {
      uint16_t pattern = 0;
      for (int t = 0; t < TAIL_LENGTH; ++t) {
        int pin = head - t;
        if (pin >= 0 && pin < NUM_PINS) {
          pattern |= (1U << pin);
        }
      }
      set_leds(pattern);
      delay_ms(speed_ms);
    }

    // Reverse sweep
    for (int head = NUM_PINS - 1; head >= -TAIL_LENGTH; --head) {
      uint16_t pattern = 0;
      for (int t = 0; t < TAIL_LENGTH; ++t) {
        int pin = head + t;
        if (pin >= 0 && pin < NUM_PINS) {
          pattern |= (1U << pin);
        }
      }
      set_leds(pattern);
      delay_ms(speed_ms);
    }
  }
  all_off();
}

/**
 * @brief Pattern 6: Random Sparkle
 * Random pins flash on and off rapidly.
 * Tests: random-access single-pin writes, write throughput.
 */
static void anim_sparkle(uint32_t speed_ms, uint32_t duration_ms) {
  ESP_LOGI(g_TAG, "  Pattern: Random Sparkle (speed=%lu ms, duration=%lu ms)", speed_ms, duration_ms);

  int64_t start = esp_timer_get_time();
  int64_t end = start + (int64_t)duration_ms * 1000;

  while (esp_timer_get_time() < end) {
    uint16_t pattern = static_cast<uint16_t>(random32() & 0xFFFF);
    set_leds(pattern);
    delay_ms(speed_ms);
  }
  all_off();
}

/**
 * @brief Pattern 7: Build-up and Teardown
 * LEDs light up one by one until all are on, then turn off one by one.
 * Tests: cumulative pin state management.
 */
static void anim_buildup_teardown(uint32_t speed_ms) {
  ESP_LOGI(g_TAG, "  Pattern: Build-up / Teardown (speed=%lu ms)", speed_ms);

  for (int rep = 0; rep < PATTERN_REPEATS; ++rep) {
    uint16_t pattern = 0;
    // Build up
    for (uint16_t i = 0; i < NUM_PINS; ++i) {
      pattern |= (1U << i);
      set_leds(pattern);
      delay_ms(speed_ms);
    }

    delay_ms(speed_ms * 2); // Hold all-on briefly

    // Tear down
    for (uint16_t i = 0; i < NUM_PINS; ++i) {
      pattern &= ~(1U << i);
      set_leds(pattern);
      delay_ms(speed_ms);
    }

    delay_ms(speed_ms);
  }
  all_off();
}

/**
 * @brief Pattern 8: Accelerating / Decelerating Scan
 * A single LED scans back and forth, starting slow and accelerating
 * to maximum I2C speed, then decelerating back to slow.
 * Tests: variable-speed I2C access, bus stability at high speeds.
 */
static void anim_accel_scan() {
  ESP_LOGI(g_TAG, "  Pattern: Accelerating Scan");

  // Speed schedule: start slow, ramp up, then ramp back down
  static constexpr uint32_t speeds[] = {
    120, 100, 80, 60, 50, 40, 30, 25, 20, 15, 12, 10,
    8, 6, 5, 4, 3, 2, 1, 1, 1, 1, 1, 1,
    2, 3, 4, 5, 6, 8, 10, 12, 15, 20, 25, 30,
    40, 50, 60, 80, 100, 120
  };
  constexpr int num_speeds = sizeof(speeds) / sizeof(speeds[0]);

  for (int s = 0; s < num_speeds; ++s) {
    // One full forward scan at this speed
    for (uint16_t i = 0; i < NUM_PINS; ++i) {
      set_leds(1U << i);
      delay_ms(speeds[s]);
    }
  }
  all_off();
}

/**
 * @brief Pattern 9: Center-out Expand / Contract
 * LEDs light from the center outward, then contract back.
 * Tests: symmetric pin addressing across both ports.
 */
static void anim_center_expand(uint32_t speed_ms) {
  ESP_LOGI(g_TAG, "  Pattern: Center Expand / Contract (speed=%lu ms)", speed_ms);

  for (int rep = 0; rep < PATTERN_REPEATS; ++rep) {
    // Expand from center
    uint16_t pattern = 0;
    for (int radius = 0; radius < 8; ++radius) {
      int left = 7 - radius;
      int right = 8 + radius;
      pattern |= (1U << left) | (1U << right);
      set_leds(pattern);
      delay_ms(speed_ms);
    }

    delay_ms(speed_ms * 3); // Hold full

    // Contract to center
    for (int radius = 7; radius >= 0; --radius) {
      int left = 7 - radius;
      int right = 8 + radius;
      pattern &= ~((1U << left) | (1U << right));
      set_leds(pattern);
      delay_ms(speed_ms);
    }

    delay_ms(speed_ms);
  }
  all_off();
}

/**
 * @brief Pattern 10: Alternating Halves Flash
 * Port 0 and Port 1 flash alternately, then even/odd pins alternate.
 * Tests: port-level writes, alternating patterns.
 */
static void anim_alternating_flash(uint32_t speed_ms) {
  ESP_LOGI(g_TAG, "  Pattern: Alternating Flash (speed=%lu ms)", speed_ms);

  // Port 0 vs Port 1
  for (int rep = 0; rep < PATTERN_REPEATS * 4; ++rep) {
    set_leds(0x00FF); // Port 0 on, Port 1 off
    delay_ms(speed_ms);
    set_leds(0xFF00); // Port 1 on, Port 0 off
    delay_ms(speed_ms);
  }

  // Even vs Odd pins
  for (int rep = 0; rep < PATTERN_REPEATS * 4; ++rep) {
    set_leds(0x5555); // Even pins
    delay_ms(speed_ms);
    set_leds(0xAAAA); // Odd pins
    delay_ms(speed_ms);
  }

  all_off();
}

//=============================================================================
// INITIALIZATION
//=============================================================================

static bool init_hardware() {
  ESP_LOGI(g_TAG, "Initializing I2C bus...");

  Esp32Pcal9555I2cBus::I2CConfig config;
  config.port = I2C_NUM_0;
  config.sda_pin = GPIO_NUM_4;
  config.scl_pin = GPIO_NUM_5;
  config.frequency = 400000;
  config.pullup_enable = true;
  config.a0_pin = GPIO_NUM_45;
  config.a1_pin = GPIO_NUM_48;
  config.a2_pin = GPIO_NUM_47;

  g_bus = CreateEsp32Pcal9555I2cBus(config);
  if (!g_bus || !g_bus->IsInitialized()) {
    ESP_LOGE(g_TAG, "Failed to initialize I2C bus");
    return false;
  }
  ESP_LOGI(g_TAG, "I2C bus initialized (SDA=GPIO%d, SCL=GPIO%d, %lu Hz)",
           config.sda_pin, config.scl_pin, config.frequency);

  ESP_LOGI(g_TAG, "Initializing PCA9555/PCAL9555A driver...");
  g_driver = std::make_unique<PCAL95555Driver>(g_bus.get(), A0_LEVEL, A1_LEVEL, A2_LEVEL);
  if (!g_driver) {
    ESP_LOGE(g_TAG, "Failed to create driver");
    return false;
  }

  // Force initialization
  if (!g_driver->EnsureInitialized()) {
    ESP_LOGE(g_TAG, "Driver initialization failed");
    return false;
  }

  // Log detected chip variant
  auto variant = g_driver->GetChipVariant();
  const char* variant_name = "Unknown";
  if (variant == pcal95555::ChipVariant::PCA9555) {
    variant_name = "PCA9555 (standard)";
  } else if (variant == pcal95555::ChipVariant::PCAL9555A) {
    variant_name = "PCAL9555A (Agile I/O)";
  }
  ESP_LOGI(g_TAG, "Chip variant: %s", variant_name);
  ESP_LOGI(g_TAG, "I2C address: 0x%02X", g_driver->GetAddress());

  // Configure all 16 pins as outputs
  for (uint16_t pin = 0; pin < NUM_PINS; ++pin) {
    if (!g_driver->SetPinDirection(pin, GPIODir::Output)) {
      ESP_LOGE(g_TAG, "Failed to set pin %d as output", pin);
      return false;
    }
  }
  ESP_LOGI(g_TAG, "All 16 pins configured as outputs");

  // Start with all LEDs off
  all_off();

  // Clear any accumulated error flags from init
  g_driver->ClearErrorFlags();

  return true;
}

//=============================================================================
// MAIN
//=============================================================================

extern "C" void app_main(void) {
  ESP_LOGI(g_TAG, "╔══════════════════════════════════════════════════════════════╗");
  ESP_LOGI(g_TAG, "║        PCA9555 / PCAL9555A  LED Animation Demo             ║");
  ESP_LOGI(g_TAG, "║              HardFOC GPIO Expander Driver                   ║");
  ESP_LOGI(g_TAG, "╚══════════════════════════════════════════════════════════════╝");

  delay_ms(500);

  if (!init_hardware()) {
    ESP_LOGE(g_TAG, "Hardware initialization failed. Halting.");
    while (true) { delay_ms(1000); }
  }

  ESP_LOGI(g_TAG, "");
  ESP_LOGI(g_TAG, "Starting LED animation loop (LEDs %s)...",
           LEDS_ACTIVE_LOW ? "active-LOW" : "active-HIGH");
  ESP_LOGI(g_TAG, "");

  int cycle = 0;

  while (true) {
    cycle++;
    ESP_LOGI(g_TAG, "========== Animation Cycle %d ==========", cycle);

    // --- 1. Sequential Chase ---
    ESP_LOGI(g_TAG, "[1/10] Sequential Chase");
    anim_sequential_chase(60);
    delay_ms(INTER_PATTERN_DELAY_MS);

    // --- 2. Bounce ---
    ESP_LOGI(g_TAG, "[2/10] Bounce");
    anim_bounce(40);
    delay_ms(INTER_PATTERN_DELAY_MS);

    // --- 3. Binary Counter ---
    ESP_LOGI(g_TAG, "[3/10] Binary Counter");
    anim_binary_counter(5);
    delay_ms(INTER_PATTERN_DELAY_MS);

    // --- 4. Breathing (software PWM) ---
    ESP_LOGI(g_TAG, "[4/10] Breathing (software PWM)");
    anim_breathing(40);
    delay_ms(INTER_PATTERN_DELAY_MS);

    // --- 5. Wave / Comet Tail ---
    ESP_LOGI(g_TAG, "[5/10] Wave / Comet Tail");
    anim_wave(50);
    delay_ms(INTER_PATTERN_DELAY_MS);

    // --- 6. Random Sparkle ---
    ESP_LOGI(g_TAG, "[6/10] Random Sparkle");
    anim_sparkle(30, 3000);
    delay_ms(INTER_PATTERN_DELAY_MS);

    // --- 7. Build-up / Teardown ---
    ESP_LOGI(g_TAG, "[7/10] Build-up / Teardown");
    anim_buildup_teardown(80);
    delay_ms(INTER_PATTERN_DELAY_MS);

    // --- 8. Accelerating Scan ---
    ESP_LOGI(g_TAG, "[8/10] Accelerating Scan");
    anim_accel_scan();
    delay_ms(INTER_PATTERN_DELAY_MS);

    // --- 9. Center Expand / Contract ---
    ESP_LOGI(g_TAG, "[9/10] Center Expand / Contract");
    anim_center_expand(80);
    delay_ms(INTER_PATTERN_DELAY_MS);

    // --- 10. Alternating Flash ---
    ESP_LOGI(g_TAG, "[10/10] Alternating Flash");
    anim_alternating_flash(100);
    delay_ms(INTER_PATTERN_DELAY_MS);

    // --- Finale: fast all-on / all-off strobe ---
    ESP_LOGI(g_TAG, "Finale: Strobe");
    for (int i = 0; i < 10; ++i) {
      all_on();
      delay_ms(50);
      all_off();
      delay_ms(50);
    }

    // Check driver health
    uint16_t errors = g_driver->GetErrorFlags();
    if (errors != 0) {
      ESP_LOGW(g_TAG, "Driver error flags after cycle %d: 0x%04X", cycle, errors);
      g_driver->ClearErrorFlags();
    } else {
      ESP_LOGI(g_TAG, "Cycle %d complete - no errors", cycle);
    }

    ESP_LOGI(g_TAG, "");
    delay_ms(2000); // Pause between cycles
  }
}
