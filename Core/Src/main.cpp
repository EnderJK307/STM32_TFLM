#include <cstdio>
#include "main.h"
#include "usart.h"
#include "lcd.h"
#include "spi.h"
#include "gpio.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "constants.h"

#include "sine_model_data.h"

extern "C" int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

constexpr int kTensorArenaSize = 2 * 1024;
uint8_t tensor_arena[kTensorArenaSize];

static void InitLed()
{
    __HAL_RCC_GPIOD_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_13;   // LD3
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

int main()
{
    HAL_Init();
    MX_GPIO_Init();
    InitLed();
    MX_USART2_UART_Init();
    MX_SPI1_Init();
    LCD_Init();
    LCD_FillScreen(LCD_COLOR_RED);
    LCD_FillScreen(LCD_COLOR_BLACK);
    printf("UART OK\r\n");

    const tflite::Model* model =
        tflite::GetModel(hello_world_float_tflite);

    tflite::MicroMutableOpResolver<1> resolver;
    resolver.AddFullyConnected();

    tflite::MicroInterpreter interpreter(
        model,
        resolver,
        tensor_arena,
        kTensorArenaSize
    );

    if (interpreter.AllocateTensors() != kTfLiteOk)
    {
        while (1)
        {
            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
            HAL_Delay(100);
        }
    }

    TfLiteTensor* input = interpreter.input(0);
    input->data.f[0] = 1.0f;

    if (interpreter.Invoke() != kTfLiteOk)
    {
        while (1)
        {
            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
            HAL_Delay(100);
        }
    }

    TfLiteTensor* output = interpreter.output(0);
    float y = output->data.f[0];

    printf("Result = %f\r\n", y);

    int prev_x = 0;
    int prev_y = LCD_HEIGHT / 2;
    bool has_prev = false;

    float x = 0.0f;

    while (1)
    {
        input->data.f[0] = x;

        if (interpreter.Invoke() == kTfLiteOk)
        {
            float y = output->data.f[0];

            int x_pos = (int)((x / kXrange) * (LCD_WIDTH - 1));

            int y_pos = (LCD_HEIGHT / 2) -
                        (int)(y * (LCD_HEIGHT / 2 - 10));

            if (has_prev)
            {
                LCD_FillCircle(prev_x, prev_y, 3, LCD_COLOR_BLACK);
            }

            LCD_FillCircle(x_pos, y_pos, 3, LCD_COLOR_GREEN);

            prev_x = x_pos;
            prev_y = y_pos;
            has_prev = true;

            HAL_Delay(20);
        }

        x += kXrange / kInferencesPerCycle;

        if (x > kXrange)
        {
            x = 0.0f;

            if (has_prev)
            {
                LCD_FillCircle(prev_x, prev_y, 3, LCD_COLOR_BLACK);
                has_prev = false;
            }
        }
    }
}

extern "C" void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}