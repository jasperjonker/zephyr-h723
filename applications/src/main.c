// Copyright 2023 Jasper Jonker
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

/* LOG Module */
LOG_MODULE_REGISTER(main);

/* Function prototypes */
int8_t init_pwm_led(uint32_t period, uint32_t pulse_width);
int8_t init_pwm_servo(void);
int8_t set_pulse_width(uint32_t pulse_width);
void ramp_servo(const uint32_t pulse_width_start, const uint32_t pulse_width_end, const uint32_t step);

/* PWM LED */
static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

/* PWM Servo at PA6 */
static const struct pwm_dt_spec servo = PWM_DT_SPEC_GET(DT_ALIAS(servo0));
static const uint32_t min_pulse = DT_PROP(DT_ALIAS(servo0), min_pulse);
static const uint32_t max_pulse = DT_PROP(DT_ALIAS(servo0), max_pulse);

// ------------------------------------------------
// Initialize PWM LED
// ------------------------------------------------
int8_t init_pwm_led(uint32_t period, uint32_t pulse_width) {
    if (!device_is_ready(pwm_led0.dev)) {
        LOG_ERR("Cannot find PWM LED device!\n");
        return -1;
    } else {
        LOG_INF("PWM LED device found: %s\r\n", pwm_led0.dev->name);
    }

    int ret = pwm_set_dt(&pwm_led0, period, pulse_width);
    if (ret < 0) {
        LOG_ERR("Error %d: failed to set pulse width\r\n", ret);
        return -1;
    } else {
        LOG_INF("PWM set!\r\n");
    }

    return 0;
}

// ------------------------------------------------
// Initialize PWM Servo
// ------------------------------------------------
int8_t init_pwm_servo(void) {
    if (!device_is_ready(servo.dev)) {
        LOG_ERR("Cannot find PWM servo device!\n");
        return -1;
    } else {
        LOG_INF("PWM servo device found: %s\r\n", servo.dev->name);
    }

    if (set_pulse_width(PWM_USEC(1500))) {
        return -1;
    }

    return 0;
}

// ------------------------------------------------
// Initialize PWM Servo
// ------------------------------------------------
int8_t set_pulse_width(uint32_t pulse_width) {
    // Make sure pulse width is within bounds
    if (pulse_width < min_pulse || pulse_width > max_pulse) {
        LOG_ERR("Error: pulse width %d us is out of bounds [%d, %d] us\r\n", pulse_width / 1000, min_pulse / 1000, max_pulse / 1000);
        return -1;
    }

    uint8_t ret = pwm_set_pulse_dt(&servo, pulse_width);

    if (ret < 0) {
        LOG_ERR("Error %d: failed to set pulse width to %d us\r\n", ret, pulse_width / 1000);
        return ret;
    } else {
        LOG_INF("PWM set to %d us\r\n", pulse_width / 1000);
        return 0;
    }
}

void ramp_servo(const uint32_t pulse_width_start, const uint32_t pulse_width_end, const uint32_t step) {
    uint32_t pulse_width = pulse_width_start;
    int ret;

    while (pulse_width != pulse_width_end) {
        if (set_pulse_width(pulse_width)) {
            return;
        }

        if (pulse_width_end >= pulse_width_start) {
            pulse_width += step;
        } else {
            pulse_width -= step;
        }

        k_sleep(K_MSEC(10));
    }
}

int main(void) {
    LOG_INF("Booting...\r\n");

    init_pwm_led((uint32_t)2e9, (uint32_t)1e9);
    init_pwm_servo();

    while (1) {
        k_sleep(K_SECONDS(5));
        // ramp_servo(PWM_USEC(1000), PWM_USEC(2100), PWM_USEC(10));
        set_pulse_width(PWM_USEC(1000));
        k_sleep(K_SECONDS(5));
        // ramp_servo(PWM_USEC(2000), PWM_USEC(900), PWM_USEC(10));
        set_pulse_width(PWM_USEC(2000));
    }

    LOG_INF("Booting...[DONE]\r\n");

    while (1) {
        k_sleep(K_SECONDS(1));
    }

    return 0;
}
