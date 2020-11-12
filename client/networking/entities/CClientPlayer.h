#pragma once

class CClientPlayer : public CNetworkPlayer
{
public:

    CClientPlayer() : m_pPed(nullptr)
    {

    }

	//librg_peer_t * client_peer;
	CPed * m_pPed;
	int m_iGameID;
	int m_iLastPacketTime;
	bool m_bStreamedIn;

	void Process();
	void StreamIn();
	void StreamOut();
	void SendTaskEvent(eTaskEventType type);
	eSyncType BuildSyncData();
	void ProcessBullet(BulletImpactSyncEvent *ev);
};