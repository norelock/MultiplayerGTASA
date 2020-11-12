#pragma once

class CNetworkPlayer : public CNetworkEntity
{
public:
    CNetworkPlayer() : syncType(eSyncType::SYNC_TYPE_ON_FOOT)
    {
        constexpr uint32_t max = std::max({ sizeof(OnfootPlayerAimingSyncPacket),sizeof(InWaterPlayerSyncPacket)});
        this->syncData = (PlayerSyncPacket*)malloc(max);// allocate memory equals the size of the biggest packet
    }

    ~CNetworkPlayer()
    {
		if(syncData)
			free(syncData);
    }

    std::string strName;
	eSyncType syncType;
    PlayerSyncPacket* syncData;
};