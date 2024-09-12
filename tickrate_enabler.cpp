#include <eiface.h>
#include <sourcehook.h>
#include <sourcehook_impl.h>

class TickrateEnabler : public IServerPluginCallbacks
{
public:

	// IServerPluginCallbacks methods.
	virtual bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory);
	virtual void Unload();
	virtual void Pause() {}
	virtual void UnPause() {}
	virtual const char *GetPluginDescription();
	virtual void LevelInit(char const *pMapName) {}
	virtual void ServerActivate(edict_t *pEdictList, int edictCount, int clientMax) {}
	virtual void GameFrame(bool simulating) {}
	virtual void LevelShutdown() {}
	virtual void ClientActive(edict_t *pEntity) {}
	virtual void ClientDisconnect(edict_t *pEntity) {}
	virtual void ClientPutInServer(edict_t *pEntity, char const *playername) {}
	virtual void SetCommandClient(int index) {}
	virtual void ClientSettingsChanged(edict_t *pEdict) {}
	virtual PLUGIN_RESULT ClientConnect(bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen) { return PLUGIN_CONTINUE; }
	virtual PLUGIN_RESULT ClientCommand(edict_t *pEntity, const CCommand &args) { return PLUGIN_CONTINUE; }
	virtual PLUGIN_RESULT NetworkIDValidated(const char *pszUserName, const char *pszNetworkID) { return PLUGIN_CONTINUE; }
	virtual void OnQueryCvarValueFinished(QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue) {}

private:
	IServerGameDLL* serverGameDLL = nullptr;
};

// The plugin is a static singleton that is exported as an interface.
TickrateEnabler g_TickrateEnabler;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(TickrateEnabler, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_TickrateEnabler);

// Needed for SourceHook Standalone.
SourceHook::Impl::CSourceHookImpl g_SourceHook;
SourceHook::ISourceHook* g_SHPtr = &g_SourceHook;
int g_PLID = 0;

float g_TickInterval = DEFAULT_TICK_INTERVAL;

SH_DECL_HOOK0(IServerGameDLL, GetTickInterval, const, 0, float);

// Redefine GetTickInterval.
float GetTickInterval()
{
	RETURN_META_VALUE(MRES_SUPERCEDE, g_TickInterval);
}

// Called when the plugin is loaded, load the interface we need from the engine.
bool TickrateEnabler::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
	serverGameDLL = reinterpret_cast<IServerGameDLL*>(gameServerFactory(INTERFACEVERSION_SERVERGAMEDLL, nullptr));
	if (serverGameDLL == nullptr)
		return false;

	// Get tick interval from command line if we have it.
	if (CommandLine()->CheckParm("-tickrate")) {
		const float tickrate = CommandLine()->ParmValue("-tickrate", 0.0f); // We want float overload.
		g_TickInterval = 1.0f / tickrate;
		Msg("TickrateEnabler: Tickrate set to commandline value %f\n", tickrate);
	}
	else
		Msg("TickrateEnabler: Tickrate set to default value %f\n", 1.0 / DEFAULT_TICK_INTERVAL);

	SH_ADD_HOOK(IServerGameDLL, GetTickInterval, serverGameDLL, SH_STATIC(GetTickInterval), false);
	return true;
}

// Called when the plugin is unloaded (turned off).
void TickrateEnabler::Unload()
{
	SH_REMOVE_HOOK(IServerGameDLL, GetTickInterval, serverGameDLL, SH_STATIC(GetTickInterval), false);
}

// The name of this plugin, returned in "plugin_print" command.
const char* TickrateEnabler::GetPluginDescription()
{
	return "TickrateEnabler by Garamond";
}