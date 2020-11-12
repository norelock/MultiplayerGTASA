#include "main.h"

CClientPlayer * CNetworkEntityManager::localPlayer;
std::vector<CClientPlayer*> CNetworkEntityManager::m_pPlayers;
std::vector<CNetworkEntity*> CNetworkEntityManager::m_pPeds;
std::vector<CNetworkEntity*> CNetworkEntityManager::m_pVehicles;

int32_t CNetworkEntityManager::FindNetworkIDByEntity(CEntity* entity, eNetworkEntityType type)
{
	if (entity == nullptr)return -1;
	if (type == eNetworkEntityType::PLAYER || type == eNetworkEntityType::ALL)
	{
		for (auto& ent : m_pPlayers)
		{
			if (ent->m_pPed == entity)
				return ent->iNetworkID;
		}
	}
	/*if (type == eNetworkEntityType::VEHICLE || type == eNetworkEntityType::ALL)
	{
		for (auto& ent : m_pVehicles)
		{
			if (ent->m_pVehicle == entity)
				return ent;
		}
	}
	if (type == eNetworkEntityType::PED || type == eNetworkEntityType::ALL)
	{
		for (auto& ent : m_pPeds)
		{
			if (ent->m_pPed == entity)
				return ent;
		}
	}*/
	return -1;
}

CNetworkEntity* CNetworkEntityManager::FindEntityByNetworkID(int32_t id, eNetworkEntityType type)
{
	if (id == -1)return nullptr;
	if (type == eNetworkEntityType::PLAYER || type == eNetworkEntityType::ALL)
	{
		for (auto& ent : m_pPlayers)
		{
			if (ent->iNetworkID == id)
				return ent;
		}
	}
	if (type == eNetworkEntityType::VEHICLE || type == eNetworkEntityType::ALL)
	{
		for (auto& ent : m_pVehicles)
		{
			if (ent->iNetworkID == id)
				return ent;
		}
	}
	if (type == eNetworkEntityType::PED || type == eNetworkEntityType::ALL)
	{
		for (auto& ent : m_pPeds)
		{
			if (ent->iNetworkID == id)
				return ent;
		}
	}
	return nullptr;
}

void CNetworkEntityManager::Process()
{
	CNetworkEntity* ent = nullptr;
	for (auto& ent : m_pPlayers)
	{
        ent->Process();
	}
    for (auto& it : m_pVehicles)
	{
		ent->Process();
	}
	for (auto& it : m_pPeds)
	{
		ent->Process();
	}
}

CClientPlayer* CNetworkEntityManager::CreatePlayer(uint32_t networkID, std::string name, bool isLocal)
{
	CClientPlayer* client = new CClientPlayer();
	client->iNetworkID = networkID;
	client->strName = name;
	client->type = eNetworkEntityType::PLAYER;
	
	if (isLocal)
	{
		client->m_pPed = FindPlayerPed(0);
		client->m_iGameID = 0;
		localPlayer = client;
	}
	else client->m_pPed = nullptr;

	CNetworkEntityManager::m_pPlayers.push_back(client);
	return client;
}

void CNetworkEntityManager::DisconnectPlayer(uint32_t networkID)
{
	CClientPlayer* player = (CClientPlayer*)FindEntityByNetworkID(networkID, eNetworkEntityType::PLAYER);
	if (player)
	{
		if(player->m_bStreamedIn)
			player->StreamOut();

		m_pPlayers.erase(std::remove(m_pPlayers.begin(),m_pPlayers.end(), player), m_pPlayers.end());
		delete player;
	}
}