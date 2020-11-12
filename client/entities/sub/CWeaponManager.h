#pragma once
class CWeaponManager
{
	CWeaponManager(){}
public:

    static void Init();

	static void GiveWeapon(CPed* ped, eWeaponType weapon, unsigned int ammo, bool armed)
	{
		CWeaponInfo * winfo = CWeaponInfo::GetWeaponInfo(weapon, 1);
		if (winfo->m_nModelId1 > 0)CModelManager::LoadModel(winfo->m_nModelId1);
		if (winfo->m_nModelId2 > 0)CModelManager::LoadModel(winfo->m_nModelId2);
		ped->GiveWeapon(weapon, ammo, false);
		ped->SetAmmo(weapon, ammo);
		if (armed)
		{
			ped->SetCurrentWeapon(weapon);
		}
	}
};