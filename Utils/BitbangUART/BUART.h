#ifndef BUART_H
#define BUART_H

#include <stdint.h>

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

void BUART_Init(uint32_t baud);
void BUART_Deinit(void);

void BUART_Write(const uint8_t * data, uint32_t count);
void BUART_WriteStr(const char * str);
void BUART_WriteFlush(void);
uint32_t BUART_WriteCount(void);

uint32_t BUART_ReadCount(void);
uint32_t BUART_Read(uint8_t * data, uint32_t count);
uint8_t BUART_Pop(void);
void BUART_ReadFlush(void);

/*
 * PUBLIC FUNCTIONS
 */

#endif //BUART_H
