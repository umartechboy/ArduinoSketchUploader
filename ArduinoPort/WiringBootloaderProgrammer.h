#include <HardwareSerial.h>
#include "RequestResponseClasses.h"
#include "Common.h"
#include <Arduino.h>
#include <SD.h>

#pragma once

class WiringBootloaderProgrammer
{
public:
	HardwareSerial *serial;
	HardwareSerial *DebugSerial;
	AtMega2560 MCU;
	WiringBootloaderProgrammer(HardwareSerial* serial_);
	void begin();
	String deviceSignature;
	uint8_t sequenceNumber = 0;
	uint8_t LastCommandSequenceNumber = 0;
	uint8_t SequenceNumber();
	bool UploadSketch(String FileName);
	bool EstablishSync();
	bool CheckDeviceSignature();
	bool InitializeDevice();
	bool EnableProgrammingMode();
	bool ProgramDevice(String* fileName);
	bool LeaveProgrammingMode();
	bool LoadAddress(IMemory* memory, int offset);
	bool ExecuteWritePage(IMemory* memory, int offset, byte* bytes, uint16_t length);
	bool ExecuteReadPage(IMemory* memory, byte* data, uint16_t maxBytesToRead);
	int16_t GetParameterValue(uint8_t param);
	bool TimedRead(uint8_t* buffer, uint16_t count = 1, uint16_t timeout = 5000);
	int16_t ReceiveNext();
	void Send(uint8_t requestByte);
	void Send(Request* request); // save memory by using pointer
	void Receive(Response* resp);
	uint16_t ReceiveNext(uint16_t length, uint8_t* buffer, uint16_t offset = 0, uint16_t timeout = 5000);
	void BlockCopy(uint8_t* src, uint16_t srcOffset, uint8_t* dest, uint16_t destOffset, uint16_t count);
};

