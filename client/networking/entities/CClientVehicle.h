#pragma once

class CClientVehicle : public CNetworkVehicle
{
public:
	//librg_peer_t * client_peer;
	CVehicle * m_pVehicle;
	int m_iGameID;
	int m_iLastPacketTime;
	bool m_bStreamedIn;

	void Process();
	void StreamIn();
	void StreamOut();
};