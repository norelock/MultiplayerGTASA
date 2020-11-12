#include "main.h"

void CClientVehicle::Process()
{
    /*
	if (this == CNetworkEntityManager::Get().vehicles)   return;

    static CVector vecPosOffset = {};
    static CVector vecSpeedOffset = {};
    static bool bResetLerp = false;
    static bool bFirstProcess = true;
    static int iCurrentPacketTime = this->iLastPacketTime;

    if (iCurrentPacketTime != this->iLastPacketTime)
    {
        bResetLerp = true;
        iCurrentPacketTime = this->iLastPacketTime;
    }
    else
        bResetLerp = false;


	if (this->bSync)
	{
		CVector* nextPosition = (CVector*)&this->position;
        CVector* nextSpeed = (CVector*)&this->syncData.vecMoveSpeed;
		CVector* currentSpeed = &this->vehicle->m_vecMoveSpeed;
		CVector* currentPosition = &this->vehicle->m_matrix->pos;

        if (bFirstProcess)
        {
            this->vehicle->m_matrix->pos = *nextPosition;
            this->vehicle->m_vecMoveSpeed = *nextSpeed;
            bFirstProcess = false;
        }

        const float fLerpFactor = 30.f;
        if (bResetLerp)
        {
            vecPosOffset.x = (nextPosition->x - currentPosition->x) / fLerpFactor;
            vecPosOffset.y = (nextPosition->y - currentPosition->y) / fLerpFactor;
            vecPosOffset.z = (nextPosition->z - currentPosition->z) / fLerpFactor;

            vecSpeedOffset.x = (nextSpeed->x - currentSpeed->x) / fLerpFactor;
            vecSpeedOffset.y = (nextSpeed->y - currentSpeed->y) / fLerpFactor;
            vecSpeedOffset.z = (nextSpeed->z - currentSpeed->z) / fLerpFactor;
        }
        
        this->vehicle->SetHeading() = this->syncData.fHeading;

        this->vehicle->m_matrix->pos += vecPosOffset;
        this->vehicle->m_vecMoveSpeed += vecSpeedOffset;

		// Other stuff
		this->vehicle->m_fHealth = this->syncData.fHealth;


        CGtaControls controls = {};
        CPlayerPedManager::Get().remotePlayerKeys[this->iGameID] = *(CControllerState*)&controls;
	}
    */
}

void CClientVehicle::StreamIn()
{
    /*
	this->iGameID = CPlayerPedManager::Get().Create(7, { 2488.562f, -1660.865f, 12.8757f });
	this->ped = CPlayerPedManager::Get().players[this->iGameID];
	this->bStreamedIn = true;
    */
}

void CClientVehicle::StreamOut()
{
    /*
	if (this->ped)
	{
		CPlayerPedManager::Get().players[this->iGameID] = nullptr;
		CPlayerPedManager::Get().playerSlots[this->iGameID] = false;
		delete this->ped;
		this->ped = nullptr;
	}
	this->bStreamedIn = false;
    */
}