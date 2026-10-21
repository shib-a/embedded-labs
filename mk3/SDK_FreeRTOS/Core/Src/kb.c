#include "main.h"
#include "pca9538.h"
#include "kb.h"
#include "usart.h"
#include <stdint.h>

#define KBRD_ADDR 0xE2

HAL_StatusTypeDef Set_Keyboard( void ) {
	HAL_StatusTypeDef ret = HAL_OK;
	uint8_t buf;

	buf = 0;
	ret = PCA9538_Write_Register(KBRD_ADDR, POLARITY_INVERSION, &buf);
	if( ret != HAL_OK ) {
		goto exit;
	}

	buf = 0;
	ret = PCA9538_Write_Register(KBRD_ADDR, OUTPUT_PORT, &buf);
exit:
	return ret;
}

uint8_t Check_Row( uint8_t  Nrow ) {
	uint8_t Nkey = 0x00;
	HAL_StatusTypeDef ret = HAL_OK;
	uint8_t buf;
	uint8_t kbd_in;

	if( ret != HAL_OK ) {
	}

	buf = Nrow;
	ret = PCA9538_Write_Register(KBRD_ADDR, CONFIG, &buf);
	if( ret != HAL_OK ) {
	}

	ret = PCA9538_Read_Inputs(KBRD_ADDR, &buf);
	if( ret != HAL_OK ) {
	}

	kbd_in = buf & 0x70;
	Nkey = kbd_in;

	return Nkey;
}

char Scan_Keyboard()
{
	static const uint8_t rows[4] = {ROW1, ROW2, ROW3, ROW4};
	static const char key_map[4][3] = {
		{'1', '2', '3'},
		{'4', '5', '6'},
		{'7', '8', '9'},
		{'*', '0', '#'}
	};
    for (int r = 0; r < 4; r++) {
        uint8_t raw = Check_Row(rows[r]);
        if (!(raw & 0x10)) return key_map[r][0];
        if (!(raw & 0x20)) return key_map[r][1];
        if (!(raw & 0x40)) return key_map[r][2];
    }
    return 0;
}
