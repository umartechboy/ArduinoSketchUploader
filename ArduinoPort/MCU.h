#pragma once
#include <stdint.h>
#include <arduino.h>

enum Command
{
	PGM_ENABLE,
	READ_FLASH,
	READ_EEPROM
};

enum MemoryType
{
	FLASH,
	EEPROM
};

class IMemory
{
public:
	MemoryType Type;
	uint32_t Size = 0;
	uint16_t PageSize = 0;
	uint8_t PollVal1 = 0, PollVal2 = 0, Delay = 0;
	uint8_t* CmdBytesRead;
	uint8_t* CmdBytesWrite;
};
class IFlashMemory:public IMemory
{
public:
	IFlashMemory();

}; 

class IEEPROMMemory:public IMemory
{
public:
	IEEPROMMemory();

private:

};

class IMCU
{
public:
	uint8_t DeviceCode = 0, DeviceRevision = 0, ProgType = 0, ParallelMode = 0, Polling = 0, SelfTimed = 0, LockBytes = 0;
	uint8_t FuseBytes = 0, Timeout = 0, StabDelay = 0, CmdExeDelay = 0, SynchLoops = 0, ByteDelay = 0, PollValue = 0, PollIndex = 0;
	
	virtual uint8_t* CommandBytes(Command command);

	IFlashMemory FlashMemory;
	IEEPROMMemory EEPROMMemory;
	
	//IList<IMemory> Memory{ get; }

};

class AtMega2560 :public IMCU
{
public:
	AtMega2560();
	uint8_t* CommandBytes(Command command);

private:
	uint8_t CommandBytes_PGM_ENABLE [4] = { 0xac, 0x53, 0x00, 0x00};
};
