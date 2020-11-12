#define LIBRG_IMPLEMENTATION
#define ZPL_IMPLEMENTATION
#define ZPLM_IMPLEMENTATION
#define ENET_IMPLEMENTATION
#define LIBRG_DEBUG
#include "server.h"
#include <thread>

#ifdef _MSC_VER
HANDLE CNetworkManager::server_handle;
#else
void* CNetworkManager::server_handle;
#endif
int CNetworkManager::iPort;
int CNetworkManager::iSecret;
bool CNetworkManager::bServerRunning;
librg_ctx_t CNetworkManager::ctx;

void CNetworkManager::Init()
{
	iSecret = DEFAULT_CONNECTION_SECRET;
	bServerRunning = true;
	ctx = { 0 };
	iPort = 7766;

#if defined(_MSC_VER)
	//server_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)server_thread, NULL, 0, NULL);
    static std::thread serverThread(server_thread);
#else
	pthread_t thread;
	double threadid;
	pthread_create(&thread, NULL, server_thread, &threadid);
#endif
}

#if defined (_MSC_VER)
void CNetworkManager::server_thread()
#else 
void *CNetworkManager::server_thread(void* p)
#endif
{
	ctx.world_size = zpl_vec3f(5000.0f, 5000.0f, 5000.0f);
	ctx.mode = LIBRG_MODE_SERVER;
	ctx.tick_delay = 60;
	ctx.max_connections = (MAX_PLAYERS * 2);
	ctx.max_entities = (MAX_ENTITIES + MAX_PLAYERS);
	librg_init(&ctx);

	//librg_option_set(LIBRG_DEFAULT_STREAM_RANGE, 10);

	librg_event_add(&ctx, LIBRG_CONNECTION_REQUEST, on_connect_request);
	librg_event_add(&ctx, LIBRG_CONNECTION_ACCEPT, on_connect_accepted);
	librg_event_add(&ctx, LIBRG_CONNECTION_DISCONNECT, on_disconnect);

	librg_event_add(&ctx, LIBRG_ENTITY_CREATE, on_creating_entity);
	librg_event_add(&ctx, LIBRG_ENTITY_UPDATE, on_entity_update);
	librg_event_add(&ctx, LIBRG_ENTITY_REMOVE, on_entity_remove);

	librg_event_add(&ctx, LIBRG_CLIENT_STREAMER_UPDATE, on_stream_update);

	librg_network_add(&ctx, eNetworkEvents::FINISH_CONNECTION, finish_connection);
	librg_network_add(&ctx, eNetworkEvents::BULLET_SYNC_EVENT, bullet_sync);
	librg_network_add(&ctx, eNetworkEvents::PLAYER_TASK_EVENT, task_sync);

	CLog::Log("[CNetworkManager][INFO] Server thread initialized\n");

	librg_address_t addr = { 7766 };
	librg_network_start(&ctx, addr);
	CLog::Log("[CNetworkManager][INFO] Server started on port %d\n", addr.port);

	while (bServerRunning) 
	{
		librg_tick(&ctx);
#ifdef _MSC_VER
		//Sleep(10);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
#else
		usleep(10);
#endif
	}

	librg_network_stop(&ctx);
	librg_free(&ctx);
}

// LIBRG_CONNECTION_REQUEST
// a client is requesting to connect, if someone with this name is already connected, reject
void CNetworkManager::on_connect_request(librg_event_t *event)
{
	// Player Name
	char name[MAX_PLAYER_NAME];
	strcpy(name, librg_data_rstr(event->data).c_str());

	u32 secret = librg_data_ru32(event->data);
	CLog::Log("[CNetworkManager] Client with name %s is requesting to connect\n", name);

	if (secret != iSecret)
	{
		CLog::Log("[CNetworkManager] Invalid server secret.\n");
		librg_event_reject(event);
	}
}

// LIBRG_CONNECTION_ACCEPT
// server accepts the connecting request, then responding to the client with the same event
// create player instance, add to array, init user_data
void CNetworkManager::on_connect_accepted(librg_event_t *event)
{
	event->entity->type = eNetworkEntityType::PLAYER;
	CLog::Log("[CNetworkManager] Network client %d connected, waiting for response\n", event->entity->id);
}

std::vector<std::string> strnsplit(const std::string str, size_t n);
std::vector<std::string> strnsplit(const std::string str, size_t n) {
	if (n == 0) {
		exit(1);
	}
	const size_t input_len = str.length() + 1;
	const size_t output_len = (input_len / n) + (input_len % n != 0);
	std::vector<std::string> ret(output_len, std::string(n, 0));
	size_t i = 0;
	while (i < output_len) {
		size_t j = 0;
		while (j < n) {
			ret[i][j] = str[(i * n) + j - 1];
			j++;
		}
		i++;
	}
	return ret;
}

void CNetworkManager::SendResourceScripts(librg_peer* receive)
{
	struct
	{
		char CodeString[1024];
	} message;

	const char *codeToSend = "printf('siema')" \
	"printf('hello world from lua')" \
	"-- Pixel Perfect Studios!" \
	"a = 3" \
	"b = 2" \
	"c = a + b" \
	"printf(c)";

	strcpy(message.CodeString, "test.lua");
	librg_message_send_to(&ctx, eNetworkEvents::NEW_SCRIPT_EVENT, receive, &message, sizeof(message));

	for (std::string s : strnsplit(codeToSend, 25))
	{
		strcpy(message.CodeString, s.c_str());
		librg_message_send_to(&ctx, eNetworkEvents::SCRIPT_EVENT, receive, &message, sizeof(message));
	}

	strcpy(message.CodeString, "test.lua");
	librg_message_send_to(&ctx, eNetworkEvents::CLOSE_SCRIPT_EVENT, receive, &message, sizeof(message));
}

void CNetworkManager::finish_connection(librg_message_t* msg)
{
	//get name here
	char name[MAX_PLAYER_NAME];
	strcpy(name, librg_data_rstr(msg->data).c_str());

	librg_entity_t* entity = librg_entity_find(msg->ctx, msg->peer);

	CServerPlayer* player = new CServerPlayer();
	player->iNetworkID = entity->id;
	player->strName = name;
	player->client_peer = entity->client_peer;
	entity->type = player->type = eNetworkEntityType::PLAYER;

	PlayerJoinPacket data;

	//Register all other players for our player
    for (auto& pl : CNetworkEntityManager::players)
	{
        if (!pl) continue;
        data.iNetworkID = pl->iNetworkID;
        data.iPlayerNameLen = pl->strName.size() + 1;
        strcpy(data.szPlayerName, pl->strName.c_str());
        librg_message_send_to(&ctx, eNetworkEvents::REGISTER_REMOTE_CLIENT, entity->client_peer, &data, sizeof(PlayerJoinPacket));
	}

	CNetworkEntityManager::players.push_back(player);
	entity->user_data = player;

	librg_entity_control_set(msg->ctx, entity->id, entity->client_peer);

	//Register our player for all other players
	data.iNetworkID = entity->id;
	data.iPlayerNameLen = player->strName.size() + 1;
	strcpy(data.szPlayerName, name);

	librg_message_send_except(&ctx, eNetworkEvents::REGISTER_REMOTE_CLIENT, entity->client_peer, &data, sizeof(PlayerJoinPacket));

	SendResourceScripts(entity->client_peer);

	CLog::Log("[CNetworkManager] Network client %s(%d) finished the connection\n", name, entity->id);
}

// LIBRG_CONNECTION_DISCONNECT
// client disconnects or timed out, no data sending
void CNetworkManager::on_disconnect(librg_event_t* event)
{
	CServerPlayer* player = (CServerPlayer*)event->entity->user_data;
	if (player) //if its null, the player is disconnected before connection finished
	{
        CNetworkEntityManager::players.erase(std::remove(CNetworkEntityManager::players.begin(), CNetworkEntityManager::players.end(), player), CNetworkEntityManager::players.end());

		delete player;
		event->entity->user_data = nullptr;
		player = nullptr;

		librg_entity_control_remove(event->ctx, event->entity->id);

		librg_message_send_except(&ctx, eNetworkEvents::UNREGISTER_REMOTE_CLIENT, event->entity->client_peer, &event->entity->id, sizeof(u32));
	}
	CLog::Log("[CNetworkManager] Network client %d disconnected from server.\n", event->entity->id);
}

// LIBRG_ENTITY_CREATE
// triggers when an entity enters a player's stream zone
// init sync data here
void CNetworkManager::on_creating_entity(librg_event_t *event)
{

}

// LIBRG_ENTITY_UPDATE
// gets triggered every time an entity is found within streamzone of a client's entity and was already created
// event can be rejected if the data was not changed
void CNetworkManager::on_entity_update(librg_event_t *event)
{
	if (!event->entity->user_data)
		return;

	if (event->entity->type == eNetworkEntityType::PLAYER)
	{
		CServerPlayer* player = (CServerPlayer*)event->entity->user_data;

		int size;
		if (player->syncType == eSyncType::SYNC_TYPE_ON_FOOT) size = sizeof(OnfootPlayerSyncPacket);
		else if (player->syncType == eSyncType::SYNC_TYPE_ON_FOOT_AIMING)size = sizeof(OnfootPlayerAimingSyncPacket);
		else if (player->syncType == eSyncType::SYNC_TYPE_IN_WATER)size = sizeof(InWaterPlayerSyncPacket);

		if (memcmp(player->lastSyncData, player->syncData, size) == 0)
		{
			librg_event_reject(event);
			return;
		}

		librg_data_wu8(event->data, player->syncType);
		librg_data_wptr(event->data, player->syncData, size);
	}
}

// LIBRG_ENTITY_REMOVE
// entity streamed out for entity
void CNetworkManager::on_entity_remove(librg_event_t *event) 
{

}

// LIBRG_CLIENT_STREAMER_UPDATE
// server received a sync packet from an entity
void CNetworkManager::on_stream_update(librg_event_t *event)
{
	if (!event->entity->user_data)
		return;

	if (event->entity->type == eNetworkEntityType::PLAYER)
	{
		CServerPlayer* player = (CServerPlayer*)event->entity->user_data;

		player->syncType = (eSyncType)librg_data_ru8(event->data);

		int size;
		if (player->syncType == eSyncType::SYNC_TYPE_ON_FOOT) size = sizeof(OnfootPlayerSyncPacket);
		else if (player->syncType == eSyncType::SYNC_TYPE_ON_FOOT_AIMING)size = sizeof(OnfootPlayerAimingSyncPacket);
		else if (player->syncType == eSyncType::SYNC_TYPE_IN_WATER)size = sizeof(InWaterPlayerSyncPacket);

        memcpy(player->lastSyncData, player->syncData, size);

		librg_data_rptr(event->data, player->syncData, size);
	}
}

void CNetworkManager::bullet_sync(librg_message_t* msg)
{
	BulletImpactSyncEvent ev;
	librg_data_rptr(msg->data, &ev, sizeof(BulletImpactSyncEvent));
    librg_message_send_instream_except(msg->ctx, eNetworkEvents::BULLET_SYNC_EVENT, ev.ownerNetworkID, msg->peer, &ev, sizeof(BulletImpactSyncEvent));
}

void CNetworkManager::task_sync(librg_message_t* msg)
{
	librg_entity_t* entity = librg_entity_find(msg->ctx, msg->peer);
	struct
	{
		uint32_t networkID;
		eTaskEventType type;
	} message;

	message.networkID = entity->id;

	librg_data_rptr(msg->data, &message.type, sizeof(eTaskEventType));
	librg_message_send_instream_except(msg->ctx, eNetworkEvents::PLAYER_TASK_EVENT, entity->id,msg->peer, &message, sizeof(message));
}