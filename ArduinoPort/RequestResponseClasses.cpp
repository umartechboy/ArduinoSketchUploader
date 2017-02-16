#include "RequestResponseClasses.h"

Response::Response(uint8_t* buffer, uint16_t bufferSize)
{
	if (bufferSize == 0)
		IsNull = true;
	else
	{
		// don't set the size. we just initialized the buffer on RAM. we don't know the content length yet.
		if (buffer)
		{
			Bytes = buffer;
		}
		else
		{
			Bytes = new uint8_t[bufferSize];
		}
		bufferCapacity = bufferSize;
	}
}
bool Response::IsInSync()
{
	return Length > 1 && Bytes[0] == CMD_SIGN_ON && Bytes[1] == STATUS_CMD_OK;
}
String Response::Signature()
{
	uint8_t signatureLength = Bytes[2];
	uint8_t* signature = new byte[signatureLength];
	return Common.GetString(Bytes, 3, signatureLength);
}

bool Response::IsSuccess()
{
	return Length > 2 && Bytes[0] == CMD_GET_PARAMETER && Bytes[1] == STATUS_CMD_OK;
}

bool Response::Succeeded()
{
	return Length == 2
	&& Bytes[0] == CMD_LOAD_ADDRESS
	&& Bytes[1] == STATUS_CMD_OK;
}
uint8_t Response::ParameterValue()
{
	return Bytes[2];
}

uint8_t Response::AnswerID()
{
	return Bytes[0];
}
uint8_t Response::Status()
{ 
	return Bytes[1]; 
} 

EnableProgrammingModeRequest::EnableProgrammingModeRequest(IMCU* mcu)
{
	uint8_t* cmdBytes = mcu->CommandBytes(PGM_ENABLE);
	Bytes = new uint8_t[12]
	{
		CMD_ENTER_PROGRMODE_ISP,
		mcu->Timeout,
		mcu->StabDelay,
		mcu->CmdExeDelay,
		mcu->SynchLoops,
		mcu->ByteDelay,
		mcu->PollValue,
		mcu->PollIndex,
		cmdBytes[0],
		cmdBytes[1],
		cmdBytes[2],
		cmdBytes[3]
	};
	Length = 12;
}
LoadAddressRequest::LoadAddressRequest(IMemory* memory, uint32_t addr)
{
	uint8_t modifier = memory->Type == FLASH ? 0x80 : 0x00;
	Bytes = new uint8_t[5]
	{
		CMD_LOAD_ADDRESS,
			(uint8_t)(((addr >> 24) & 0xff) | modifier),
			(uint8_t)((addr >> 16) & 0xff),
			(uint8_t)((addr >> 8) & 0xff),
			(uint8_t)(addr & 0xff)
	};
	Length = 5;
}


ExecuteReadPageRequest::ExecuteReadPageRequest(uint8_t readCmd, IMemory* memory)
{
	uint16_t pageSize = memory->PageSize;
	uint8_t cmdByte = memory->CmdBytesRead[0];
	Bytes = new uint8_t[4]
	{
		readCmd,
			(uint8_t)(pageSize >> 8),
			(uint8_t)(pageSize & 0xff),
			cmdByte
	};
	Length = 4;
}
ExecuteProgramPageRequest::ExecuteProgramPageRequest(byte writeCmd, IMemory* memory, uint8_t* data, uint16_t length)
{
	uint16_t mode = 0xc1;
	Bytes = data;
	for (uint16_t i = 1; i <= length; i++)
	{
		Bytes[length - i + 10] = Bytes[length - i];
	}
	uint8_t* headerBytes = new uint8_t[10]
	{
		writeCmd,
			(uint8_t)(length >> 8),
			(uint8_t)(length & 0xff),
			mode,
			memory->Delay,
			memory->CmdBytesWrite[0],
			memory->CmdBytesWrite[1],
			memory->CmdBytesWrite[2],
			memory->PollVal1,
			memory->PollVal2
	};
	Common.BlockCopy(headerBytes, 0, Bytes, 0, 10);
	Length = length + 10;
}

LeaveProgrammingModeRequest::LeaveProgrammingModeRequest()
{
	Bytes = new uint8_t[3]
	{
		CMD_LEAVE_PROGMODE_ISP,
			(byte)0x01,
			(byte)0x01
	};
	Length = 3;
}
