#include "main.h"

void CVehicleManager::Init() {

}

CVehicle *CVehicleManager::Create(int modelid, const CVector& position)
{
	if (CModelManager::LoadModel(modelid))
	{
		CVehicle *vehicle = nullptr;

		switch (GetVehicleType(modelid))
		{
		case VEHICLE_MTRUCK:
			vehicle = new CMonsterTruck(modelid, 1);
			break;
		case VEHICLE_QUAD:
			vehicle = new CQuadBike(modelid, 1);
			break;
		case VEHICLE_HELI:
			vehicle = new CHeli(modelid, 1);
			break;
		case VEHICLE_PLANE:
			vehicle = new CPlane(modelid, 1);
			break;
		case VEHICLE_BIKE:
			vehicle = new CBike(modelid, 1);
			reinterpret_cast<CBike *>(vehicle)->m_nDamageFlags |= 0x10;
			break;
		case VEHICLE_BMX:
			vehicle = new CBmx(modelid, 1);
			reinterpret_cast<CBmx *>(vehicle)->m_nDamageFlags |= 0x10;
			break;
		case VEHICLE_TRAILER:
			vehicle = new CTrailer(modelid, 1);
			break;
		case VEHICLE_BOAT:
			vehicle = new CBoat(modelid, 1);
			break;
		default:
			vehicle = new CAutomobile(modelid, 1, true);
			break;
		}
		if (vehicle) 
		{
			vehicle->SetPosn(position);
			vehicle->SetOrientation(0.0f, 0.0f, 0.0f);
			vehicle->m_nStatus = 4;
			vehicle->m_nDoorLock = CARLOCK_UNLOCKED;
			CWorld::Add(vehicle);
			return vehicle;
		}
	}
	return nullptr;
}

eVehicleType CVehicleManager::GetVehicleType(int modelid)
{
	return (eVehicleType)reinterpret_cast<CVehicleModelInfo *>(CModelInfo::ms_modelInfoPtrs[modelid])->m_nVehicleType;
}

void CVehicleManager::Delete(CVehicle* veh)
{
	CWorld::Remove(veh);
	operator_delete<CVehicle>(veh);
}