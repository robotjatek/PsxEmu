#pragma once
#include <cstdint>

class ICoprocessor
{
public:
	virtual void Operation(uint32_t cop_fun) = 0;
	virtual void LoadWord(uint32_t w, uint8_t rt) = 0;
	virtual uint32_t GetWord(uint8_t rt) = 0;
	virtual uint32_t MoveControlFromCoprocessor(uint8_t rd) = 0;
	virtual void MoveControlToCoprocessor(uint8_t rd, uint32_t control) = 0;
	virtual uint32_t MoveFromCoprocessor(uint8_t rd) = 0;
	virtual void MoveToCoprocessor(uint8_t rd, uint32_t data) = 0;
};