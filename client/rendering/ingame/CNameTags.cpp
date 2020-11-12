#include "main.h"

void CNameTags::Draw()
{
	int size = CNetworkEntityManager::m_pPlayers.size();
	if (size <= 0)
		return;


	for (auto it : CNetworkEntityManager::m_pPlayers)
	{
		CClientPlayer * player = (CClientPlayer*)it;
		CPed * ped = player->m_pPed;
		if (ped && ped != FindPlayerPed(0) && player->m_bStreamedIn)
		{
			CVector camPos = TheCamera.m_vecGameCamPos;
			RwV3d posn;

			//if (!CWorld::GetIsLineOfSightClear(camPos, posn, true, false, false, false, false, true, false))continue;
			if (!ped->m_bIsVisible || ped->m_bOffscreen)continue;

			/*float distance = DistanceBetweenPoints(camPos, posn);
			float above = distance * 0.25f;
			if (distance <= 5)above = 1.25f;
			if(distance > 10)above = above / (1.25 * 1.45);*/

			float above = 1.25f;
			if (ped->m_fArmour > 0)above = 1.5f;
			ped->GetBonePosition(posn, 8, false);
			RwV3d screenCoors; float w, h;
			if (CSprite::CalcScreenCoors({ posn.x, posn.y, posn.z + 0.5f }, &screenCoors, &w, &h, false, false))
			{
				char text[MAX_PLAYER_NAME+11+2];
				if (ped->IsPlayer())
				{
					sprintf(text, "%s(%d)", player->strName.c_str(), player->iNetworkID);

					SIZE size = CRenderManager::MeasureText(text);

                    CRenderManager::RenderText(text, { (LONG)screenCoors.x - (LONG)ceil(size.cx / 2.0), (LONG)screenCoors.y - 20 }, 0xFFFFFFFF);

					/*if (ped->m_fArmour > 0)
					{
						CRenderManager::DrawProgressBar(CRect(screenCoors.x - 50, screenCoors.y - 5 + 25, screenCoors.x + 50, screenCoors.y + 5 + 25), (int)ped->m_fArmour, CRGBA(185, 185, 185, 255));
						CRenderManager::DrawProgressBar(CRect(screenCoors.x - 50, screenCoors.y - 5 + 40, screenCoors.x + 50, screenCoors.y + 5 + 40), (int)ped->m_fHealth, CRGBA(255, 150, 225, 255));
					}
					else
					{
                        CRenderManager::DrawProgressBar(CRect(screenCoors.x - 20, screenCoors.y - 3, screenCoors.x + 20, screenCoors.y + 2), (int)ped->m_fHealth, CRGBA(255, 0, 0, 255));
					}*/

					//CSprite2d::DrawRect(CRect(screenCoors.x - 60, screenCoors.y - 5 + 30, screenCoors.x + 60, screenCoors.y + 5 + 30), CRGBA(0, 0, 0, 255));
					//CSprite2d::DrawRect(CRect(screenCoors.x - 60 + 3, screenCoors.y - 5 + 30 + 1, screenCoors.x + 60 - 3, screenCoors.y + 5 + 30 - 1), CRGBA(255, 95, 225, 255));
					//CSprite2d::DrawRect(CRect(screenCoors.x - 60 + 3, screenCoors.y - 5 + 30 + 1, (screenCoors.x - 60 + 3) + (((screenCoors.x + 60 - 3) - (screenCoors.x - 60 + 3)) / 100.0f)*ped->m_fHealth, screenCoors.y + 5 + 30 - 1), CRGBA(255, 150, 225, 255));
				}
			}
		}
	}
}