#include "adc.h"

adc_channel_enum adc_channels[INDUCTOR_COUNT] = {
    RIGHT_REAR_INDUCTOR,
    RIGHT_FRONT_INDUCTOR,
    LEFT_FRONT_INDUCTOR,
    LEFT_REAR_INDUCTOR
};

uint16 adc_filtered[INDUCTOR_COUNT];
uint16 adc_value[INDUCTOR_COUNT];

uint16 adc_cal_max[INDUCTOR_COUNT] = {
    4095,
    4095,
    4095,
    4095
};

uint16 adc_cal_min[INDUCTOR_COUNT] = {
    0,
    0,
    0,
    0
};

uint16 front_signal_threshold = 5;
uint16 rear_signal_threshold = 5;

uint16 front_error_weight = 3;
uint16 rear_error_weight = 1;

int16 front_error_mm;
int16 rear_error_mm;
int16 front_rear_delta_mm;

uint16 front_signal_sum;
uint16 rear_signal_sum;

int16 electromagnetic_error_mm;

void my_adc_init(void)
{
    uint8 i;

    for (i = 0; i < INDUCTOR_COUNT; i++) {
        adc_init(adc_channels[i], ADC_12BIT);
    }
    
}

uint16 adc_trimmed_mean(adc_channel_enum channel)
{
    uint8 i;
    uint32 sum;
    uint16 sample;
    uint16 max_value;
    uint16 min_value;
    uint16 samples[ADC_SAMPLE_COUNT];

    sum = 0;

    for (i = 0; i < ADC_SAMPLE_COUNT; i++) {
        samples[i] = adc_convert(channel);
    }

    max_value = samples[0];
    min_value = samples[0];

    for (i = 0; i < ADC_SAMPLE_COUNT; i++) {
        sample = samples[i];
        sum += sample;

        if (sample > max_value) {
            max_value = sample;
        }

        if (sample < min_value) {
            min_value = sample;
        }
    }

    sum -= max_value;
    sum -= min_value;

    return (uint16)(sum / (ADC_SAMPLE_COUNT - ADC_SAMPLE_LOSE));
}

uint16 adc_normalize(uint16 raw_value, uint8 index)
{
    uint32 result;

    if (adc_cal_max[index] <= adc_cal_min[index]) {
        return 0;
    }

    if (raw_value <= adc_cal_min[index]) {
        return 0;
    }

    if (raw_value >= adc_cal_max[index]) {
        return ADC_NORMAL_MAX;
    }

    result = (uint32)(raw_value - adc_cal_min[index])
             * ADC_NORMAL_MAX
             / (adc_cal_max[index] - adc_cal_min[index]);

    return (uint16)result;
}

void adc_update(void)
{
    uint8 i;

    for (i = 0; i < INDUCTOR_COUNT; i++) {
        adc_filtered[i] = adc_trimmed_mean(adc_channels[i]);
        adc_value[i] = adc_normalize(adc_filtered[i], i);
    }
}

int16 adc_calculate_row_error(
    uint16 left_value,
    uint16 right_value,
    int16 left_x_mm,
    int16 right_x_mm,
    uint16 signal_threshold,
    uint16 *signal_sum)
{
    int32 sum;
    int32 numerator;

    sum = (int32)left_value + right_value;
    *signal_sum = (uint16)sum;

    if (sum < signal_threshold || sum == 0) {
        return 0;
    }

    numerator = (int32)left_value * left_x_mm
              + (int32)right_value * right_x_mm;

    return (int16)(numerator / sum);
}

void adc_calculate_front_error(void)
{
    front_error_mm = adc_calculate_row_error(
        adc_value[INDUCTOR_LEFT_FRONT],
        adc_value[INDUCTOR_RIGHT_FRONT],
        LEFT_FRONT_X_MM,
        RIGHT_FRONT_X_MM,
        front_signal_threshold,
        &front_signal_sum);
}

void adc_calculate_rear_error(void)
{
    rear_error_mm = adc_calculate_row_error(
        adc_value[INDUCTOR_LEFT_REAR],
        adc_value[INDUCTOR_RIGHT_REAR],
        LEFT_REAR_X_MM,
        RIGHT_REAR_X_MM,
        rear_signal_threshold,
        &rear_signal_sum);
}

void adc_calculate_delta(void)
{
    if ((front_signal_sum >= front_signal_threshold)
        && (rear_signal_sum >= rear_signal_threshold)) {
        front_rear_delta_mm = front_error_mm - rear_error_mm;
    } else {
        front_rear_delta_mm = 0;
    }
}

void adc_calculate_fusion(void)
{
    int32 total_weight = 0;
    int32 weighted_error = 0;

    if ((front_signal_sum >= front_signal_threshold)
        && (front_error_weight > 0)) {
        weighted_error += (int32)front_error_mm * front_error_weight;
        total_weight += front_error_weight;
    }

    if ((rear_signal_sum >= rear_signal_threshold)
        && (rear_error_weight > 0)) {
        weighted_error += (int32)rear_error_mm * rear_error_weight;
        total_weight += rear_error_weight;
    }

    if (total_weight > 0) {
        electromagnetic_error_mm = (int16)(weighted_error / total_weight);
    } else {
        electromagnetic_error_mm = 0;
    }
}

void adc_process(void)
{
    adc_update();

    adc_calculate_front_error();
    adc_calculate_rear_error();

    adc_calculate_delta();
    adc_calculate_fusion();
}
