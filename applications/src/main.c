// Copyright 2023 Jasper Jonker
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/uart.h>
// #include <zephyr/drivers/adc.h>
// #include <stdio.h>

// #include <zephyr/logging/log.h>

/* LOG Module */
// LOG_MODULE_REGISTER(main);

/* Function prototypes */
int8_t init_pwm_led(uint32_t period, uint32_t pulse_width);
int8_t init_pwm_servo(void);

/* PWM LED */
static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

/* PWM Servo at PA6 */
static const struct pwm_dt_spec servo_pwm = PWM_DT_SPEC_GET(DT_ALIAS(servo0));

// ------------------------------------------------
// Initialize PWM LED
// ------------------------------------------------
int8_t init_pwm_led(uint32_t period, uint32_t pulse_width) {
    if (!device_is_ready(pwm_led0.dev)) {
        printk("Cannot find PWM LED device!\n");
        return -1;
    } else {
        printk("PWM LED device found: %s\r\n", pwm_led0.dev->name);
    }

    int ret = pwm_set_dt(&pwm_led0, period, pulse_width);
    if (ret < 0) {
        printk("Error %d: failed to set pulse width\r\n", ret);
        return -1;
    } else {
        printk("PWM set!\r\n");
    }

    return 0;
}

// ------------------------------------------------
// Initialize PWM Servo
// ------------------------------------------------
int8_t init_pwm_servo(void) {
    if (!device_is_ready(servo_pwm.dev)) {
        printk("Cannot find PWM servo device!\n");
        return -1;
    } else {
        printk("PWM servo device found: %s\r\n", servo_pwm.dev->name);
    }

    k_sleep(K_USEC(200));

    if (!pwm_set_pulse_dt(&servo_pwm, 1500)) {
        printk("Error Servo: failed to set pulse and period\r\n");
    } else {
        printk("PWM set to 1500!\r\n");
    }

    return 0;
}

void ramp_servo(const uint32_t pulse_width_start, const uint32_t pulse_width_end) {
    uint32_t pulse_width = pulse_width_start;

    while (pulse_width != pulse_width_end) {
        int ret = pwm_set_pulse_dt(&servo_pwm, pulse_width);
        if (ret < 0) {
            printk("Error %d: failed to set pulse width\r\n", ret);
        } else {
            printk("PWM set to %d!\r\n", pulse_width);
        }
        // if (!pwm_set_pulse_dt(&servo_pwm, pulse_width)) {
        //     printk("Error: failed to set pulse and period\r\n");
        // } else {
        //     printk("PWM set to %d!\r\n", pulse_width);
        // }

        if (pulse_width_end > pulse_width_start) {
            pulse_width += 10;
        } else {
            pulse_width -= 10;
        }

        k_sleep(K_MSEC(100));
    }
}

int main(void) {
    // LOG_INF("Booting...\r\n");

    init_pwm_led((uint32_t)2e9, (uint32_t)1e9);
    init_pwm_servo();
    ramp_servo(1500, 2000);

    while (1) {
        ramp_servo(2000, 1000);
        k_sleep(K_SECONDS(1));
        ramp_servo(1000, 2000);
        k_sleep(K_SECONDS(1));
    }

    // LOG_INF("Booting...[DONE]\r\n");

    while (1) {
        k_sleep(K_MSEC(1000));
    }

    return 0;
}
