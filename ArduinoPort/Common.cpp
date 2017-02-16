#pragma once
#include "Common.h"

CommonUtils::CommonUtils()
{}
void CommonUtils::BlockCopy(uint8_t* src, uint16_t srcOffset, uint8_t* dest, uint16_t destOffset, uint16_t count)
{
	for (uint16_t i = 0; i < count; i++)
		dest[destOffset + i] = src[srcOffset+ i];
}
String CommonUtils::GetString(uint8_t* buffer, uint8_t offset, uint8_t length)
{
	String s = "";
	for (uint8_t i = 0; i < length; i++)
	{
		s += (char)buffer[offset + i];
	}
}

CommonUtils Common;
