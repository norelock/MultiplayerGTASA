#pragma once

class CTaskSerializer
{
public:
	uint16_t taskID;

	virtual void Serialize(CTask* task) = 0;
	virtual void Deserialize() = 0;
};