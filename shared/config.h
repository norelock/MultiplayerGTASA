#pragma once

#define STR_HELPER(x) #x
#define TOSTR(x) STR_HELPER(x)

// Versioning, SemVer
#define VERSION_MAJOR					0
#define VERSION_MINOR					1
#define VERSION_PATCH					1

#define VERSION_DATA					TOSTR(VERSION_MAJOR) "." TOSTR(VERSION_MINOR) "." TOSTR(VERSION_PATCH)
#define PROJECT_NAME					"Render Multiplayer"
#ifdef ALTSA_CLIENT						
#define PROJECT_TYPE					"client"
#else
#define PROJECT_TYPE					"server"
#endif
#define PROJECT_INFO					PROJECT_NAME " " PROJECT_TYPE " v" VERSION_DATA " - " __DATE__ " - " __TIME__ "\n"

// Default server tick delay
#define SERVER_TICK_DELAY				16

// Default max players
#define MAX_PLAYERS						100

// Default max objects
#define MAX_OBJECTS						1000

// Default max vehicles
#define MAX_VEHICLES					1000

// Default max peds
#define MAX_PEDS						1000

// Internal-use only
#define MAX_ENTITIES					(MAX_PEDS+MAX_OBJECTS+MAX_VEHICLES)

// Default max player name length
#define MAX_PLAYER_NAME					25

#define DEFAULT_CONNECTION_SECRET		420666


//additional network messages
enum eNetworkEvents
{
	REGISTER_REMOTE_CLIENT = LIBRG_EVENT_LAST,
	UNREGISTER_REMOTE_CLIENT,
	FINISH_CONNECTION,
	DEATH_EVENT,
	SPAWN_EVENT,
	PLAYER_TASK_EVENT,
	BULLET_SYNC_EVENT,
	SCRIPT_EVENT,
	NEW_SCRIPT_EVENT,
	CLOSE_SCRIPT_EVENT
};

enum eTaskEventType : uint8_t
{
	TASK_EVENT_JUMP,
	TASK_EVENT_DUCK
};