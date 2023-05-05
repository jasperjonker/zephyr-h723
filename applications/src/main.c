// Copyright 2023 Jasper Jonker
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/adc.h>
#include <string.h>  // memset

/* #include <zephyr/logging/log.h> */
/* LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL); */

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
    !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
    DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels, DT_SPEC_AND_COMMA)
};

// #define BUFFER_SIZE                 10000
// #define BAD_ANALOG_READ             -1

/* Thread settings */
#define STACKSIZE                   1024
#define PRIORITY                    7

/* Console UART settings*/
#define CONSOLE_UART_NAME           DT_NODELABEL(usart3)

/* Get UART device */
#if DT_NODE_HAS_STATUS(CONSOLE_UART_NAME, okay)
const struct device *const uart_dev = DEVICE_DT_GET(CONSOLE_UART_NAME);
#else
#error "UART is disabled"
#endif

static uint16_t buf;

static struct adc_sequence sequence = {
    .buffer = &buf,
    /* buffer size in bytes, not number of samples */
    .buffer_size = sizeof(buf),
};

// struct k_poll_signal async_sig;


/* Initialize the ADC device */
void init_adc(void) {
    for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
        if (!device_is_ready(adc_channels[i].dev)) {
            printk("ADC controller device %s not ready\n", adc_channels[i].dev->name);
            return;
        }

        int err = adc_channel_setup_dt(&adc_channels[i]);
        if (err < 0) {
            printk("Could not setup channel #%d (%d)\n", i, err);
            return;
        }
    }

    if (!device_is_ready(uart_dev)) {
        printk("UART device %s not ready\r\n", uart_dev->name);
        return;
    }

    // Setup async signal
    // k_poll_signal_init(&async_sig);
    // struct k_poll_event async_event =
    //     K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SIGNAL, K_POLL_MODE_NOTIFY_ONLY, &async_sig);

    // Clean buffer
    // memset(sample_buffer, 0, sizeof(sample_buffer));

    print_uart3("ADC setup done\n");
    // printk("ADC setup done!\n");

    int err;
    int32_t val_mv;
    while (1) {
        for (int i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
            adc_sequence_init_dt(&adc_channels[i], &sequence);

            err = adc_read(adc_channels[i].dev, &sequence);
            if (err < 0) {
                print_uart3("ADC read failed\n");
            }

            if (adc_channels[i].channel_cfg.differential) {
                val_mv = (int32_t)((int16_t)buf);
            } else {
                val_mv = (int32_t)buf;
            }

            err = adc_raw_to_millivolts_dt(&adc_channels[i], &val_mv);
            if (err < 0) {
                print_uart3("ADC conversion to milivolt not supported\n");
            }
            char buffer[50];
            snprintf(buffer, sizeof(buffer), "ADC read (channel %d): %d mV\r\n", adc_channels[i].channel_id, val_mv);
            print_uart3(buffer);
        }
        k_sleep(K_MSEC(1000));
    }

    // bool first_half = true;
    // int half_buffer_size = sizeof(sample_buffer) / 2;

    // Start sampling
    // int err = adc_read_async(adc_dev, &sequence, &async_sig);
    // if (err) {
    //     printk("ADC read failed with code %d!\n", err);
    // } else {
    //     printk("ADC read started!\n");
    // }

    // while (1) {
    //     k_poll(&async_event, 1, K_FOREVER);

    //     async_event.signal->signaled = 0;
    //     async_event.state = K_POLL_STATE_NOT_READY;

    //     if (first_half) {
    //         send_all_usb(uart_dev,
    //             (uint8_t*)sample_buffer, half_buffer_size);
    //         first_half = false;
    //     } else {
    //         send_all_usb(uart_dev,
    //             ((uint8_t*)sample_buffer) + half_buffer_size, half_buffer_size);
    //         first_half = true;
    //     }
    // }
}

// ------------------------------------------------
// high level read adc channel and convert to float voltage
// ------------------------------------------------
void print_uart3(char *buf) {
    int msg_len = strlen(buf);
    for (int i = 0; i < msg_len; i++) {
        uart_poll_out(uart_dev, buf[i]);
    }
}


int8_t init_pwm(void) {
    static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));
    if (!device_is_ready(pwm_led0.dev)) {
        printk("Cannot find PWM device!\n");
        return -1;
    } else {
        printk("PWM device found: %s\n", pwm_led0.dev->name);
    }

    int ret = pwm_set_dt(&pwm_led0, 2e9, 1e9);
    if (ret < 0) {
        printk("Error %d: failed to set pulse width\n", ret);
        return -1;
    } else {
        printk("PWM set!\n");
    }

    return 0;
}

K_THREAD_DEFINE(adc_uart_thread, STACKSIZE, init_adc, NULL, NULL, NULL, PRIORITY, 0, 0);

int main(void) {
    printk("Booting...\n");

    init_pwm();

    printk("Booting...[DONE]\n");

    while (1) {
        k_sleep(K_MSEC(1000));
        // printk("Zephyr Example\n");
    }

    return 0;
}
