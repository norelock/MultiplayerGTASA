#pragma once
class CNetworkEntityManager
{
private:
	CNetworkEntityManager() {}
public:

	static CClientPlayer * localPlayer;

	static std::vector<CClientPlayer*> m_pPlayers;
	static std::vector<CNetworkEntity*> m_pPeds;
	static std::vector<CNetworkEntity*> m_pVehicles;

    static void Init();
	static CNetworkEntity* FindEntityByNetworkID(int32_t id, eNetworkEntityType type = eNetworkEntityType::ALL);
	static int32_t FindNetworkIDByEntity(CEntity* entity, eNetworkEntityType type = eNetworkEntityType::ALL);

	static CClientPlayer* CreatePlayer(uint32_t networkID, std::string name, bool isLocal = false);
	static void DisconnectPlayer(uint32_t networkID);

	static void Process();
};