#ifndef __ADC_H__
#define __ADC_H__

#include "zf_common_headfile.h"

#define RIGHT_REAR_X_MM     39
#define RIGHT_REAR_Y_MM    -10
#define RIGHT_FRONT_X_MM    48
#define RIGHT_FRONT_Y_MM     0
#define LEFT_FRONT_X_MM    -48
#define LEFT_FRONT_Y_MM      0
#define LEFT_REAR_X_MM     -39
#define LEFT_REAR_Y_MM     -10

#define RIGHT_REAR_INDUCTOR   ADC2_CH4_P04
#define RIGHT_FRONT_INDUCTOR  ADC2_CH2_P02
#define LEFT_FRONT_INDUCTOR   ADC2_CH0_P00
#define LEFT_REAR_INDUCTOR    ADC2_CH3_P03

#define INDUCTOR_RIGHT_REAR   0
#define INDUCTOR_RIGHT_FRONT  1
#define INDUCTOR_LEFT_FRONT   2
#define INDUCTOR_LEFT_REAR    3
#define INDUCTOR_COUNT        4

#define ADC_SAMPLE_COUNT 5
#define ADC_SAMPLE_LOSE  2
#define ADC_NORMAL_MAX   100

extern adc_channel_enum adc_channels[INDUCTOR_COUNT];
extern uint16 adc_filtered[INDUCTOR_COUNT];
extern uint16 adc_value[INDUCTOR_COUNT];
extern uint16 adc_cal_max[INDUCTOR_COUNT];
extern uint16 adc_cal_min[INDUCTOR_COUNT];

extern uint16 front_signal_threshold;
extern uint16 rear_signal_threshold;
extern uint16 front_error_weight;
extern uint16 rear_error_weight;

extern int16 front_error_mm;
extern int16 rear_error_mm;
extern int16 front_rear_delta_mm;

extern uint16 front_signal_sum;
extern uint16 rear_signal_sum;

extern int16 electromagnetic_error_mm;

void my_adc_init(void);
uint16 adc_trimmed_mean(adc_channel_enum channel);
uint16 adc_normalize(uint16 raw_value, uint8 index);
void adc_update(void);
int16 adc_calculate_row_error(
    uint16 left_value,
    uint16 right_value,
    int16 left_x_mm,
    int16 right_x_mm,
    uint16 signal_threshold,
    uint16 *signal_sum);
void adc_calculate_front_error(void);
void adc_calculate_rear_error(void);
void adc_calculate_delta(void);
void adc_calculate_fusion(void);
void adc_process(void);

#endif /* __ADC_H__ */
