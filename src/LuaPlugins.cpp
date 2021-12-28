#include "../include/LuaPlugins.hpp"
#include "../include/Server.hpp"
#include "../include/Utils/Vector.hpp"

#include <iostream>
#include <vector>
#include <map>
#include <filesystem>

#undef SendMessage

namespace LuaPlugins {
	std::vector<sol::function> authEvent;
	std::vector<sol::function> messageEvent;
	std::vector<sol::function> joinEvent;
	std::vector<sol::function> positionOrientationEvent;
	std::vector<sol::function> setBlockEvent;
	std::vector<sol::function> disconnectEvent;
	std::vector<sol::function> playerClickedEvent;
	std::vector<sol::function> extEntryEvent;
} // namespace LuaPlugins

void LuaPlugin::Init()
{
	m_lua->script("function include(filename) dofile(\"plugins/" + m_name + "/\" .. filename) end", m_env);
	m_lua->script_file(m_filename, m_env);

	sol::function init = m_env["Init"];
	if (!init.valid())
		throw std::runtime_error(std::string("LuaPlugin: " + m_name + " (" + m_filename + ") Init function not found"));

	sol::function tick = m_env["Tick"];
	if (tick.valid())
		m_tick = tick;

	try {
		init();
	}
	catch (std::runtime_error& e) {
		std::cout << e.what() << std::endl;
	}
}

void LuaPlugin::Tick()
{
	if (m_tick != sol::nil)
		m_tick();
}

void PluginHandler::InitLua()
{
	m_lua = std::make_shared<sol::state>();
	m_lua->open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::io, sol::lib::table, sol::lib::math, sol::lib::os, sol::lib::debug);

	m_lua->new_usertype<Net::Client>("Client",
		"GetSID", &Net::Client::GetSID,
		"QueuePacket", &Net::Client::QueuePacket
		);

	m_lua->new_usertype<Utils::MCString>("MCString",
		sol::constructors<Utils::MCString(const std::string&)>()
		);

	m_lua->new_usertype<Player>("Player",
		"GetClient", &Player::GetClient,
		"GetName", &Player::GetName,
		"GetWorld", &Player::GetWorld,
		"GetPID", &Player::GetPID,
		"GetPosition", &Player::GetPosition,
		"SetPosition", &Player::SetPosition,
		"GetHeldBlock", &Player::GetHeldBlock,
		"SetHotbarSlot", &Player::SetHotbarSlot,
		"SetInventoryOrder", &Player::SetInventoryOrder,
		"SendMessage", &Player::SendMessage
		);

	m_lua->new_usertype<World>("World",
		"AddPlayer", &World::AddPlayer,
		"RemovePlayer", &World::RemovePlayer,
		"GetName", &World::GetName,
		"GetPlayers", &World::GetPlayers,
		"GetSpawnPosition", &World::GetSpawnPosition,
		"GetMap", &World::GetMap,
		"GetWeatherType", &World::GetWeatherType,
		"SetSpawnPosition", &World::SetSpawnPosition,
		"SetMap", &World::SetMap,
		"SetWeatherType", &World::SetWeatherType,
		"SendWeatherType", &World::SendWeatherType
		);

	m_lua->new_usertype<Map>("Map",
		"PeekBlock", &Map::PeekBlock,
		"LoadFromFile", &Map::LoadFromFile
		);

	m_lua->new_usertype<Utils::Vector>("Vector",
		sol::constructors<Utils::Vector(float, float, float)>(),
		"Normalized", &Utils::Vector::Normalized,
		"x", sol::property(&Utils::Vector::SetX, &Utils::Vector::GetX),
		"y", sol::property(&Utils::Vector::SetY, &Utils::Vector::GetY),
		"z", sol::property(&Utils::Vector::SetZ, &Utils::Vector::GetZ)
		);

	m_lua->new_usertype<Net::Packet>("Packet");

	(*m_lua)["GetPlugin"] = [&](std::string name) {
		auto plugin = dynamic_cast<LuaPlugin*>(GetPlugin(name));
		assert(plugin != nullptr);
		return plugin->GetEnv();
	};

	m_lua->set_function("RegisterEvent", [&](std::string name, sol::function func)
		{
			if (name == "OnAuthentication")
				LuaPlugins::authEvent.push_back(func);
			else if (name == "OnMessage")
				LuaPlugins::messageEvent.push_back(func);
			else if (name == "OnJoin")
				LuaPlugins::joinEvent.push_back(func);
			else if (name == "OnPositionOrientation")
				LuaPlugins::positionOrientationEvent.push_back(func);
			else if (name == "OnSetBlock")
				LuaPlugins::setBlockEvent.push_back(func);
			else if (name == "OnDisconnect")
				LuaPlugins::disconnectEvent.push_back(func);
			else if (name == "OnPlayerClicked")
				LuaPlugins::playerClickedEvent.push_back(func);
			else if (name == "OnExtEntry")
				LuaPlugins::extEntryEvent.push_back(func);
			else
				LOG(LOGLEVEL_WARNING, "RegisterEvent: Invalid event %s", name.c_str());
		}
	);

	(*m_lua)["ReloadPlugins"] = [&]() { m_reloadPlugins = true; };
	(*m_lua)["BlockDefaultEventHandler"] = [&]() { Server::GetInstance()->BlockDefaultEventHandler(); };
	(*m_lua)["GetWorld"] = [&](std::string name) { return Server::GetInstance()->GetWorld(name); };
	(*m_lua)["GetWorlds"] = [&]() { return Server::GetInstance()->GetWorlds(); };
	(*m_lua)["GetPlayer"] = [&](int8_t pid) { return Server::GetInstance()->GetPlayer(pid); };
	(*m_lua)["GivePrivilege"] = [&](std::string name, std::string priv) { return Server::GetInstance()->GetPrivilegeHandler().GivePrivilege(name, priv); };
	(*m_lua)["TakePrivilege"] = [&](std::string name, std::string priv) { return Server::GetInstance()->GetPrivilegeHandler().TakePrivilege(name, priv); };
	(*m_lua)["BroadcastMessage"] = [&](std::string message, int messageType) { Server::GetInstance()->BroadcastMessage(message, messageType); };
	(*m_lua)["MakeMap"] = [&]() { return std::make_shared<Map>(); };

	/* BEGIN AUTOGENERATED CODE SECTION */
	// ClassicProtocol
	(*m_lua)["MakePositionOrientationPacket"] = &Net::ClassicProtocol::MakePositionOrientationPacket;
	(*m_lua)["MakeOrientationPacket"] = &Net::ClassicProtocol::MakeOrientationPacket;
	(*m_lua)["MakeDespawnPacket"] = &Net::ClassicProtocol::MakeDespawnPacket;
	(*m_lua)["MakeMessagePacket"] = &Net::ClassicProtocol::MakeMessagePacket;
	(*m_lua)["MakeServerIdentificationPacket"] = &Net::ClassicProtocol::MakeServerIdentificationPacket;
	(*m_lua)["MakeLevelInitializePacket"] = &Net::ClassicProtocol::MakeLevelInitializePacket;
	(*m_lua)["MakeLevelDataChunkPacket"] = &Net::ClassicProtocol::MakeLevelDataChunkPacket;
	(*m_lua)["MakeLevelFinalizePacket"] = &Net::ClassicProtocol::MakeLevelFinalizePacket;
	(*m_lua)["MakeSetBlock2Packet"] = &Net::ClassicProtocol::MakeSetBlock2Packet;
	(*m_lua)["MakeSpawnPlayerPacket"] = &Net::ClassicProtocol::MakeSpawnPlayerPacket;
	(*m_lua)["MakeUserTypePacket"] = &Net::ClassicProtocol::MakeUserTypePacket;
	(*m_lua)["MakeDisconnectPlayerPacket"] = &Net::ClassicProtocol::MakeDisconnectPlayerPacket;

	// ExtendedProtocol
	(*m_lua)["MakeExtInfoPacket"] = &Net::ExtendedProtocol::MakeExtInfoPacket;
	(*m_lua)["MakeExtEntryPacket"] = &Net::ExtendedProtocol::MakeExtEntryPacket;
	(*m_lua)["MakeSetClickDistancePacket"] = &Net::ExtendedProtocol::MakeSetClickDistancePacket;
	(*m_lua)["MakeCustomBlocksPacket"] = &Net::ExtendedProtocol::MakeCustomBlocksPacket;
	(*m_lua)["MakeHoldThisPacket"] = &Net::ExtendedProtocol::MakeHoldThisPacket;
	(*m_lua)["MakeSetTextHotKeyPacket"] = &Net::ExtendedProtocol::MakeSetTextHotKeyPacket;
	(*m_lua)["MakeExtAddPlayerNamePacket"] = &Net::ExtendedProtocol::MakeExtAddPlayerNamePacket;
	(*m_lua)["MakeExtAddEntity2Packet"] = &Net::ExtendedProtocol::MakeExtAddEntity2Packet;
	(*m_lua)["MakeExtRemovePlayerNamePacket"] = &Net::ExtendedProtocol::MakeExtRemovePlayerNamePacket;
	(*m_lua)["MakeEnvSetColorPacket"] = &Net::ExtendedProtocol::MakeEnvSetColorPacket;
	(*m_lua)["MakeMakeSelectionPacket"] = &Net::ExtendedProtocol::MakeMakeSelectionPacket;
	(*m_lua)["MakeRemoveSelectionPacket"] = &Net::ExtendedProtocol::MakeRemoveSelectionPacket;
	(*m_lua)["MakeSetBlockPermissionPacket"] = &Net::ExtendedProtocol::MakeSetBlockPermissionPacket;
	(*m_lua)["MakeChangeModelPacket"] = &Net::ExtendedProtocol::MakeChangeModelPacket;
	(*m_lua)["MakeEnvSetWeatherTypePacket"] = &Net::ExtendedProtocol::MakeEnvSetWeatherTypePacket;
	(*m_lua)["MakeHackControlPacket"] = &Net::ExtendedProtocol::MakeHackControlPacket;
	(*m_lua)["MakeDefineBlockPacket"] = &Net::ExtendedProtocol::MakeDefineBlockPacket;
	(*m_lua)["MakeRemoveBlockDefinitionPacket"] = &Net::ExtendedProtocol::MakeRemoveBlockDefinitionPacket;
	(*m_lua)["MakeDefineBlockExtPacket"] = &Net::ExtendedProtocol::MakeDefineBlockExtPacket;
	(*m_lua)["MakeSetTextColorPacket"] = &Net::ExtendedProtocol::MakeSetTextColorPacket;
	(*m_lua)["MakeSetMapEnvURLPacket"] = &Net::ExtendedProtocol::MakeSetMapEnvURLPacket;
	(*m_lua)["MakeSetMapEnvPropertyPacket"] = &Net::ExtendedProtocol::MakeSetMapEnvPropertyPacket;
	(*m_lua)["MakeSetEntityPropertyPacket"] = &Net::ExtendedProtocol::MakeSetEntityPropertyPacket;
	(*m_lua)["MakeTwoWayPingPacket"] = &Net::ExtendedProtocol::MakeTwoWayPingPacket;
	(*m_lua)["MakeSetInventoryOrderPacket"] = &Net::ExtendedProtocol::MakeSetInventoryOrderPacket;
	(*m_lua)["MakeSetHotbarPacket"] = &Net::ExtendedProtocol::MakeSetHotbarPacket;
	(*m_lua)["MakeSetSpawnpointPacket"] = &Net::ExtendedProtocol::MakeSetSpawnpointPacket;
	(*m_lua)["MakeVelocityControlPacket"] = &Net::ExtendedProtocol::MakeVelocityControlPacket;
	(*m_lua)["MakeDefineEffectPacket"] = &Net::ExtendedProtocol::MakeDefineEffectPacket;
	(*m_lua)["MakeSpawnEffectPacket"] = &Net::ExtendedProtocol::MakeSpawnEffectPacket;
	/* END AUTOGENERATED CODE SECTION */
}

void PluginHandler::LoadPlugins()
{
	std::string pluginDir = "plugins";
	for (auto iter = std::filesystem::recursive_directory_iterator(pluginDir);
			iter != std::filesystem::recursive_directory_iterator();
			++iter) {
		const std::string filename = iter->path().string();
		std::string pluginName = iter->path().parent_path().filename().string();
		if (iter->path().filename().string() == "init.lua") {
			std::unique_ptr<IPlugin> plugin = std::make_unique<LuaPlugin>(m_lua, filename, pluginName);
			AddPlugin(std::move(plugin));
		}
	}

	// FIXME: Add priorities instead?
	auto iter = std::find_if(
		m_plugins.begin(), m_plugins.end(),
		[&](std::unique_ptr<IPlugin>& plugin)
		{
			return plugin->GetName() == "Core";
		}
	);

	if (iter != m_plugins.end())
		std::rotate(m_plugins.begin(), iter, iter + 1);

	for (auto& plugin : m_plugins) {
		std::string pluginName = plugin->GetName();
		try {
			plugin->Init();
			LOG(LOGLEVEL_DEBUG, "Loaded plugin: %s", pluginName.c_str());
		}
		catch (const std::runtime_error& e) {
			LOG(LOGLEVEL_WARNING, "PluginHandler error (%s): %s", pluginName.c_str(), e.what());
		}
	}
}

void PluginHandler::ReloadPlugins()
{
	LuaPlugins::authEvent.clear();
	LuaPlugins::messageEvent.clear();
	LuaPlugins::joinEvent.clear();
	LuaPlugins::positionOrientationEvent.clear();
	LuaPlugins::setBlockEvent.clear();
	LuaPlugins::disconnectEvent.clear();
	LuaPlugins::playerClickedEvent.clear();
	LuaPlugins::extEntryEvent.clear();

	m_plugins.clear();
	m_lua.reset();
	InitLua();
	LoadPlugins();

	// Re-trigger auth/join events for all players
	auto worlds = Server::GetInstance()->GetWorlds();
	for (auto& world : worlds) {
		auto players = world.second->GetPlayers();
		for (auto& player : players) {
			TriggerAuthEvent(player);
			TriggerJoinEvent(player, player->GetWorld());
		}
	}
}

void PluginHandler::AddPlugin(std::unique_ptr<IPlugin> plugin)
{
	m_plugins.push_back(std::move(plugin));
}

void PluginHandler::Update()
{
	if (m_reloadPlugins) {
		m_reloadPlugins = false;
		ReloadPlugins();
		return;
	}

	// TODO: update at regular intervals (tick rate)
	for (auto& plugin : m_plugins) {
		try {
			plugin->Tick();
		} catch(std::runtime_error& e) {
			std::cerr << e.what() << std::endl;
		}
	}
}

void PluginHandler::TriggerAuthEvent(Player::PlayerPtr player)
{
	try {
		for (auto& func : LuaPlugins::authEvent)
			func(player);
	}
	catch (std::runtime_error& e) {
		std::cerr << "TriggerAuthEvent exception: " << e.what() << std::endl;
	}
}

void PluginHandler::TriggerMessageEvent(Player::PlayerPtr player, std::string message, uint8_t flag)
{
	sol::table table = m_lua->create_table_with("message", message, "flag", flag);
	try {
		for (auto& func : LuaPlugins::messageEvent)
			func(player, table);
	}
	catch (std::runtime_error& e) {
		std::cerr << "TriggerMessageEvent exception: " << e.what() << std::endl;
	}
}

void PluginHandler::TriggerJoinEvent(Player::PlayerPtr player, World* world)
{
	try {
		for (auto& func : LuaPlugins::joinEvent)
			func(player, world);
	}
	catch (std::runtime_error& e) {
		std::cerr << "TriggerJoinEvent exception: " << e.what() << std::endl;
	}
}

void PluginHandler::TriggerPositionOrientationEvent(Player::PlayerPtr player, Utils::Vector position, uint8_t yaw, uint8_t pitch)
{
	sol::table table = m_lua->create_table_with("position", position, "yaw", yaw, "pitch", pitch);
	try {
		for (auto& func : LuaPlugins::positionOrientationEvent)
			func(player, table);
	}
	catch (std::runtime_error& e) {
		std::cerr << "TriggerPositionOrientationEvent exception: " << e.what() << std::endl;
	}
}

void PluginHandler::TriggerSetBlockEvent(Player::PlayerPtr player, int blockType, Utils::Vector position)
{
	try {
		for (auto& func : LuaPlugins::setBlockEvent)
			func(player, blockType, position);
	}
	catch (std::runtime_error& e) {
		std::cerr << "TriggerSetBlockEvent exception: " << e.what() << std::endl;
	}
}

void PluginHandler::TriggerDisconnectEvent(Player::PlayerPtr player)
{
	try {
		for (auto& func : LuaPlugins::disconnectEvent)
			func(player);
	}
	catch (std::runtime_error& e) {
		std::cerr << "TriggerDisconnectEvent exception: " << e.what() << std::endl;
	}
}

void PluginHandler::TriggerPlayerClickedEvent(
	Player::PlayerPtr player,
	uint8_t button,
	uint8_t action,
	uint16_t yaw,
	uint16_t pitch,
	int8_t targetEntityID,
	int16_t targetBlockX,
	int16_t targetBlockY,
	int16_t targetBlockZ,
	uint8_t targetBlockFace
)
{
	sol::table table = m_lua->create_table_with(
		"button", button,
		"action", action,
		"yaw", yaw,
		"pitch", pitch,
		"targetEntityID", targetEntityID,
		"targetBlockX", targetBlockX,
		"targetBlockY", targetBlockY,
		"targetBlockZ", targetBlockZ,
		"targetBlockFace", targetBlockFace
	);

	try {
		for (auto& func : LuaPlugins::playerClickedEvent)
			func(player, table);
	}
	catch (std::runtime_error& e) {
		std::cerr << "TriggerPlayerClickedEvent exception: " << e.what() << std::endl;
	}
}

void PluginHandler::TriggerExtEntryEvent(Player::PlayerPtr player, std::string name, uint32_t version)
{
	sol::table table = m_lua->create_table_with(
		"name", name,
		"version", version
	);

	try {
		for (auto& func : LuaPlugins::extEntryEvent)
			func(player, table);
	}
	catch (std::runtime_error& e) {
		std::cerr << "TriggerExtEntryEvent exception: " << e.what() << std::endl;
	}
}
