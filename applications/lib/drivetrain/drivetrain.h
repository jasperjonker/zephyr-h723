#ifndef DRIVETRAIN_H_
#define DRIVETRAIN_H_

#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <stdlib.h>

// ------------------------------------------------
// Devicetree config macros
// ------------------------------------------------

/* PWM Servo at PA6 */
static const struct pwm_dt_spec servo = PWM_DT_SPEC_GET(DT_ALIAS(servo0));
static const uint32_t min_pulse = DT_PROP(DT_ALIAS(servo0), min_pulse);
static const uint32_t max_pulse = DT_PROP(DT_ALIAS(servo0), max_pulse);

// ------------------------------------------------
// Parameters
// ------------------------------------------------
static const uint32_t ramp_step = PWM_USEC(1);        // pwm ticks
static const uint32_t ramp_delay_ms = 10;             // delay between steps in ms

// ------------------------------------------------
// Enumerations
// ------------------------------------------------
enum ramp_direction {
    RAMP_UP,
    RAMP_DOWN
};

// ------------------------------------------------
// Function prototypes
// ------------------------------------------------
int8_t init_drivetrain(void);
int8_t set_pulse_width_drivetrain(uint32_t pulse_width);
void ramp_pwm_drivetrain(const uint32_t pulse_width_start, const uint32_t pulse_width_end);
int8_t cli_ramp_pwm_drivetrain(const struct shell *sh, size_t argc, char **argv);

#endif /* DRIVETRAIN_H_ */
