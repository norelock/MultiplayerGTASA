#pragma once

class CNetworkManager
{
private:
    CNetworkManager();
public:
	

    static void Init();

#ifdef _MSC_VER
	static HANDLE server_handle;
#else
	static void* server_handle;
#endif
	static int iPort;
	static int iSecret;
	static bool bServerRunning;
	static librg_ctx_t ctx;

	static void on_connect_request(librg_event_t *event);
	static void on_connect_accepted(librg_event_t *event);
	static void on_creating_entity(librg_event_t *event);
	static void on_entity_update(librg_event_t *event);
	static void on_disconnect(librg_event_t* event);
	static void on_stream_update(librg_event_t *event);
	static void on_entity_remove(librg_event_t *event);

	//custom events
	static void finish_connection(librg_message_t* msg);
	static void bullet_sync(librg_message_t* msg);
	static void task_sync(librg_message_t* msg);

	static void SendResourceScripts(librg_peer* receive);

#if defined(_MSC_VER)
	static void server_thread();
#else 
	static void *server_thread(void* p);
#endif
};
