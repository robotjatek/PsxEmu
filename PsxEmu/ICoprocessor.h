#pragma once
#include <cstdint>

class ICoprocessor
{
public:
	virtual void Operation(uint32_t cop_fun) = 0;
	virtual void LoadWord(uint32_t w) = 0;
};