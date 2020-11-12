#pragma once

class CServerPlayer : public CNetworkPlayer
{
public:
    CServerPlayer() : client_peer(nullptr) , lastSyncType(eSyncType::SYNC_TYPE_ON_FOOT)
    {
        constexpr uint32_t max = std::max({ sizeof(OnfootPlayerAimingSyncPacket),sizeof(InWaterPlayerSyncPacket) });
        this->lastSyncData = (PlayerSyncPacket*)malloc(max);// allocate memory equals the size of the biggest packet
    }

    ~CServerPlayer()
    {
        free(lastSyncData);
    }

	librg_peer_t* client_peer;
	
	eSyncType lastSyncType;
    PlayerSyncPacket* lastSyncData;

	void Process() {};
};