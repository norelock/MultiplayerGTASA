#define LIBRG_IMPLEMENTATION
#define ZPL_IMPLEMENTATION
#define ZPLM_IMPLEMENTATION
#define ENET_IMPLEMENTATION
#define LIBRG_DEBUG
#include "main.h"
#include "CTaskSimpleDuck.h"
#include "CTaskSimpleDuckToggle.h"
#include <thread>

char CNetworkManager::m_szPlayerName[MAX_PLAYER_NAME];
char CNetworkManager::m_szServerAddress[15];
int CNetworkManager::m_iServerPort;

bool CNetworkManager::m_bConnected;
bool CNetworkManager::m_bRunning;

librg_address_t CNetworkManager::m_addr;
librg_ctx_t CNetworkManager::m_ctx;

void CNetworkManager::Init()
{
	m_ctx = { 0 };
	m_bConnected = false;
	m_bRunning = false;

	Events::drawingEvent += []
	{
		CNetworkEntityManager::Process();
	};
}

void CNetworkManager::AttemptConnect(const char* szAddress, int iPort, const char* szName)
{
    m_bRunning = true;

    m_ctx.mode = LIBRG_MODE_CLIENT;
    librg_init(&m_ctx);
    m_ctx.tick_delay = 60;

    librg_event_add(&m_ctx, LIBRG_CONNECTION_REQUEST, on_connect_request);
    librg_event_add(&m_ctx, LIBRG_CONNECTION_ACCEPT, on_connect_accepted);
    librg_event_add(&m_ctx, LIBRG_CONNECTION_DISCONNECT, on_disconnect);

    librg_event_add(&m_ctx, LIBRG_ENTITY_CREATE, on_entity_create);
    librg_event_add(&m_ctx, LIBRG_ENTITY_UPDATE, on_entity_update);
    librg_event_add(&m_ctx, LIBRG_ENTITY_REMOVE, on_entity_remove);

    librg_event_add(&m_ctx, LIBRG_CLIENT_STREAMER_UPDATE, on_client_stream);

    librg_network_add(&m_ctx, eNetworkEvents::REGISTER_REMOTE_CLIENT, on_register_player);
    librg_network_add(&m_ctx, eNetworkEvents::UNREGISTER_REMOTE_CLIENT, on_unregister_player);
    librg_network_add(&m_ctx, eNetworkEvents::BULLET_SYNC_EVENT, bullet_sync);
	librg_network_add(&m_ctx, eNetworkEvents::PLAYER_TASK_EVENT, task_sync);
	librg_network_add(&m_ctx, eNetworkEvents::NEW_SCRIPT_EVENT, new_script_event);
	librg_network_add(&m_ctx, eNetworkEvents::SCRIPT_EVENT, script_event);
	librg_network_add(&m_ctx, eNetworkEvents::CLOSE_SCRIPT_EVENT, close_script_event);

    m_addr.host = (char*)szAddress;
    m_addr.port = iPort;

    strcpy(m_szPlayerName, szName);

    printf("[CNetworkManager] Attempting to connect to %s:%d\n", m_addr.host, m_addr.port);
    librg_network_start(&m_ctx, m_addr);

    //CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&client_thread, NULL, 0, NULL);
    static std::thread clientThread(client_thread);
}

void CNetworkManager::client_thread()
{
	while (m_bRunning)
	{
		librg_tick(&m_ctx);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	librg_network_stop(&m_ctx);
	librg_free(&m_ctx);
}

//for local player only
void CNetworkManager::on_connect_request(librg_event_t *event)
{
	librg_data_wstr(event->data, (void*)&m_szPlayerName, strlen(m_szPlayerName)+1);
	librg_data_wu32(event->data, DEFAULT_CONNECTION_SECRET);

	printf("[CNetworkManager] Connecting as %s\n", m_szPlayerName);
}

void CNetworkManager::on_connect_accepted(librg_event_t *event)
{
	//local network entity
	CClientPlayer* client = CNetworkEntityManager::CreatePlayer(event->entity->id, m_szPlayerName, true);

	event->entity->type = eNetworkEntityType::PLAYER;
	event->entity->user_data = client;

	struct
	{
		usize size;
		char str[MAX_PLAYER_NAME];
	} data;

	data.size = client->strName.length()+1;
	strcpy(data.str, client->strName.c_str());

	librg_message_send_all(&m_ctx, eNetworkEvents::FINISH_CONNECTION, &data, sizeof(data));

	printf("[CNetworkManager] Connection Accepted my network id is %d\n", client->iNetworkID);
}

void CNetworkManager::on_connect_refused(librg_event_t *event)
{
	m_bConnected = false;
	m_bRunning = false;

	printf("[CNetworkManager] Connection Refused\n");
}

void CNetworkManager::on_disconnect(librg_event_t *event)
{
	m_bConnected = false;
	m_bRunning = false;

	printf("[CNetworkManager] Connection Closed\n");
}

// processing remote entities
void CNetworkManager::on_register_player(librg_message_t *msg)
{
	PlayerJoinPacket data;
	librg_data_rptr(msg->data, &data, sizeof(PlayerJoinPacket));

	CClientPlayer* client = CNetworkEntityManager::CreatePlayer(data.iNetworkID, std::string(data.szPlayerName, data.iPlayerNameLen));

	printf("[CNetworkManager] Player %s(%d) is joined, added to player pool\n", client->strName.c_str(), data.iNetworkID);
}

void CNetworkManager::on_unregister_player(librg_message_t *msg)
{
	u32 networkID = librg_data_ru32(msg->data);
	CNetworkEntityManager::DisconnectPlayer(networkID);
	librg_entity_t * entity = librg_entity_fetch(msg->ctx, networkID);
	if(entity)entity->user_data = nullptr;
	printf("[CNetworkManager] Player %d left the server\n", networkID);
}

//entities
//streaming in an entity
void CNetworkManager::on_entity_create(librg_event_t *event)
{
	switch (event->entity->type)
	{
	case eNetworkEntityType::PLAYER:
	{
		CClientPlayer* player = (CClientPlayer*)CNetworkEntityManager::FindEntityByNetworkID(event->entity->id, eNetworkEntityType::PLAYER);
		if (player)
		{
			player->StreamIn();
			event->entity->user_data = player;
		}
		else printf("Something went wrong while streaming in a player\n");
		break;
	}
	}
}

//getting updated data of an entity
void CNetworkManager::on_entity_update(librg_event_t *event)
{
	if (!event->entity->user_data)return;

	switch (event->entity->type)
	{
	case eNetworkEntityType::PLAYER:
	{
		CClientPlayer* player = (CClientPlayer*)event->entity->user_data;
		if (player && player->m_bStreamedIn)
		{
			player->position = event->entity->position;
			player->m_iLastPacketTime = GetTickCount();

			player->syncType = (eSyncType)librg_data_ru8(event->data);


			int size;
            if (player->syncType == eSyncType::SYNC_TYPE_ON_FOOT) size = sizeof(OnfootPlayerSyncPacket);
            else if (player->syncType == eSyncType::SYNC_TYPE_ON_FOOT_AIMING)size = sizeof(OnfootPlayerAimingSyncPacket);
            else if (player->syncType == eSyncType::SYNC_TYPE_IN_WATER)size = sizeof(InWaterPlayerSyncPacket);

            

			librg_data_rptr(event->data, player->syncData, size);
			player->bSync = true;
		}
		break;
	}
	}
}

//streaming out an entity
void CNetworkManager::on_entity_remove(librg_event_t *event)
{
	switch (event->entity->type)
	{
	case eNetworkEntityType::PLAYER:
	{
		CClientPlayer* player = (CClientPlayer*)event->entity->user_data;
		if (player)
		{
			player->StreamOut();
			event->entity->user_data = nullptr;
		}
		break;
	}
	}
}

//sending data of entities who we have control over
void CNetworkManager::on_client_stream(librg_event_t *event)
{
	//we have control over own entity, we set it on the server side on finishing connection
	if (event->entity->type == eNetworkEntityType::PLAYER)
	{
		CClientPlayer* player = (CClientPlayer *)event->entity->user_data;
		CPed* ped = player->m_pPed;
		event->entity->position = *(zpl_vec3 *)&ped->GetPosition();
		
        player->syncType = player->BuildSyncData();


        int size;
        if (player->syncType == eSyncType::SYNC_TYPE_ON_FOOT) size = sizeof(OnfootPlayerSyncPacket);
        else if (player->syncType == eSyncType::SYNC_TYPE_ON_FOOT_AIMING)size = sizeof(OnfootPlayerAimingSyncPacket);
        else if (player->syncType == eSyncType::SYNC_TYPE_IN_WATER)size = sizeof(InWaterPlayerSyncPacket);

		librg_data_wu8(event->data, player->syncType);
		librg_data_wptr(event->data, player->syncData, size);
	}
}

void CNetworkManager::bullet_sync(librg_message_t* msg)
{
    BulletImpactSyncEvent ev;
	librg_data_rptr(msg->data, &ev, sizeof(BulletImpactSyncEvent));
	CNetworkEntityManager::localPlayer->ProcessBullet(&ev);
}

void CNetworkManager::task_sync(librg_message_t* msg)
{
	struct
	{
		uint32_t networkID;
		eTaskEventType type;
	} message;

	librg_data_rptr(msg->data, &message, sizeof(message));

	CClientPlayer* player = (CClientPlayer*)CNetworkEntityManager::FindEntityByNetworkID(message.networkID, eNetworkEntityType::PLAYER);
	if (player)
	{
		eTaskType activeType = player->m_pPed->m_pIntelligence->m_TaskMgr.GetActiveTask()->GetId();
		//eTaskType simplestActiveType = player->m_pIntelligence->m_TaskMgr.GetSimplestActiveTask()->GetId();

        if (message.type == eTaskEventType::TASK_EVENT_JUMP)
        {
            if (activeType != TASK_COMPLEX_JUMP && player->m_pPed->m_pIntelligence->GetTaskDuck(true))
            {
                CTaskSimpleDuckToggle duckToggle(0);
                duckToggle.ProcessPed(player->m_pPed);
            }
            else if (activeType != TASK_COMPLEX_JUMP)
            {
                player->m_pPed->m_pIntelligence->m_TaskMgr.SetTask(new CTaskComplexJump(0), 3, false);
            }
        }
        else if(message.type == eTaskEventType::TASK_EVENT_DUCK)
        {
            CTaskSimpleDuckToggle duckToggle(1);
            duckToggle.ProcessPed(player->m_pPed);
        }
	}

}

extern "C"
{
#include "Lua/include/lua.h"
#include "Lua/include/lualib.h"
#include "Lua/include/lauxlib.h"
}

#ifdef _WIN32
#pragma comment(lib, "Lua/lua5.1.lib")
#endif

struct Script {
	char* name;
	char* code;
	lua_State* LuaState;
};

char CurrentScript[1024];

Script Scripts[1024];

void CNetworkManager::new_script_event(librg_message_t* msg)
{
	struct
	{
		char CodeString[1024];
	} message;

	librg_data_rptr(msg->data, &message, sizeof(message));

	printf("Create new script: %s\n\n", message.CodeString);
	//CurrentScript = message.CodeString;
	memcpy(CurrentScript, message.CodeString, 1024);
}

void CNetworkManager::script_event(librg_message_t* msg)
{
	struct
	{
		char CodeString[1024];
	} message;

	librg_data_rptr(msg->data, &message, sizeof(message));

	printf("Received script packet to %s\n\n", CurrentScript);
}

void CNetworkManager::close_script_event(librg_message_t* msg)
{
	struct
	{
		char CodeString[1024];
	} message;

	librg_data_rptr(msg->data, &message, sizeof(message));

	printf("Close script %s\n", message.CodeString);
}