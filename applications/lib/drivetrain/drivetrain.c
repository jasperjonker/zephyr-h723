#include "drivetrain.h"

/* LOG Module */
LOG_MODULE_REGISTER(pwm);


// ------------------------------------------------
// Initialize PWM Drivetrain
// ------------------------------------------------
int8_t init_drivetrain(void) {
    if (!device_is_ready(servo.dev)) {
        LOG_ERR("Cannot find PWM servo device!\n");
        return -1;
    } else {
        LOG_INF("PWM servo device found: %s\r\n", servo.dev->name);
    }

    if (set_pulse_width_drivetrain(PWM_USEC(1500))) {
        return -1;
    }

    return 0;
}

// ------------------------------------------------
// Set the pulse width of the PWM drivetrain (duty cycle)
// ------------------------------------------------
int8_t set_pulse_width_drivetrain(uint32_t pulse_width) {
    // Make sure pulse width is within bounds
    if (pulse_width < min_pulse || pulse_width > max_pulse) {
        LOG_ERR("Error: pulse width %d us is out of bounds [%d, %d] us\r\n", pulse_width / 1000, min_pulse / 1000, max_pulse / 1000);
        return -1;
    }

    uint8_t ret = pwm_set_pulse_dt(&servo, pulse_width);

    if (ret < 0) {
        LOG_ERR("Error %d: failed to set pulse width to %d us\r\n", ret, pulse_width / 1000);
    } else {
        LOG_DBG("PWM set to %d us\r\n", pulse_width / 1000);
    }
    return ret;
}

// ------------------------------------------------
// Ramp the PWM from pulse_width_start to
// pulse_width_end.
// ------------------------------------------------

void ramp_pwm_drivetrain(const uint32_t pulse_width_start, const uint32_t pulse_width_end) {
    LOG_INF("Ramping PWM from %dus to %dus...", pulse_width_start / 1000, pulse_width_end / 1000);

    uint32_t pulse_width = pulse_width_start;
    enum ramp_direction direction = pulse_width_end >= pulse_width_start ? RAMP_UP : RAMP_DOWN;

    while ((direction == RAMP_UP && pulse_width < pulse_width_end) ||
            (direction == RAMP_DOWN && pulse_width > pulse_width_end)) {

        if (set_pulse_width_drivetrain(pulse_width)) {
            // Failed to set pulse width
            LOG_ERR("Failed to set pulse width. Cancel ramping.");
            return;
        }

        if (direction == RAMP_UP) {
            pulse_width += ramp_step;
        } else {
            pulse_width -= ramp_step;
        }

        k_sleep(K_MSEC(ramp_delay_ms));
    }
    LOG_INF("Ramping PWM from %dus to %dus...[DONE]", pulse_width_start / 1000, pulse_width_end / 1000);
}

// ------------------------------------------------
// Command line functions
// ------------------------------------------------

int8_t cli_ramp_pwm_drivetrain(const struct shell *sh, size_t argc, char **argv) {
    if (argc != 3) {
        LOG_ERR("Usage: %s <pulse_width_start> <pulse_width_end> in us", argv[0]);
        return -1;
    }

    for (int i = 0; i < argc; i++) {
        LOG_INF("argv[%d] = %s", i, argv[i]);
    }

    const uint32_t pulse_width_start = PWM_USEC(atoi(argv[1]));
    const uint32_t pulse_width_end = PWM_USEC(atoi(argv[2]));

    LOG_INF("Ramping PWM from %dus to %dus...", pulse_width_start / 1000, pulse_width_end / 1000);

    uint32_t pulse_width = pulse_width_start;
    enum ramp_direction direction = pulse_width_end >= pulse_width_start ? RAMP_UP : RAMP_DOWN;

    while ((direction == RAMP_UP && pulse_width < pulse_width_end) ||
            (direction == RAMP_DOWN && pulse_width > pulse_width_end)) {

        if (set_pulse_width_drivetrain(pulse_width)) {
            // Failed to set pulse width
            LOG_ERR("Failed to set pulse width. Cancel ramping.");
            return -1;
        }

        if (direction == RAMP_UP) {
            pulse_width += ramp_step;
        } else {
            pulse_width -= ramp_step;
        }

        k_sleep(K_MSEC(ramp_delay_ms));
    }
    LOG_INF("Ramping PWM from %dus to %dus...[DONE]", pulse_width_start / 1000, pulse_width_end / 1000);
    return 0;
}


/* Creating subcommands (level 1 command) array for command "pwm_dt" */
SHELL_STATIC_SUBCMD_SET_CREATE(dt_actions,
    SHELL_CMD(ramp, NULL, "Ramp the PWM drivetrain from pulse_width_start to pulse_width_end", cli_ramp_pwm_drivetrain),
    SHELL_SUBCMD_SET_END
);

/* Creating root (level 0) command "pwm_dt" */
SHELL_CMD_REGISTER(
    pwm_dt,
    &dt_actions,
    "Command to control the PWM of the drivetrain",
    NULL
);

