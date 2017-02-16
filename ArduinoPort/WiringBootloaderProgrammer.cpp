#include "WiringBootloaderProgrammer.h"


WiringBootloaderProgrammer::WiringBootloaderProgrammer(HardwareSerial* serial_)
{
	serial = serial_;
	DebugSerial = &Serial;
}
void WiringBootloaderProgrammer::begin()
{
	serial->begin(115200);
	delay(10);
	while (serial->available()) serial->read();
}

bool WiringBootloaderProgrammer::UploadSketch(String FileName)
{
	//while (serial->available())serial->read();
	DebugSerial->println("Establishing sync... ");
	if (!EstablishSync())
		return false;

	DebugSerial->println("Sync established.");

	DebugSerial->println("Checking device signature...");
	if (!CheckDeviceSignature())
		return false;
	DebugSerial->println("Device signature checked.");

	DebugSerial->println("Initializing device...");
	if (!InitializeDevice())
		return false;
	DebugSerial->println("Device initialized.");

	DebugSerial->println("Enabling programming mode on the device...");
	if (!EnableProgrammingMode())
		return false;
	DebugSerial->println("Programming mode enabled.");

	DebugSerial->println("Programming device...");
	//We, surely, cannot go with the approach of loading the file at once. Our device is so small.
	if (!ProgramDevice(&FileName))
		return false;
	DebugSerial->println("Device programmed.");

	DebugSerial->println("Leaving programming mode...");
	if (!LeaveProgrammingMode())
		return false;
	DebugSerial->println("Left programming mode!");
	DebugSerial->println("All done, shutting down!");
}

bool WiringBootloaderProgrammer::EstablishSync()
{
	int i;
	for (i = 0; i < MaxSyncRetries; i++)
	{
		digitalWrite(2, 0);
		delay(1);
		digitalWrite(2, 1);
		delay(100);
		Send((uint8_t)CMD_SIGN_ON); // we added a send overwride for sending bytes

		Response result;
		Receive(&result);
		if (result.IsNull || !result.IsInSync()) 
		{
			if (result.IsNull)
				DebugSerial->println("NullResponse.");
			if (!result.IsInSync())
			{
				sequenceNumber = 0;
				DebugSerial->println("Not in sync.");
			}
			continue;
		}
		deviceSignature = result.Signature();
			break;
	}

	return i < MaxSyncRetries;
}

bool WiringBootloaderProgrammer::CheckDeviceSignature()
{
	if (deviceSignature.compareTo("AVRISP_2"))
		return true;
	else
	{
		DebugSerial->print("Unknown Device Signature: ");
		DebugSerial->println(deviceSignature.c_str());
		return false;
	}
}

bool WiringBootloaderProgrammer::InitializeDevice()
{
	int16_t hardwareVersion = GetParameterValue(PARAM_HW_VER);
	if (hardwareVersion == -1) return false;
	int16_t softwareMajor = GetParameterValue(PARAM_SW_MAJOR);
	if (softwareMajor == -1) return false;
	int16_t softwareMinor = GetParameterValue(PARAM_SW_MINOR);
	if (softwareMinor == -1) return false;
	DebugSerial->print("Retrieved software version: ");
	DebugSerial->print(hardwareVersion);
	DebugSerial->print(" (hardware) - ");
	DebugSerial->print(softwareMajor);
	DebugSerial->print(".");
	DebugSerial->print(softwareMinor);
	DebugSerial->println(" (software)");
	return true;
}

bool WiringBootloaderProgrammer::EnableProgrammingMode()
{
	EnableProgrammingModeRequest request = EnableProgrammingModeRequest(&MCU);
	Send(&request);
	Response response;
	Receive(&response);
	if (response.IsNull)
	{
		DebugSerial->println("Unable to enable programming mode on the device!");
		return false;
	}
	else
	{
		return true;
	}
}

bool WiringBootloaderProgrammer::ProgramDevice(String* fileName)
{
	//File file = SD.open(fileName->c_str());
	//uint32_t sizeToWrite = file.size();
	uint32_t sizeToWrite = 256 * 16; // 16 pages (pseudo)
	IFlashMemory* flashMem = &MCU.FlashMemory;
	uint16_t pageSize = flashMem->PageSize;
	DebugSerial->print("Total bytes to write: "); DebugSerial->println(sizeToWrite);
	int offset;
	uint16_t dataBufSize = 270;
	uint8_t *bytesToCopy = new uint8_t[dataBufSize]; // leave some space for working
	uint8_t *bytesRead = new uint8_t[dataBufSize]; // leave some space for working
	for (offset = 0; offset < sizeToWrite; offset += pageSize)
	{
		if (!LoadAddress(flashMem, offset))
			return false;
		for (int i = 0; i < pageSize; i++) // Pseudo Bytes.
			//bytesToCopy[i] = (uint8_t)pgm_read_byte(offset + i);
			bytesToCopy[i] = i;

		if (!ExecuteWritePage(flashMem, offset, bytesToCopy, pageSize))
			return false;
		DebugSerial->print("[");

		delay(100);
		
		if (!LoadAddress(flashMem, offset))
			return false;

		// this call will set the receive buffer to bytesRead, set the capacity to pageSize. Then receive the data into it so that we don't lose the pointer.
		ExecuteReadPage(flashMem, bytesRead, dataBufSize);

		for (int i = 0; i < pageSize; i++)
		{
			if (bytesRead[i] != bytesToCopy[i + 10]) // we shifted them to the right in sending process
			{
				DebugSerial->println("Difference found during verification, write failed!");

				DebugSerial->print("At: 0x");
				DebugSerial->print(offset + i, 16);
				DebugSerial->print(", 0x");
				DebugSerial->print(bytesToCopy[i + 10], 16);
				DebugSerial->print(" != 0x");
				DebugSerial->println(bytesRead[i], 16);
				DebugSerial->println("Sent:");
				for (int j = 0; j < pageSize; j++)
				{
					if (bytesToCopy[j + 10] < 16) DebugSerial->print(0);
					DebugSerial->print(bytesToCopy[j + 10], 16); 
					DebugSerial->print(", ");
				}
				DebugSerial->println("\r\nReceived:");
				for (int j = 0; j < pageSize; j++)
				{
					if (bytesRead[j] < 16) DebugSerial->print(0);
					DebugSerial->print(bytesRead[j], 16);
					DebugSerial->print(", ");
				}

				DebugSerial->println();
				return false;
			}
		}
		DebugSerial->print("]");
	}
	DebugSerial->print("Bytes Written: "); DebugSerial->println(sizeToWrite);
	return true;
}

bool WiringBootloaderProgrammer::LeaveProgrammingMode()
{
	LeaveProgrammingModeRequest request = LeaveProgrammingModeRequest();
	Send(&request);
	Response response;
	Receive(&response);
	if (response.IsNull)
	{
		DebugSerial->println("Unable to leave programming mode on the device!");
		return false;
	}
	else return true;
}
bool WiringBootloaderProgrammer::ExecuteReadPage(IMemory* memory, byte* data, uint16_t maxBytesToRead)
{
	uint8_t readCmd = memory->Type == FLASH ? CMD_READ_FLASH_ISP : CMD_READ_EEPROM_ISP;

	ExecuteReadPageRequest request = ExecuteReadPageRequest(readCmd, memory);
	Send(&request);
	Response response = Response(data, maxBytesToRead); // this will set the buffer capacity but not initialize it. and set the pointer to given buffer
	Receive(&response);
	if (response.IsNull || response.AnswerID() != readCmd || response.Status() != STATUS_CMD_OK)
	{
		DebugSerial->println("Executing read page request failed!");
		return false;
	}

	// no sure if we need to do this.
	Common.BlockCopy(response.Bytes, 2, response.Bytes, 0, memory->PageSize);
	return true;
}
bool WiringBootloaderProgrammer::LoadAddress(IMemory* memory, int offset)
{
	offset = offset >> 1;
	LoadAddressRequest request = LoadAddressRequest(memory, offset);
	Send(&request);
	Response response;
	Receive(&response);
	if (response.IsNull || !response.Succeeded())
	{
		DebugSerial->println("Unable to execute load address!");
		return false;
	}
	else return true;
}

bool WiringBootloaderProgrammer::ExecuteWritePage(IMemory *memory, int offset, uint8_t* bytes, uint16_t length)
{
	uint8_t writeCmd = memory->Type == FLASH ? CMD_PROGRAM_FLASH_ISP : CMD_PROGRAM_EEPROM_ISP;

	ExecuteProgramPageRequest request = ExecuteProgramPageRequest(writeCmd, memory, bytes, length);
	
	Send(&request);
	Response response;
	Receive(&response);
	
	if (response.IsNull || response.AnswerID() != writeCmd
		|| response.Status() != STATUS_CMD_OK)
	{
		DebugSerial->print("Executing write page request failed at offset ");
		DebugSerial->println(offset);
		return false;
	}
	return true;
}
int16_t WiringBootloaderProgrammer::GetParameterValue(uint8_t param)
{
	Send((uint8_t)CMD_GET_PARAMETER);
	Response response;
	Receive(&response);
	if (response.IsNull || !response.IsSuccess())
		return -1;
	else return response.ParameterValue();
}
void WiringBootloaderProgrammer::Receive(Response* resp)
{
	//this function must receive a pointer to already initialized Response bytes length to accomodate expected number of bytes.
	int16_t messageStart = ReceiveNext();
	if (messageStart != MESSAGE_START)
	{
		DebugSerial->println("No Start Message detected!");
		resp->IsNull = true; return;
	}
	uint8_t tBuf[6];
	tBuf[0] = (uint8_t)messageStart;
	//DebugSerial->println("Received MESSAGE_START.");

	int16_t seqNumber = ReceiveNext();
	if (seqNumber != LastCommandSequenceNumber)
	{
		DebugSerial->println("Wrong sequence number!");
		resp->IsNull = true; return;
	}
	tBuf[1] = sequenceNumber;
	int16_t messageSizeHighByte = ReceiveNext();
	if (messageSizeHighByte == -1)
	{
		DebugSerial->println("Timeout ocurred!");
		resp->IsNull = true; return;
	}
	tBuf[2] = (byte)messageSizeHighByte;

	int16_t messageSizeLowByte = ReceiveNext();
	if (messageSizeLowByte == -1)
	{
		DebugSerial->println("Timeout ocurred!");
		resp->IsNull = true; return;
	}
	tBuf[3] = (byte)messageSizeLowByte;

	uint16_t messageSize = (messageSizeHighByte << 8) + messageSizeLowByte;
	int16_t token = ReceiveNext();
	if (token != TOKEN)
	{
		DebugSerial->println("Token not received!");
		resp->IsNull = true; return;
	}
	tBuf[4] = (byte)token;
	if (messageSize > resp->bufferCapacity)
	{
		DebugSerial->print("Coming data too big to fit in the buffer: ");
		DebugSerial->print(messageSize);
		DebugSerial->print(" > ");
		DebugSerial->println(resp->bufferCapacity);
		resp->IsNull = true; return;
	}
	int16_t tRec = ReceiveNext(messageSize, resp->Bytes , 0);
	if (tRec != messageSize)
	{
		DebugSerial->println("Inner message not received!");
		resp->IsNull = true; return;
	}

	uint16_t responseCheckSum = ReceiveNext();
	if (responseCheckSum == -1)
	{
		DebugSerial->println("Checksum not received!");
		resp->IsNull = true; return;
	}
	//tBuf[5] = (byte)responseCheckSum;

	byte checksum = 0;
	for (uint16_t i = 0; i < 5; i++) checksum ^= tBuf[i];
	for (uint16_t i = 0; i < messageSize; i++) checksum ^= resp->Bytes[i];

	if (responseCheckSum != checksum)
	{
		DebugSerial->println("Checksum incorrect!");
		resp->IsNull = true; return;
	}

	//Common.BlockCopy(resp->Bytes, 5, resp->Bytes, 0, messageSize); // shifts the data to the left by five counts
	resp->Length = messageSize;
}

void WiringBootloaderProgrammer::Send(uint8_t requestByte)
{
	Request req = Request();
	req.Bytes = new uint8_t[1];
	req.Bytes[0] = requestByte;
	req.Length = 1;
	Send(&req);
}
void WiringBootloaderProgrammer::Send(Request* request)
{
	uint8_t tArray[] = {
		(uint8_t)MESSAGE_START,
		(uint8_t)(LastCommandSequenceNumber = SequenceNumber()),
		(uint8_t)(request->Length >> 8),
		(uint8_t)(request->Length & 0xFF),
		(uint8_t)TOKEN
	};

	uint8_t checksum = 0;
	for (uint16_t i = 0; i < 5; i++)
		checksum ^= tArray[i];
	for (uint16_t i = 0; i < request->Length; i++)
		checksum ^= request->Bytes[i];
	serial->write(tArray, 5);
	serial->write(request->Bytes, request->Length);
	serial->write((uint8_t)checksum);
}
bool WiringBootloaderProgrammer::TimedRead(uint8_t* buffer, uint16_t count, uint16_t timeout)
{
	long start = millis();
	uint16_t crsr = 0;
	while (millis() - start < timeout && crsr < count)
	{
		if (serial->available()) buffer[crsr++] = serial->read();
		else delay(0); // for ESP which needs to keep other tasks alive as well
	}
	if (crsr == count) // no timeout occured
		return true;
	else
		return false;
}

int16_t WiringBootloaderProgrammer::ReceiveNext()
{
	uint8_t* bytes = new byte[1];
	if (TimedRead(bytes))
		return bytes[0];
	else return -1;
}
uint16_t WiringBootloaderProgrammer::ReceiveNext(uint16_t length, uint8_t* buffer, uint16_t offset, uint16_t timeout)
{
	uint16_t retrieved = 0;
	long last = millis();
	while (millis() - last < timeout && retrieved < length)
	{
		if (serial->available())
		{
			buffer[offset + retrieved++] = serial->read();
		}
	}
	return retrieved;
}
uint8_t WiringBootloaderProgrammer::SequenceNumber()
{
	if (sequenceNumber == 255) sequenceNumber = 0;
	return ++sequenceNumber;
}
