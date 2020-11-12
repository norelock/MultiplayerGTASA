#pragma once

class CNetworkManager
{
private:
	CNetworkManager();
public:

	static char m_szPlayerName[MAX_PLAYER_NAME];
	static char	m_szServerAddress[15];
	static int m_iServerPort;

	static bool m_bConnected;
	static bool m_bRunning;

	static librg_address_t m_addr;
	static librg_ctx_t m_ctx;

    static void Init();

	static void on_connect_request(librg_event_t *event);
	static void on_connect_accepted(librg_event_t *event);
	static void on_connect_refused(librg_event_t *event);
	static void on_disconnect(librg_event_t* event);

	static void on_register_player(librg_message_t *msg);
	static void on_unregister_player(librg_message_t *msg);

	static void on_entity_create(librg_event_t *event);
	static void on_entity_update(librg_event_t *event);
	static void on_entity_remove(librg_event_t *event);

	static void on_client_stream(librg_event_t *event);

	static void bullet_sync(librg_message_t *msg); 
	static void task_sync(librg_message_t* msg);
	static void new_script_event(librg_message_t* msg);
	static void script_event(librg_message_t* msg);
	static void close_script_event(librg_message_t* msg);

	static void client_thread();

	static void AttemptConnect(const char* szAddress, int iPort, const char* szName);
};