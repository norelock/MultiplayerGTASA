#include "main.h"

int pedModelIds[] = { 7, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 32, 33, 34, 35, 36, 37, 43, 44, 45, 46,
47, 48, 49, 50, 51, 52, 57, 58, 59, 60, 61, 62, 66, 67, 68, 70, 71, 72, 73, 78, 79, 80, 81, 82, 83, 84, 94, 95, 96, 97, 98, 99, 100, 101,
102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 120, 121, 122, 123, 124, 125, 126, 127, 128, 132,
133, 134, 135, 136, 137, 142, 143, 144, 146, 147, 153, 154, 155, 156, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 170, 171,
173, 174, 175, 176, 177, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 200, 202, 203, 204, 206, 209, 210, 212, 213, 217, 220,
221, 222, 223, 227, 228, 229, 230, 234, 235, 236, 239, 240, 241, 242, 247, 248, 249, 250, 252, 253, 254, 255, 258, 259, 260, 261, 262,
9, 10, 11, 12, 13, 31, 38, 39, 40, 41, 53, 54, 55, 56, 63, 64, 69, 75, 76, 77, 85, 87, 88, 89, 90, 91, 92, 93, 129, 130, 131, 138, 139,
140, 141, 145, 148, 150, 151, 152, 157, 169, 172, 178, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 201, 205, 207, 211, 214, 215,
216, 218, 219, 224, 225, 226, 231, 232, 233, 237, 238, 243, 244, 245, 246, 251, 256, 257, 263 };

int vehModelIds[] = { 400, 401, 402, 404, 405, 410, 411, 412, 413, 414, 415, 418, 419, 420, 421, 422, 424, 426, 429, 436, 438, 439, 445, 451, 458, 461, 462, 463,
466, 467, 468, 471, 474, 475, 477, 478, 479, 480, 482, 489, 491, 492, 496, 500, 505, 506, 507, 516, 517, 518, 521, 522, 526, 527, 529, 533, 534, 535, 536, 540, 541, 542,
542, 543, 545, 546, 547, 549, 550, 551, 554, 555, 558, 559, 560, 561, 562, 565, 566, 567, 568, 575, 576, 579, 580, 581, 585, 586, 587,
589, 600, 602, 603 };

std::list<CPed*> CPopulationManager::m_pPeds;
std::list<CVehicle*> CPopulationManager::m_pVehicles;

//void(__cdecl* original_CPopulation__AddToPopulation)(float, float, float, float);
void __cdecl CPopulation__AddToPopulation_Hook(float a1, float a2, float a3, float a4)
{
	//printf("Game is trying to add a ped to population\n");
    CPopulation::AddToPopulation(a1, a2, a3, a4);
	//return;
}

//CPed*(__cdecl* original_CPopulation__AddPed)(ePedType, int, CVector*, bool);
CPed* __cdecl CPopulation__AddPed_Hook(ePedType pedType, int modelIndex, CVector *posn, bool unknown)
{
	CPed* ped = CPopulation::AddPed(pedType, modelIndex, *posn, unknown);
	if (ped)
	{
		printf("The game added a ped to population\n");
		printf("- type: %d | model: %d | bIsCop: %d\n", pedType, modelIndex, unknown);
		if (ped->m_pVehicle)printf("-- has vehicle\n");
		int blip = CRadar::SetEntityBlip(eBlipType::BLIP_CHAR, CPools::GetPedRef(ped), 0, eBlipDisplay::BLIP_DISPLAY_BLIPONLY);
		CRadar::SetBlipSprite(blip, 0);
		CRadar::ChangeBlipScale(blip, 1);
		CRadar::ChangeBlipColour(blip, 0xFFFF00FF);
	}
	return ped;
}

CPed*(__cdecl* original_CPopulation__AddPedInCar)(CVehicle*, bool, int, int, bool, bool);
CPed* __cdecl CPopulation__AddPedInCar_Hook(CVehicle *vehicle, bool driver, int a3, int seatNumber, bool male, bool criminal)
{
	/*CPed* ped = original_CPopulation__AddPedInCar(vehicle, driver, a3, seatNumber, male, criminal);
	if (ped)
	{
		printf("The game added a ped to population\n");
		printf("- type: %d | model: %d | unk: %d\n", pedType, modelIndex, unknown);
		if (ped->m_pVehicle)printf("-- has vehicle\n");
	}*/
	int v6 = 0;
	CPed* ped = nullptr;
	CVector pos = vehicle->GetPosition();
	int driverModel = CPopulation::FindSpecificDriverModelForCar_ToUse(vehicle->m_nModelIndex);
	int pedType = 0;

	if (!driver || driverModel < 0 || CStreaming::ms_aInfoForModel[driverModel].m_nLoadState != 1)
	{
		switch (vehicle->m_nModelIndex)
		{
		case 407u:
			pedType = 19;
			driverModel = CStreaming::GetDefaultFiremanModel();
			goto LABEL_23;
		case 416u:
			pedType = 18;
			driverModel = CStreaming::GetDefaultMedicModel();
			goto LABEL_23;
		case 427u:
			pedType = 6;
			driverModel = 2;
			goto LABEL_28;
		case 430u:
		case 497u:
		case 596u:
		case 597u:
		case 598u:
		case 599u:
			pedType = 6;
			driverModel = 0;
			goto LABEL_28;
		case 432u:
		case 433u:
			pedType = 6;
			driverModel = 5;
			goto LABEL_28;
		case 490u:
			pedType = 6;
			driverModel = 4;
			goto LABEL_28;
		case 523u:
			pedType = 6;
			driverModel = 1;
			goto LABEL_28;
		case 570u:
			driverModel = CPopulation::ChooseCivilianOccupation(0, 0, -1, -1, -1, 0, 1, 0, 0);
			if (driverModel == -1)
			{
				goto LABEL_21;
			}
			break;
		default:
			v6 = 0;
			if (a3 >= 14 && a3 <= 23)
			{
				pedType = a3 - 7;
				driverModel = 7;
				goto LABEL_23;
			}
			driverModel = CPopulation::ChooseCivilianOccupationForVehicle(male, vehicle);
			if (driverModel < 0)
			{
			LABEL_21:
				driverModel = 7;
			}
			break;
		}
	}
	pedType = CModelInfo::ms_modelInfoPtrs[driverModel][1].m_nRefCount;
LABEL_23:
	if (driverModel < 0)
	{
		driverModel = 7;
	}
	if (!v6 && !CModelInfo::ms_modelInfoPtrs[driverModel]->m_pRwObject)
	{
		pedType = CModelInfo::ms_modelInfoPtrs[7][1].m_nRefCount;
		driverModel = 7;
	}
LABEL_28:
	ped = CPopulation::AddPed((ePedType)pedType, driverModel, pos, 0);
	int door = 0;
	if (seatNumber >= 0)
	{
		door = CCarEnterExit::ComputeTargetDoorToEnterAsPassenger(vehicle, seatNumber);
	}
	else
	{
		door = 0;
	}
	CCarEnterExit::SetPedInCarDirect(ped, vehicle, door, driver);
	if (criminal)
	{
		CPopulation::UpdatePedCount(ped, 1);
		ped->m_nPedType = 20;
		CPopulation::UpdatePedCount(ped, 0);
	}
	return ped;
}

/*
CPed* vehicle_CPopulation_AddPed(ePedType pedType, int modelIndex, CVector *posn, bool unknown)
{

}*/

//void(__cdecl* original_CPopulation__Update)(bool);
void  __cdecl CPopulation__Update_Hook(bool bGeneratePeds)
{
    CPopulation::Update(bGeneratePeds);
}

//void(__cdecl* original_CCarCtrl__GenerateRandomCars)();
void __cdecl CCarCtrl__GenerateRandomCars_Hook()
{
    CCarCtrl::GenerateRandomCars();
}

void CPopulationManager::Init()
{

}