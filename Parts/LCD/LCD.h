#ifndef LCD_H
#define LCD_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

#define LCD_LINE_SIZE	16

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

void LCD_Init(void);
void LCD_Deinit(void);

void LCD_Clear(void);
void LCD_Seek(uint8_t row, uint8_t col);

void LCD_Write(const uint8_t * data, uint32_t size);
void LCD_WriteStr(const char * str);

/*
 * EXTERN DECLARATIONS
 */

#endif //LCD_H
