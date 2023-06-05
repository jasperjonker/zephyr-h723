// Copyright 2023 Jasper Jonker
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>
#include "../lib/drivetrain/drivetrain.h"

/* LOG Module */
LOG_MODULE_REGISTER(main);

/* Function prototypes */
int8_t init_pwm_led(uint32_t period, uint32_t pulse_width);

/* PWM LED */
static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

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


int main(void) {
    LOG_INF("Booting...\r\n");

    init_pwm_led((uint32_t)2e9, (uint32_t)1e9);
    init_drivetrain();

    /*
    while (1) {
        k_sleep(K_SECONDS(5));
        // ramp_pwm(PWM_USEC(1000), PWM_USEC(2100), PWM_USEC(10));
        set_pulse_width_drivetrain(PWM_USEC(1000));
        k_sleep(K_SECONDS(5));
        // ramp_pwm(PWM_USEC(2000), PWM_USEC(900), PWM_USEC(10));
        set_pulse_width_drivetrain(PWM_USEC(2000));
    }
    */
    LOG_INF("Booting...[DONE]\r\n");

    // while (1) {
    //     ramp_pwm_drivetrain(PWM_USEC(1000), PWM_USEC(2000));
    //     ramp_pwm_drivetrain(PWM_USEC(2000), PWM_USEC(1000));
    // }


    while (1) {
        k_sleep(K_SECONDS(1));
    }

    return 0;
}
