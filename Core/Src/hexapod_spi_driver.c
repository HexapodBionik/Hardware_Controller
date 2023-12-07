#include "hexapod_spi_driver.h"
#include "spi.h"
#include <stdbool.h>
#include "servo_control.h"


static TIM_HandleTypeDef* servo_timers[] = {
        SERVO_11_TIMER,
        SERVO_12_TIMER,
        SERVO_13_TIMER,
        SERVO_21_TIMER,
        SERVO_22_TIMER,
        SERVO_23_TIMER,
        SERVO_31_TIMER,
        SERVO_32_TIMER,
        SERVO_33_TIMER,
        SERVO_41_TIMER,
        SERVO_42_TIMER,
        SERVO_43_TIMER,
        SERVO_51_TIMER,
        SERVO_52_TIMER,
        SERVO_53_TIMER,
        SERVO_61_TIMER,
        SERVO_62_TIMER,
        SERVO_63_TIMER
};

static uint8_t servo_channels[] = {
        SERVO_11_CHANNEL,
        SERVO_12_CHANNEL,
        SERVO_13_CHANNEL,
        SERVO_21_CHANNEL,
        SERVO_22_CHANNEL,
        SERVO_23_CHANNEL,
        SERVO_31_CHANNEL,
        SERVO_32_CHANNEL,
        SERVO_33_CHANNEL,
        SERVO_41_CHANNEL,
        SERVO_42_CHANNEL,
        SERVO_43_CHANNEL,
        SERVO_51_CHANNEL,
        SERVO_52_CHANNEL,
        SERVO_53_CHANNEL,
        SERVO_61_CHANNEL,
        SERVO_62_CHANNEL,
        SERVO_63_CHANNEL
};

static bool isFrameType(uint8_t frame_length, uint8_t suspected_frame_length){
    if(frame_length == suspected_frame_length){
        return true;
    }
    return false;
}


void sendSPIBlocking(SPI_HandleTypeDef* hspi, RAW_SPI_Message* message){
    // First send data lenth
    HAL_SPI_Transmit_IT(hspi, &message->dataLength, 1);

    HAL_SPI_Transmit_IT(hspi, message->pData, message->dataLength);
}


void receiveSPIBlocking(SPI_HandleTypeDef* hspi, RAW_SPI_Message* message){
    HAL_StatusTypeDef status;

    // Get message length
    status = HAL_SPI_Receive(hspi, &message->dataLength, 1, HAL_MAX_DELAY);

    if(status == HAL_OK){
        HAL_SPI_Receive(hspi, message->pData, message->dataLength, HAL_MAX_DELAY);
    }
}


void analyzeRawMessage(RAW_SPI_Message* message){
    uint8_t type = message->pData[0];

    switch (type) {
        case ALL_SERVO:
            if(isFrameType(message->dataLength, ALL_SERVO_TYPE_LEN)){

            }
            break;
        case ONE_LEG:
            if(isFrameType(message->dataLength, ONE_LEG_TYPE_LEN)){

            }
            break;
        case ONE_SERVO:
            if(isFrameType(message->dataLength, ONE_SERVO_LEN)){
                uint8_t servo_id = message->pData[1];
                uint8_t servo_tables_index = ((servo_id / 10) - 1)*3 + ((servo_id % 10) - 1);
                uint8_t servo_operation_type = message->pData[2];

                float angle = (float)message->pData[3] + (float)message->pData[4] / 100.f;
                //float angle = 0;

                if(servo_operation_type == 0){
                    // Start servo PWW with given angle
                    startPWMServo(servo_timers[servo_tables_index], servo_channels[servo_tables_index]);
                    setServoAngle(servo_timers[servo_tables_index], servo_channels[servo_tables_index], angle);
                    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
                }
                else if(servo_operation_type == 1) {
                    // Stop servo PWM
                    disablePWMServo(servo_timers[servo_tables_index], servo_channels[servo_tables_index]);
                }
                else {
                    setServoAngle(servo_timers[servo_tables_index], servo_channels[servo_tables_index], angle);
                }
            }
            break;
        case READ_ADC:
            // TODO To be completed in the future
            break;
        default:
            break;
    }
}