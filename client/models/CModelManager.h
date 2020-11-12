#pragma once
class CModelManager
{
	CModelManager(){}

public:

    static void Init();

	static bool IsModelLoaded(int modelid)
	{
		return CStreaming::ms_aInfoForModel[modelid].m_nLoadState == LOADSTATE_LOADED;
	}

	static bool LoadModel(int modelid)
	{
		bool loaded = IsModelLoaded(modelid);
		if (!loaded)
		{
			unsigned char oldFlags = CStreaming::ms_aInfoForModel[modelid].m_nFlags;
			CStreaming::RequestModel(modelid, GAME_REQUIRED | PRIORITY_REQUEST);
			CStreaming::LoadAllRequestedModels(true);

			loaded = IsModelLoaded(modelid);
			if (loaded)
			{
				if (!(oldFlags & GAME_REQUIRED))
				{
					CStreaming::SetModelIsDeletable(modelid);
					CStreaming::SetModelTxdIsDeletable(modelid);
				}
			}
		}
		return loaded;
	}
};