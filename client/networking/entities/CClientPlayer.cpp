#include "main.h"

void CClientPlayer::Process()
{
    if (this == CNetworkEntityManager::localPlayer)   return;

    static CVector vecPosOffset = {};
    static CVector vecSpeedOffset = {};
    static bool bFirstProcess = true;
    static int iCurrentPacketTime = this->m_iLastPacketTime;
    static int iLerpTimer = 0;

    // detect if we received a new packet
    if (iCurrentPacketTime != this->m_iLastPacketTime)
    {
        iLerpTimer = 0;
        iCurrentPacketTime = this->m_iLastPacketTime;
    }

    if (this->bSync)
    {
        CVector* nextPosition = (CVector*)&this->position;
        CVector* nextSpeed = (CVector*)&this->syncData->vecMoveSpeed;
        CVector* currentSpeed = &this->m_pPed->m_vecMoveSpeed;
        CVector* currentPosition = &this->m_pPed->m_matrix->pos;

        if (bFirstProcess)
        {
            this->m_pPed->m_matrix->pos = *nextPosition;
            this->m_pPed->m_vecMoveSpeed = *nextSpeed;
            bFirstProcess = false;
        }

        this->m_pPed->m_fCurrentRotation = syncData->fCurrentRotation;
        this->m_pPed->m_fAimingRotation = syncData->fCurrentRotation;

        if (syncType == eSyncType::SYNC_TYPE_ON_FOOT || syncType == eSyncType::SYNC_TYPE_ON_FOOT_AIMING)
        {
            OnfootPlayerAimingSyncPacket* data = (OnfootPlayerAimingSyncPacket*)this->syncData;
            this->m_pPed->SetHeading(data->fHeading);
			if (syncType == eSyncType::SYNC_TYPE_ON_FOOT_AIMING)
			{
                CTaskSimpleUseGun* useGun = this->m_pPed->m_pIntelligence->GetTaskUseGun();
                if (useGun)
                {
                    useGun->m_vecTarget = *(CVector*)&data->vecAimPos;
                    //if (useGun->m_pAnim)
                    //    this->m_pPed->m_pedIK.PointGunAtPosition(*(CVector*)&data->vecAimPos, useGun->m_pAnim->m_fBlendAmount);
                }
                
				printf("%.2f %.2f %.2f\n", data->vecAimPos.x, data->vecAimPos.y, data->vecAimPos.z);
			}
        }
        else if (syncType == eSyncType::SYNC_TYPE_IN_WATER)
        {
            InWaterPlayerSyncPacket* data = (InWaterPlayerSyncPacket*)this->syncData;

            CQuaternion *quat = (CQuaternion*)&data->quatRotation;
            quat->Get((RwMatrix*)&this->m_pPed->m_matrix);
        }

		const float fLerpFactor = 30.f;

        // iLerpTimer gets zeroed whenever a new packet is received
        if (iLerpTimer == 0)
        {
			if (this->m_pPed->m_aWeapons[this->m_pPed->m_nActiveWeaponSlot].m_nType != this->syncData->iWeaponID)
			{
				if (this->syncData->iWeaponID == 0)
				{
					this->m_pPed->SetCurrentWeapon(eWeaponType::WEAPON_UNARMED);
				}
				else CWeaponManager::GiveWeapon(this->m_pPed, (eWeaponType)this->syncData->iWeaponID, 1000, true);
			}

            // Other stuff
            this->m_pPed->m_fHealth = this->syncData->fHealth;
            this->m_pPed->m_fArmour = this->syncData->fArmour;

            this->syncData->controls.ToOnfootGtaControls((CGtaControls*)&CPlayerPedManager::m_RemotePlayerKeys[this->m_iGameID]);

            // interpolation-like method
            vecPosOffset.x = (nextPosition->x - currentPosition->x) / fLerpFactor;
            vecPosOffset.y = (nextPosition->y - currentPosition->y) / fLerpFactor;
            vecPosOffset.z = (nextPosition->z - currentPosition->z) / fLerpFactor;

            vecSpeedOffset.x = (nextSpeed->x - currentSpeed->x) / fLerpFactor;
            vecSpeedOffset.y = (nextSpeed->y - currentSpeed->y) / fLerpFactor;
            vecSpeedOffset.z = (nextSpeed->z - currentSpeed->z) / fLerpFactor;
        }

        // max value have to be same as LerpFactor
        if (iLerpTimer < 30)
        {
            this->m_pPed->m_matrix->pos += vecPosOffset;
            this->m_pPed->m_vecMoveSpeed += vecSpeedOffset;
            iLerpTimer++;
        }
    }
}

void CClientPlayer::StreamIn()
{
	this->m_iGameID = CPlayerPedManager::Create(7, { 2488.562f, -1660.865f, 12.8757f });
	this->m_pPed = CPlayerPedManager::m_pPlayers[this->m_iGameID];
	this->m_bStreamedIn = true;
}

void CClientPlayer::StreamOut()
{
	if (this->m_pPed)
	{
		CPlayerPedManager::m_pPlayers[this->m_iGameID] = nullptr;
		CPlayerPedManager::m_bPlayerSlots[this->m_iGameID] = false;
		delete this->m_pPed;
		this->m_pPed = nullptr;
	}
	this->m_bStreamedIn = false;
}

eSyncType CClientPlayer::BuildSyncData()
{
    if (this->m_pPed->m_nPedFlags.bInVehicle) return eSyncType::SYNC_TYPE_IN_VEHICLE;

    eSyncType syncType;


	if (!this->m_pPed->m_nPhysicalFlags.bTouchingWater)
	{
        OnfootPlayerAimingSyncPacket* syncData = static_cast<OnfootPlayerAimingSyncPacket*>(this->syncData);

        syncData->fHeading = m_pPed->GetHeading();

        syncType = eSyncType::SYNC_TYPE_ON_FOOT;

		eCamMode mode = TheCamera.m_aCams[TheCamera.m_nActiveCam].m_nMode;
		if (mode == eCamMode::MODE_AIMING 
            || mode == eCamMode::MODE_AIMWEAPON 
            || mode == eCamMode::MODE_AIMWEAPON_ATTACHED 
            || mode == eCamMode::MODE_AIMWEAPON_FROMCAR)
		{
			CVector source = this->m_pPed->GetPosition();
            source.z += 0.7f;
            CVector pCamVec;
            TheCamera.Find3rdPersonCamTargetVector(20.0f, source, &pCamVec, (CVector*)&syncData->vecAimPos);
			syncType = eSyncType::SYNC_TYPE_ON_FOOT_AIMING;
		}
	}
	else
	{
        InWaterPlayerSyncPacket* syncData = static_cast<InWaterPlayerSyncPacket*>(this->syncData);

        reinterpret_cast<CQuaternion*>(&syncData->quatRotation)->Set(*(RwMatrix*)m_pPed->m_matrix);

        syncType = eSyncType::SYNC_TYPE_IN_WATER;
	}

    syncData->fCurrentRotation = m_pPed->m_fCurrentRotation;
        
    syncData->vecMoveSpeed = *(zplm_vec3_t *)&m_pPed->m_vecMoveSpeed;

    syncData->controls.FromOnfootGtaControls((CGtaControls*)&CPad::GetPad(0)->NewState);

    syncData->iWeaponID = m_pPed->m_aWeapons[m_pPed->m_nActiveWeaponSlot].m_nType;

    syncData->fArmour = m_pPed->m_fArmour;
    syncData->fHealth = m_pPed->m_fHealth;
    return syncType;
	
}

void CClientPlayer::SendTaskEvent(eTaskEventType type)
{
	librg_message_send_all(&CNetworkManager::m_ctx, eNetworkEvents::PLAYER_TASK_EVENT, &type, sizeof(type));
}


void CClientPlayer::ProcessBullet(BulletImpactSyncEvent *ev)
{
    if (ev->ownerNetworkID == CNetworkEntityManager::localPlayer->iNetworkID)
    {
        //called by local player, send data to server
        librg_message_send_all(&CNetworkManager::m_ctx, eNetworkEvents::BULLET_SYNC_EVENT, ev, sizeof(BulletImpactSyncEvent));
        printf("sending bullet packet from local player\n");
    }
    else
    {
        CClientPlayer* owner = (CClientPlayer*)CNetworkEntityManager::FindEntityByNetworkID(ev->ownerNetworkID);
        if (owner && owner->m_pPed)
        {
            CWeapon* weap = &owner->m_pPed->m_aWeapons[owner->m_pPed->m_nActiveWeaponSlot];
            if (weap->m_nType == ev->weaponID)
            {
                CEntity* victim = nullptr;
                CClientPlayer* player = (CClientPlayer*)CNetworkEntityManager::FindEntityByNetworkID(ev->victimNetworkID);
                if (player && player->m_pPed)victim = player->m_pPed;
                //weap->Fire(owner->m_pPed, (CVector*)&ev->vecOrigin, (CVector*)&ev->vecEffectPosn, victim, (CVector*)&ev->vecTarget, (CVector*)&ev->vecOriginForDriveBy);

                CColPoint colPoint{};
                CEntity* pDummy;
                CWorld::ProcessLineOfSight(*(CVector*)&ev->startPoint,*(CVector*)&ev->endPoint,colPoint,pDummy,true,true,true,true,true,false,false,true);
                weap->DoBulletImpact(owner->m_pPed, victim, (CVector*)&ev->startPoint, (CVector*)&ev->endPoint, &colPoint, ev->arg7);
                printf("Processing bullet for remote player\n");
            }
        }
    }
}