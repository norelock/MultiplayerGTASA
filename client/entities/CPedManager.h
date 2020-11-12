#pragma once

class CPedManager
{
    CPedManager();
public:
    static void Init();
	static CPed *Create(int modelid, const CVector& position, bool wander = false);
	static void Delete(CPed * ped);
};