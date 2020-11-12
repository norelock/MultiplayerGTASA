#pragma once
class CVehicleManager
{
private:
    CVehicleManager();
public:

    static void Init();
    static CVehicle *Create(int modelid, const CVector& position);
    static eVehicleType GetVehicleType(int modelid);
    static void Delete(CVehicle * veh);
};