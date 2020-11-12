#pragma once

// Game controls. 
// credits: mta

enum eSyncType : uint8_t
{
    SYNC_TYPE_ON_FOOT,
	SYNC_TYPE_ON_FOOT_AIMING,
    SYNC_TYPE_IN_WATER,
	SYNC_TYPE_IN_VEHICLE,
	SYNC_TYPE_UNOCCUPIED,
	SYNC_TYPE_PASSENGER,
	SYNC_TYPE_PASSENGER_AIMING,
	SYNC_TYPE_SURFING
};

//always CPad::NewState
class CGtaControls
{
public:
	int16_t LeftStickX; // move/steer left (-128?)/right (+128)
	int16_t LeftStickY; // move back(+128)/forwards(-128?)
	int16_t RightStickX; // numpad 6(+128)/numpad 4(-128?)
	int16_t RightStickY;

	int16_t LeftShoulder1;
	int16_t LeftShoulder2;
	int16_t RightShoulder1; // target / hand brake
	int16_t RightShoulder2;

	int16_t DPadUp; // radio change up           Next radio station / Call gang forward/Recruit gang member
	int16_t DPadDown; // radio change down       Previous radio station / Gang stay back/Release gang (hold)
	int16_t DPadLeft; //                         Skip trip/Action / Negative talk reply
	int16_t DPadRight; //                        Next user MP3 track / Positive talk reply

	int16_t Start;                             //Pause
	int16_t Select;                            //Camera modes

	int16_t ButtonSquare; // jump / reverse      Break/Reverse / Jump/Climb
	int16_t ButtonTriangle; // get in/out        Exit vehicle / Enter veihcle
	int16_t ButtonCross; // sprint / accelerate  Accelerate / Sprint/Swim
	int16_t ButtonCircle; // fire                Fire weapon

	int16_t ShockButtonL;
	int16_t ShockButtonR; // look behind

	int16_t m_bChatIndicated;
	int16_t m_bPedWalk;
	int16_t m_bVehicleMouseLook;
	int16_t m_bRadioTrackSkip;
};

struct CCompressedControls
{
	union
	{
		struct
		{
			uint8_t bAnalogLeft : 1;
			uint8_t bAnalogRight : 1;
			uint8_t bAnalogUp : 1;
			uint8_t bAnalogDown : 1;
			uint8_t bAccelerate : 1;
			uint8_t bBrake : 1;
			uint8_t bHandBrake : 1;
			//uint8_t bPad : 1;
			//uint8_t bPad2 : 1;
		} inCarKeys;
		struct
		{
			uint8_t bAnalogLeft : 1;
			uint8_t bAnalogRight : 1;
			uint8_t bAnalogUp : 1;
			uint8_t bAnalogDown : 1;
			//uint8_t bDuck : 1;
			uint8_t bMelee : 1;
			uint8_t bSprint : 1;
			uint8_t bJump : 1;
			uint8_t bWalk : 1;
            uint8_t bAim : 1;
            uint8_t bEnterVeh : 1;
		} onFootKeys;
	};

	void ToOnfootGtaControls(CGtaControls* pGtaControls)
	{
        memset(pGtaControls, 0, sizeof(CGtaControls)); // we are already zeroing all members so we don't have to check for zero conditions

		if (onFootKeys.bAnalogLeft)         pGtaControls->LeftStickX = -128;
		else if (onFootKeys.bAnalogRight)   pGtaControls->LeftStickX = 128;
        //else                                pGtaControls->LeftStickX = 0;

		if (onFootKeys.bAnalogUp)           pGtaControls->LeftStickY = -128;
		else if (onFootKeys.bAnalogDown)    pGtaControls->LeftStickY = 128;
        //else                                pGtaControls->LeftStickY = 0;

		//if (onFootKeys.bDuck)   pGtaControls->ShockButtonL = 128;
		if (onFootKeys.bMelee)  pGtaControls->ButtonCircle = 128;
        if (onFootKeys.bAim)    pGtaControls->RightShoulder1 = 128;
		if (onFootKeys.bSprint) pGtaControls->ButtonCross = 128;
		if (onFootKeys.bJump)   pGtaControls->ButtonSquare = 128;
		if (onFootKeys.bWalk)   pGtaControls->m_bPedWalk = 128;
        if (onFootKeys.bEnterVeh)   pGtaControls->ButtonTriangle = 128;
	}

	void FromOnfootGtaControls(CGtaControls *pGtaControls)
	{
        memset(this, 0, sizeof(CCompressedControls));
		if (pGtaControls->LeftStickX < 0)   onFootKeys.bAnalogLeft = true;
		if (pGtaControls->LeftStickX > 0)   onFootKeys.bAnalogRight = true;
		if (pGtaControls->LeftStickY < 0)   onFootKeys.bAnalogUp = true;
		if (pGtaControls->LeftStickY > 0)   onFootKeys.bAnalogDown = true;
            
		//if (pGtaControls->ShockButtonL > 0) onFootKeys.bDuck = true;
		if (pGtaControls->ButtonCircle > 0) onFootKeys.bMelee = true;
		if (pGtaControls->ButtonCross > 0)  onFootKeys.bSprint = true;
		if (pGtaControls->ButtonSquare > 0) onFootKeys.bJump = true;
		if (pGtaControls->m_bPedWalk > 0)   onFootKeys.bWalk = true;
        if (pGtaControls->RightShoulder1 > 0) onFootKeys.bAim = true;
        if (pGtaControls->ButtonTriangle > 0) onFootKeys.bEnterVeh = true;
	}

	void ToIncarGtaControls(CGtaControls* pGtaControls)
	{
        memset(pGtaControls, 0, sizeof(CGtaControls)); // we are already zeroing all members so we don't have to check for zero conditions
        if (inCarKeys.bAnalogLeft)       pGtaControls->LeftStickX = -128;
        else if (inCarKeys.bAnalogRight) pGtaControls->LeftStickX = 128;
        //else                             pGtaControls->LeftStickX = 0;

        if (inCarKeys.bAnalogUp)         pGtaControls->RightStickX = -128;
        else if (inCarKeys.bAnalogDown)  pGtaControls->RightStickX = 128;
        //else                             pGtaControls->RightStickX = 0;
 
        if (inCarKeys.bAccelerate)  pGtaControls->ButtonCross = 128;
        if (inCarKeys.bBrake)       pGtaControls->ButtonSquare = 128;
        if (inCarKeys.bHandBrake)   pGtaControls->ButtonTriangle = 128;
	}

	void FromIncarGtaControls(CGtaControls* pGtaControls)
	{
        memset(this, 0, sizeof(CCompressedControls));
        if (pGtaControls->LeftStickX < 0)   inCarKeys.bAnalogLeft = true;
        if (pGtaControls->LeftStickX > 0)   inCarKeys.bAnalogRight = true;
		if (pGtaControls->RightStickX < 0)  inCarKeys.bAnalogUp = true;
		if (pGtaControls->RightStickX > 0)  inCarKeys.bAnalogDown = true;

		if (pGtaControls->ButtonCross > 0)  inCarKeys.bAccelerate = true;
		if (pGtaControls->ButtonSquare > 0) inCarKeys.bBrake = true;
        if (pGtaControls->ButtonTriangle > 0)   inCarKeys.bHandBrake = true;
	}
};

//int size = sizeof(CCompressedControls);


struct tCameraData
{
	zplm_vec3_t vecFront;
	zplm_vec3_t vecSource;
	zplm_vec3_t vecSourceBeforeLookBehind;
	zplm_vec3_t vecUp;
};

enum eNetworkEntityType : uint8_t
{
	ALL,
	PLAYER,
	VEHICLE,
	PED
};

inline void librg_data_wstr(librg_data_t* data, void* ptr, usize size)
{
	librg_data_wu32(data, size + 1);
	librg_data_wptr(data, ptr, size + 1);
}

inline std::string librg_data_rstr(librg_data_t* data)
{
	usize size = librg_data_ru32(data);
	char* str = (char*)malloc(sizeof(char) * size);
	librg_data_rptr(data, str, size);
	return std::string(str);
}

struct PlayerJoinPacket
{
	usize iPlayerNameLen;
	char szPlayerName[MAX_PLAYER_NAME];
	uint32_t iNetworkID;
};

/*
#pragma pack(push, 1)
struct PlayerSyncPacket
{
	uint8_t bInCar;

	zplm_vec3_t vecMoveSpeed;
	zplm_vec4_t quatRotation;
	float fHeading;

	CCompressedControls controls;
	zplm_vec3_t vecAimingTarget;
	float fCameraZoom;
	uint8_t iCameraMode;

	uint8_t iWeaponID;
	uint16_t iAmmo;

	uint16_t iNetworkVehicleID;
	uint8_t iSeat;

	uint8_t fHealth;
	uint8_t fArmour;

	bool operator==(const PlayerSyncPacket& right)
	{
		return memcmp(this, &right, sizeof(PlayerSyncPacket)) == 0;
	}
};
#pragma pack(pop)*/

struct SyncPacket{};

#pragma pack(push, 1)
struct PlayerSyncPacket
{
    float fCurrentRotation;
    //float fAimingRotation;
    zplm_vec3_t vecMoveSpeed;

    CCompressedControls controls;

    uint8_t iWeaponID;

    uint8_t fHealth;
    uint8_t fArmour;
};

struct OnfootPlayerSyncPacket : public PlayerSyncPacket
{
	float fHeading;

	bool operator==(const OnfootPlayerSyncPacket& right)
	{
		return memcmp(this, &right, sizeof(OnfootPlayerSyncPacket)) == 0;
	}
};

struct OnfootPlayerAimingSyncPacket : public OnfootPlayerSyncPacket
{
	zplm_vec3_t vecAimPos;

	bool operator==(const OnfootPlayerAimingSyncPacket& right)
	{
		return memcmp(this, &right, sizeof(OnfootPlayerAimingSyncPacket)) == 0;
	}
};

struct InWaterPlayerSyncPacket : public PlayerSyncPacket
{
	zplm_vec4_t quatRotation;

	bool operator==(const InWaterPlayerSyncPacket& right)
	{
		return memcmp(this, &right, sizeof(InWaterPlayerSyncPacket)) == 0;
	}
};

struct VehicleSyncPacket : public SyncPacket
{
    zplm_vec4_t quatRotation;

    uint8_t fHealth;

    bool operator==(const VehicleSyncPacket& right)
    {
        return memcmp(this, &right, sizeof(VehicleSyncPacket)) == 0;
    }
};

struct BulletSyncEvent
{
	uint8_t weaponID;
	uint32_t ownerNetworkID;
	int32_t victimNetworkID;
	zplm_vec3_t vecOrigin;
	zplm_vec3_t vecTarget;
	zplm_vec3_t vecEffectPosn;
	zplm_vec3_t vecOriginForDriveBy;
};

struct BulletImpactSyncEvent
{
    uint8_t weaponID;
    uint32_t ownerNetworkID;
    int32_t victimNetworkID;

    zplm_vec3_t startPoint; // origin
    zplm_vec3_t endPoint; // origin
    int arg7;
};

#pragma pack(pop)

