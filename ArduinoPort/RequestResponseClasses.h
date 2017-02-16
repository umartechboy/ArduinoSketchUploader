#pragma once
#include <stdint.h>
#include <arduino.h>
#include "Common.h"
#include "MCU.h"

class IMessage
{
public:
	uint8_t* Bytes;
	uint16_t Length = 0;
};
class Request : public IMessage
{
};
class Response : public IMessage
{
public:
	bool IsNull = false;
	Response(uint8_t* buffer = 0, uint16_t bufferSize = 16); // by default, let it be 16.
	uint16_t bufferCapacity = 0;
	bool IsInSync();
	uint8_t ParameterValue();;
	bool IsSuccess();
	String Signature();
	bool Succeeded();
	uint8_t AnswerID();
	uint8_t Status();
};

class EnableProgrammingModeRequest :public  Request
{
public:
	EnableProgrammingModeRequest(IMCU* mcu); // use pointer to save memory
};

class ExecuteProgramPageRequest : public Request
{
public:
	ExecuteProgramPageRequest(byte writeCmd, IMemory* memory, uint8_t* data, uint16_t length);
};
class ExecuteReadPageRequest : public Request
{
public:
	ExecuteReadPageRequest(uint8_t readCmd, IMemory* memory);
};
class LoadAddressRequest : public Request
{
public:
	LoadAddressRequest(IMemory* memory, uint32_t addr);
};

class LeaveProgrammingModeRequest : public Request
{
public:
	LeaveProgrammingModeRequest();
};
