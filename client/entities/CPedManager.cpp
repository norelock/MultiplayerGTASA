#include "main.h"

void CPedManager::Init() {}

CPed *CPedManager::Create(int modelid, const CVector& position, bool wander)
{
	if (CModelManager::LoadModel(modelid))
	{
		CPed* ped = new CCivilianPed(CPopulation::IsFemale(modelid) ? PED_TYPE_CIVFEMALE : PED_TYPE_CIVMALE, modelid);
		ped->SetPosn(position);
		ped->SetOrientation(0.0f, 0.0f, 0.0f);
		CWorld::Add(ped);
		if (wander)
		{
			CTask * wanderTask = plugin::CallAndReturn<CTask*, 0x673D00, CPed*>(ped);
			if (wanderTask)ped->m_pIntelligence->m_TaskMgr.SetTask(wanderTask, 4, false);
		}
		return ped;
	}
	return nullptr;
}

void CPedManager::Delete(CPed* ped)
{
	CWorld::Remove(ped);
	operator_delete<CPed>(ped);
}