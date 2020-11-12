#pragma once
class CNetworkEntityManager
{
	CNetworkEntityManager() {}
public:
	static std::vector<CServerPlayer*> players;

    void Init();

};