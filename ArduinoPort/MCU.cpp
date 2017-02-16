#include "MCU.h"

IFlashMemory::IFlashMemory()
{
	Type = FLASH;
};

IEEPROMMemory::IEEPROMMemory()
{
	Type = EEPROM;
};

uint8_t* IMCU::CommandBytes(Command command)
{
	return 0;
}
AtMega2560::AtMega2560()
{
	DeviceCode = 0xb2;
	DeviceRevision = 0;
	LockBytes = 1;
	FuseBytes = 3;
	Timeout = 200;
	StabDelay = 100;
	CmdExeDelay = 25;
	SynchLoops = 32;
	ByteDelay = 0;
	PollIndex = 3;
	PollValue = 0x53;
	FlashMemory.Size = 256 * 1024;
	FlashMemory.PageSize = 256;
	FlashMemory.PollVal1 = 0x00;
	FlashMemory.PollVal2 = 0x00;
	FlashMemory.Delay = 10;
	FlashMemory.CmdBytesRead = new uint8_t[3] { 0x20, 0x00, 0x00 };
	FlashMemory.CmdBytesWrite = new uint8_t[3] { 0x40, 0x4c, 0x00 };
	EEPROMMemory.Size = 4 * 1024;
	EEPROMMemory.PollVal1 = 0x00;
	EEPROMMemory.PollVal2 = 0x00;
	EEPROMMemory.Delay = 10;
	EEPROMMemory.CmdBytesRead = new byte[3] { 0xa0, 0x00, 0x00 };
	EEPROMMemory.CmdBytesWrite = new byte[3] { 0xc1, 0xc2, 0x00 };
}
uint8_t* AtMega2560::CommandBytes(Command command)
{
	if (command == PGM_ENABLE)
		return CommandBytes_PGM_ENABLE;
	else return 0;
}
