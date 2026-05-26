#include "lcd.h"
#include "spi.h"

#define LCD_CS_LOW()    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET)
#define LCD_CS_HIGH()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET)

#define LCD_DC_LOW()    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET)
#define LCD_DC_HIGH()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET)

#define LCD_RST_LOW()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET)
#define LCD_RST_HIGH()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET)

static void LCD_WriteCommand(uint8_t cmd)
{
    LCD_DC_LOW();
    LCD_CS_LOW();

    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);

    LCD_CS_HIGH();
}

static void LCD_WriteData(uint8_t* buff, size_t size)
{
    LCD_DC_HIGH();
    LCD_CS_LOW();

    HAL_SPI_Transmit(&hspi1, buff, size, HAL_MAX_DELAY);

    LCD_CS_HIGH();
}

void LCD_Init(void)
{
    LCD_CS_HIGH();
    LCD_DC_HIGH();
    LCD_RST_HIGH();
    HAL_Delay(50);

    LCD_RST_LOW();
    HAL_Delay(50);
    LCD_RST_HIGH();
    HAL_Delay(150);

    auto writeReg = [](uint8_t reg, uint16_t value)
    {
        uint8_t data[2];

        LCD_WriteCommand(reg);
        data[0] = value >> 8;
        data[1] = value & 0xFF;
        LCD_WriteData(data, 2);
    };

    writeReg(0x01, 0x011C);
    writeReg(0x02, 0x0100);
    writeReg(0x03, 0x1030);
    writeReg(0x08, 0x0808);
    writeReg(0x0C, 0x0000);
    writeReg(0x0F, 0x0A01);

    writeReg(0x10, 0x0A00);
    writeReg(0x11, 0x1038);
    HAL_Delay(50);

    writeReg(0x12, 0x1121);
    writeReg(0x13, 0x0066);
    writeReg(0x14, 0x5F60);

    writeReg(0x30, 0x0000);
    writeReg(0x31, 0x00DB);
    writeReg(0x32, 0x0000);
    writeReg(0x33, 0x0000);
    writeReg(0x34, 0x00DB);
    writeReg(0x35, 0x0000);
    writeReg(0x36, 0x00AF);
    writeReg(0x37, 0x0000);
    writeReg(0x38, 0x00DB);
    writeReg(0x39, 0x0000);

    writeReg(0x50, 0x0000);
    writeReg(0x51, 0x0808);
    writeReg(0x52, 0x080A);
    writeReg(0x53, 0x000A);
    writeReg(0x54, 0x0A08);
    writeReg(0x55, 0x0808);
    writeReg(0x56, 0x0000);
    writeReg(0x57, 0x0A00);
    writeReg(0x58, 0x0710);
    writeReg(0x59, 0x0710);

    writeReg(0x07, 0x1017);
}

void LCD_FillScreen(uint16_t color)
{
    uint8_t data[2];

    LCD_WriteCommand(0x22);

    data[0] = color >> 8;
    data[1] = color & 0xFF;

    LCD_DC_HIGH();
    LCD_CS_LOW();

    for (uint32_t i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++)
    {
        HAL_SPI_Transmit(&hspi1, data, 2, HAL_MAX_DELAY);
    }

    LCD_CS_HIGH();
}

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;

    uint8_t data[2];

    auto writeReg = [](uint8_t reg, uint16_t value)
    {
        uint8_t d[2];
        LCD_WriteCommand(reg);
        d[0] = value >> 8;
        d[1] = value & 0xFF;
        LCD_WriteData(d, 2);
    };

    writeReg(0x20, x);
    writeReg(0x21, y);

    LCD_WriteCommand(0x22);

    data[0] = color >> 8;
    data[1] = color & 0xFF;
    LCD_WriteData(data, 2);
}

void LCD_FillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
    for (int16_t y = -r; y <= r; y++)
    {
        for (int16_t x = -r; x <= r; x++)
        {
            if (x * x + y * y <= r * r)
            {
                LCD_DrawPixel(x0 + x, y0 + y, color);
            }
        }
    }
}