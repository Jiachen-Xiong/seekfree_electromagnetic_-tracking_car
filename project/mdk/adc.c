#include "zf_common_headfile.h"

#define RIGHT_REAR_X_MM    39
#define RIGHT_REAR_Y_MM   -10
#define RIGHT_FRONT_X_MM   48
#define RIGHT_FRONT_Y_MM    0
#define LEFT_FRONT_X_MM   -48
#define LEFT_FRONT_Y_MM     0
#define LEFT_REAR_X_MM    -39
#define LEFT_REAR_Y_MM    -10

#define RIGHT_REAR_INDUCTOR  ADC2_CH4_P04 // 右后：(3.9, -1.0)，P0.4，IN1
#define RIGHT_FORNT_INDUCTOR ADC2_CH2_P02 // 右前：(4.8, 0)，P0.2，IN2
#define LEFT_FORNT_INDUCTOR  ADC2_CH0_P00 // 左前：(-4.8, 0)，P0.0，IN3
#define LEFT_REAR_INDUCTOR   ADC2_CH3_P03 // 左后：(-3.9, -1.0)，P0.3，IN4

#define ADC_SAMPLE_COUNT 5
#define ADC_SAMPLE_LOSE  2
#define ADC_NORMAL_MAX   100

uint16 right_rear_adc;
uint16 right_front_adc;
uint16 left_front_adc;
uint16 left_rear_adc;

uint16 right_rear_filtered;
uint16 right_front_filtered;
uint16 left_front_filtered;
uint16 left_rear_filtered;

uint16 right_rear_value;
uint16 right_front_value;
uint16 left_front_value;
uint16 left_rear_value;

// test
uint16 right_rear_cal_max  = 4095;
uint16 right_front_cal_max = 4095;
uint16 left_front_cal_max  = 4095;
uint16 left_rear_cal_max   = 4095;

uint16 right_rear_cal_min  = 0;
uint16 right_front_cal_min = 0;
uint16 left_front_cal_min  = 0;
uint16 left_rear_cal_min   = 0;

uint16 front_signal_threshold = 5;
uint16 rear_signal_threshold  = 5;

uint16 front_error_weight = 3;
uint16 rear_error_weight  = 1;

int16 front_error_mm;
int16 rear_error_mm;

uint16 front_signal_sum;
uint16 rear_signal_sum;

int16 front_rear_delta_mm;

int16 electromagnetic_center_x_mm;
int16 electromagnetic_center_y_mm;

int16 electromagnetic_error_mm;

void my_adc_init(void)
{
    adc_init(RIGHT_REAR_INDUCTOR,  ADC_12BIT);
    adc_init(RIGHT_FORNT_INDUCTOR, ADC_12BIT);
    adc_init(LEFT_REAR_INDUCTOR,   ADC_12BIT);
    adc_init(LEFT_FORNT_INDUCTOR,  ADC_12BIT);
}

uint16 adc_trimmed_mean(adc_channel_enum channel)
{
    uint8 i;
    uint8 max_index;
    uint8 min_index;
    uint32 sum;
    uint16 sample;
    uint16 max_value;
    uint16 min_value;

    uint16 samples[ADC_SAMPLE_COUNT];

    for (i = 0; i < ADC_SAMPLE_COUNT; i++)
    {
        samples[i] = adc_convert(channel);
    }

    max_index = 0;
    min_index = 0;
    max_value = samples[0];
    min_value = samples[0];
    sum = 0;

    for (i = 0; i < ADC_SAMPLE_COUNT; i++)
    {
        sample = samples[i];
        sum += sample;

        if (sample > max_value)
        {
            max_value = sample;
            max_index = i;
        }

        if (sample < min_value)
        {
            min_value = sample;
            min_index = i;
        }
    }

    sum -= max_value;
    sum -= min_value;

    return (uint16)(sum / (ADC_SAMPLE_COUNT - ADC_SAMPLE_LOSE));
}

uint16 adc_normalize(uint16 raw_value, uint16 cal_max, uint16 cal_min)
{
    uint32 result;

    if (cal_max <= cal_min)
    {
        return 0;
    }

    if (raw_value <= cal_min)
    {
        return 0;
    }

    if (raw_value >= cal_max)
    {
        return 100;
    }

    result = (uint32)(raw_value - cal_min) * ADC_NORMAL_MAX / (cal_max - cal_min);

    return (uint16)result;
}

void my_adc_update(void)
{
    right_rear_filtered = adc_trimmed_mean(RIGHT_REAR_INDUCTOR);
    right_front_filtered = adc_trimmed_mean(RIGHT_FORNT_INDUCTOR);
    left_front_filtered = adc_trimmed_mean(LEFT_FORNT_INDUCTOR);
    left_rear_filtered = adc_trimmed_mean(LEFT_REAR_INDUCTOR);

    // debug
    right_rear_adc  = right_rear_filtered;
    right_front_adc = right_front_filtered;
    left_front_adc  = left_front_filtered;
    left_rear_adc   = left_rear_filtered;

    right_rear_value = adc_normalize(right_rear_filtered, right_rear_cal_max, right_rear_cal_min);
    right_front_value = adc_normalize(right_front_filtered, right_front_cal_max, right_front_cal_min);
    left_front_value = adc_normalize(left_front_filtered, left_front_cal_max, left_front_cal_min);
    left_rear_value = adc_normalize(left_rear_filtered, left_rear_cal_max, left_rear_cal_min);
}

// test
void my_adc_calculate_error(void)
{

}

void my_adc_process(void)
{
    my_adc_update();
    my_adc_calculate_error();
}
