#pragma once

class CNetworkEntity
{
public:
	uint32_t iNetworkID;
	eNetworkEntityType type;
	zpl_vec3 position;
	bool bSync = false;

	virtual void Process() = 0;
};