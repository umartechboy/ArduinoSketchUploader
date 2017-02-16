#pragma once
#include <stdint.h>
#include <Arduino.h>

#define MaxSyncRetries 5

#define CMD_SIGN_ON 0x01
#define CMD_GET_PARAMETER 0x03
#define CMD_LOAD_ADDRESS 0x06
#define CMD_ENTER_PROGRMODE_ISP 0x10
#define CMD_LEAVE_PROGMODE_ISP 0x11
#define CMD_PROGRAM_FLASH_ISP 0x13
#define CMD_READ_FLASH_ISP 0x14
#define CMD_PROGRAM_EEPROM_ISP 0x15
#define CMD_READ_EEPROM_ISP 0x16
#define STATUS_CMD_OK 0x00
#define MESSAGE_START 0x1b
#define TOKEN 0x0e
#define PARAM_HW_VER 0x90
#define PARAM_SW_MAJOR 0x91
#define PARAM_SW_MINOR 0x92

class CommonUtils
{
public:
	CommonUtils();

	void BlockCopy(uint8_t* src, uint16_t srcOffset, uint8_t* dest, uint16_t destOffset, uint16_t count);
	String GetString(uint8_t* buffer, uint8_t offset, uint8_t length);

private:

};
extern CommonUtils Common;


