#pragma once
class CPopulationManager
{
    CPopulationManager();
public:

    static void Init();
    static void Update();
    static CVector GetPedCreationCoords(CVector pos, bool vehicle = false);
    
    static std::list<CPed*> m_pPeds;
    static std::list<CVehicle*> m_pVehicles;
};

void __cdecl CPopulation__AddToPopulation_Hook(float a1, float a2, float a3, float a4);
CPed* __cdecl CPopulation__AddPed_Hook(ePedType pedType, int modelIndex, CVector *posn, bool unknown);
CPed* __cdecl CPopulation__AddPedInCar_Hook(CVehicle *vehicle, bool driver, int a3, int seatNumber, bool male, bool criminal);
void  __cdecl CPopulation__Update_Hook(bool bGeneratePeds);
void __cdecl CCarCtrl__GenerateRandomCars_Hook();