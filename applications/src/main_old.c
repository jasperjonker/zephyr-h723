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

/* ADC Settings */
#define ADC_DEVICE_NAME             DT_NODELABEL(adc1)
#define ADC_CHANNEL                 3
#define ADC_RESOLUTION              16
#define ADC_GAIN                    ADC_GAIN_1
#define ADC_REFERENCE               ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME        ADC_ACQ_TIME_DEFAULT
#define BUFFER_SIZE                 10000
#define BAD_ANALOG_READ             -1

/* Thread settings */
#define STACKSIZE                   1024
#define PRIORITY                    7

/* Console UART settings*/
#define CONSOLE_UART_NAME           DT_NODELABEL(usart3)

/* Get ADC device */
#if DT_NODE_HAS_STATUS(ADC_DEVICE_NAME, okay)
const struct device *const adc_dev = DEVICE_DT_GET(ADC_DEVICE_NAME);
#else
#error "ADC is disabled"
#endif

/* Get UART device */
#if DT_NODE_HAS_STATUS(CONSOLE_UART_NAME, okay)
const struct device *const uart_dev = DEVICE_DT_GET(CONSOLE_UART_NAME);
#else
#error "UART is disabled"
#endif

static int16_t sample_buffer[BUFFER_SIZE];

static struct adc_channel_cfg channel_cfg = {
    .gain = ADC_GAIN,
    .reference = ADC_REFERENCE,
    .acquisition_time = ADC_ACQUISITION_TIME,
    // Channel ID will be overwritten below
    .channel_id = 0,
    .differential = 0
};

static struct adc_sequence sequence = {
    .channels = BIT(0),
    .buffer = sample_buffer,
    .buffer_size = sizeof(sample_buffer),
    .resolution = ADC_RESOLUTION,
    .oversampling = 0,
    .calibrate = 0,
    .options = NULL
};

struct k_poll_signal async_sig;


static void send_all_usb(const struct device *dev, uint8_t *buffer, size_t size) {
    size_t count = size;
    while (count > 0) {
        count -= uart_fifo_fill(dev, buffer + size - count, count);
    }
}


/* Initialize the ADC device */
void init_adc(void) {
    int channel = ADC_CHANNEL;

    if (!device_is_ready(uart_dev)) {
        printk("UART device %s not ready\r\n", uart_dev->name);
        return;
    }

    if (!device_is_ready(adc_dev)) {
        printk("ADC device %s is not ready!\n", adc_dev->name);
        return;
    }

    // Setup channel
    channel_cfg.channel_id = channel;
    int8_t ret = adc_channel_setup(adc_dev, &channel_cfg);
    if (ret) {
        printk("ADC channel %d setup failed with code %d!\n", channel, ret);
    }
    sequence.channels |= BIT(channel);

    // Setup async signal
    k_poll_signal_init(&async_sig);
    struct k_poll_event async_event =
        K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SIGNAL, K_POLL_MODE_NOTIFY_ONLY, &async_sig);

    // Clean buffer
    memset(sample_buffer, 0, sizeof(sample_buffer));

    printk("ADC channel %d setup done!\n", channel);

    bool first_half = true;
    int half_buffer_size = sizeof(sample_buffer) / 2;

    // Start sampling
    int err = adc_read_async(adc_dev, &sequence, &async_sig);
    if (err) {
        printk("ADC read failed with code %d!\n", err);
    } else {
        printk("ADC read started!\n");
    }

    while (1) {
        k_poll(&async_event, 1, K_FOREVER);

        async_event.signal->signaled = 0;
        async_event.state = K_POLL_STATE_NOT_READY;

        if (first_half) {
            send_all_usb(uart_dev,
                (uint8_t*)sample_buffer, half_buffer_size);
            first_half = false;
        } else {
            send_all_usb(uart_dev,
                ((uint8_t*)sample_buffer) + half_buffer_size, half_buffer_size);
            first_half = true;
        }
    }
}

// ------------------------------------------------
// high level read adc channel and convert to float voltage
// ------------------------------------------------


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
