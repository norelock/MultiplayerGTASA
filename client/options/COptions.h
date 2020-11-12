#pragma once

class COptions
{
    COptions();
public:

    static void Init()
    {
        m_bLocalPlayerSpawned = false;
        m_iStartTime = 0;
        m_bEngineInitialized = false;
    }

	//bool GameReady;
	static bool m_bLocalPlayerSpawned;
	static int m_iStartTime;
	static bool m_bEngineInitialized;

	static bool IsGameReady()
	{
		if (m_iStartTime - CTimer::m_snTimeInMilliseconds >= 5000)return true;
		return false;
	}

};