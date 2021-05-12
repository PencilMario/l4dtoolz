#include "l4dtoolz_mm.h"
#include "game_signature.h"

l4dtoolz g_l4dtoolz;
IVEngineServer *engine = NULL;
ICvar *icvar = NULL;

#ifdef L4D1
void *l4dtoolz::max_players_friend_lobby = NULL;
void *l4dtoolz::chuman_limit = NULL;
void *l4dtoolz::friends_lobby_org = NULL;
void *l4dtoolz::human_limit_org = NULL;
#endif
void *l4dtoolz::max_players_connect = NULL;
void *l4dtoolz::max_players_server_browser = NULL;
void *l4dtoolz::lobby_sux_ptr = NULL;
void *l4dtoolz::tmp_player = NULL;
void *l4dtoolz::tmp_player2 = NULL;
void *l4dtoolz::unreserved_ptr = NULL;
void *l4dtoolz::lobby_match_ptr = NULL;
void *l4dtoolz::max_players_org = NULL;
void *l4dtoolz::server_bplayers_org = NULL;
void *l4dtoolz::lobby_sux_org = NULL;
void *l4dtoolz::players_org = NULL;
void *l4dtoolz::players_org2 = NULL;
void *l4dtoolz::unreserved_org = NULL;
void *l4dtoolz::lobby_match_org = NULL;

ConVar sv_maxplayers("sv_maxplayers", "-1", 0, "Max Human Players", true, -1, true, 32, l4dtoolz::OnChangeMaxplayers);
#ifdef L4D1
ConVar sv_removehumanlimit("sv_removehumanlimit", "0", 0, "Remove Human limit reached kick", true, 0, true, 1, l4dtoolz::OnChangeRemovehumanlimit);
#endif
ConVar sv_force_unreserved("sv_force_unreserved", "0", 0, "Disallow lobby reservation cookie", true, 0, true, 1, l4dtoolz::OnChangeUnreserved);

void l4dtoolz::OnChangeMaxplayers(IConVar *var, const char *pOldValue, float flOldValue){
	int new_value = ((ConVar *)var)->GetInt();
	int old_value = atoi(pOldValue);
	if(new_value==old_value) return;
#ifdef L4D1
	if(!max_players_friend_lobby || !max_players_connect || !max_players_server_browser || !lobby_sux_ptr){
#else
	if(!max_players_connect || !max_players_server_browser || !lobby_sux_ptr){
#endif
		Msg("sv_maxplayers init error\n");
		return;
	}
	if(new_value>=0){
	#ifdef L4D1
		max_players_new[4] = friends_lobby_new[3] = server_bplayers_new[3] = new_value;
	#else
		max_players_new[4] = server_bplayers_new[3] = (unsigned char)new_value;
	#endif
		if(lobby_match_ptr){
			lobby_match_new[2] = (unsigned char)new_value;
			write_signature(lobby_match_ptr, lobby_match_new);
		}else{
			Msg("sv_maxplayers MS init error\n");
		}
	#ifdef L4D1
		write_signature(max_players_friend_lobby, friends_lobby_new);
	#endif
		write_signature(max_players_connect, max_players_new);
		write_signature(lobby_sux_ptr, lobby_sux_new);
		write_signature(max_players_server_browser, server_bplayers_new);
	}else{
	#ifdef L4D1
		write_signature(max_players_friend_lobby, friends_lobby_org);
	#endif
		write_signature(max_players_connect, max_players_org);
		write_signature(lobby_sux_ptr, lobby_sux_org);
		write_signature(max_players_server_browser, server_bplayers_org);
		if(lobby_match_ptr) write_signature(lobby_match_ptr, lobby_match_org);
	}
}

#ifdef L4D1
void l4dtoolz::OnChangeRemovehumanlimit(IConVar *var, const char *pOldValue, float flOldValue){
	int new_value = ((ConVar *)var)->GetInt();
	int old_value = atoi(pOldValue);
	if(new_value==old_value) return;
	if(!chuman_limit){
		Msg("sv_removehumanlimit init error\n");
		return;
	}
	if(new_value==1) write_signature(chuman_limit, human_limit_new);
	else write_signature(chuman_limit, human_limit_org);
}
#endif

void l4dtoolz::OnChangeUnreserved(IConVar *var, const char *pOldValue, float flOldValue){
	int new_value = ((ConVar *)var)->GetInt();
	int old_value = atoi(pOldValue);
	if(new_value==old_value) return;
	if(!unreserved_ptr){
		Msg("unreserved_ptr init error\n");
		return;
	}
	if(new_value==1){
		write_signature(unreserved_ptr, unreserved_new);
		engine->ServerCommand("sv_allow_lobby_connect_only 0\n");
	}else{
		write_signature(unreserved_ptr, unreserved_org);
	}
}

class BaseAccessor: public IConCommandBaseAccessor{
public:
	bool RegisterConCommandBase(ConCommandBase *pCommandBase){
		return META_REGCVAR(pCommandBase);
	}
} s_BaseAccessor;

PLUGIN_EXPOSE(l4dtoolz, g_l4dtoolz);

bool l4dtoolz::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late){
	PLUGIN_SAVEVARS();
	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
#if SOURCE_ENGINE >= SE_ORANGEBOX
	g_pCVar = icvar;
	ConVar_Register(0, &s_BaseAccessor);
#else
	ConCommandBaseMgr::OneTimeInit(&s_BaseAccessor);
#endif
	struct base_addr_t base_addr;
	base_addr.addr = NULL;
	base_addr.len = 0;

	find_base_from_list(matchmaking_dll, &base_addr);
	if(!lobby_match_ptr){
		lobby_match_ptr = find_signature(lobby_match, &base_addr, 1);
		get_original_signature(lobby_match_ptr, lobby_match_new, lobby_match_org);
	}

	find_base_from_list(engine_dll, &base_addr);
#ifdef L4D1
	if(!max_players_friend_lobby){
		max_players_friend_lobby = find_signature(friends_lobby, &base_addr, 0);
		get_original_signature(max_players_friend_lobby, friends_lobby_new, friends_lobby_org);
	}
#endif
	if(!max_players_connect){
		max_players_connect = find_signature(max_players, &base_addr, 0);
		get_original_signature(max_players_connect, max_players_new, max_players_org);
	}
	if(!lobby_sux_ptr){
	#ifdef WIN32
		lobby_sux_ptr = max_players_connect;
	#else
		lobby_sux_ptr = find_signature(lobby_sux, &base_addr, 0);
	#endif
		get_original_signature(lobby_sux_ptr, lobby_sux_new, lobby_sux_org);
	}
#ifdef L4D1
#ifdef WIN32
	if(!max_players_server_browser){
		max_players_server_browser = find_signature(server_bplayers, &base_addr, 0);
		get_original_signature(max_players_server_browser, server_bplayers_new, server_bplayers_org);
	}
#endif
#endif
	if(!tmp_player){
		tmp_player = find_signature(players, &base_addr, 0);
		if(tmp_player){
			tmp_player2 = find_signature(players2, &base_addr, 0);
			if(tmp_player2){
				get_original_signature(tmp_player, players_new, players_org);
				get_original_signature(tmp_player2, players_new2, players_org2);
				write_signature(tmp_player, players_new);
				write_signature(tmp_player2, players_new2);
				engine->ServerCommand("maxplayers 32\n");
			}
		}
	}
	if(!unreserved_ptr){
		unreserved_ptr = find_signature(unreserved, &base_addr, 0);
		get_original_signature(unreserved_ptr, unreserved_new, unreserved_org);
	}

	find_base_from_list(server_dll, &base_addr);
#ifdef L4D1
	if(!chuman_limit){
		chuman_limit = find_signature(human_limit, &base_addr, 0);
		get_original_signature(chuman_limit, human_limit_new, human_limit_org);
	}
	#ifndef WIN32
	if(!max_players_server_browser){
		max_players_server_browser = find_signature(server_bplayers, &base_addr, 0);
		get_original_signature(max_players_server_browser, server_bplayers_new, server_bplayers_org);
	}
	#endif
#else
	if(!max_players_server_browser){
		max_players_server_browser = find_signature(server_bplayers, &base_addr, 0);
		get_original_signature(max_players_server_browser, server_bplayers_new, server_bplayers_org);
	}
#endif
	return true;
}

bool l4dtoolz::Unload(char *error, size_t maxlen){
#ifdef L4D1
	safe_free(max_players_friend_lobby, friends_lobby_org);
	safe_free(chuman_limit, human_limit_org);
#endif
	safe_free(max_players_connect, max_players_org);
	safe_free(lobby_sux_ptr, lobby_sux_org);
	safe_free(max_players_server_browser, server_bplayers_org);
	safe_free(tmp_player, players_org);
	safe_free(tmp_player2, players_org2);
	safe_free(unreserved_ptr, unreserved_org);
	safe_free(lobby_match_ptr, lobby_match_org);
	return true;
}

const char *l4dtoolz::GetAuthor(){
	return "Ivailosp, lakwsh";
}

const char *l4dtoolz::GetName(){
	return "L4DToolZ";
}

const char *l4dtoolz::GetDescription(){
	return "";
}

const char *l4dtoolz::GetURL(){
	return "";
}

const char *l4dtoolz::GetLicense(){
	return "";
}

const char *l4dtoolz::GetVersion(){
	return "1.0.0.11";
}

const char *l4dtoolz::GetDate(){
	return __DATE__;
}

const char *l4dtoolz::GetLogTag(){
	return "L4DToolZ";
}