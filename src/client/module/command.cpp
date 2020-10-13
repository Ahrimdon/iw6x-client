#include <std_include.hpp>
#include "command.hpp"

#include "game/game.hpp"
#include "utils/hook.hpp"
#include "utils/string.hpp"
#include "utils/memory.hpp"

std::unordered_map<std::string, std::function<void(command::params&)>> command::handlers;
std::unordered_map<std::string, std::function<void(int, command::params_sv&)>> command::handlers_sv;

int command::params::size()
{
	return game::Cmd_Argc();
}

const char* command::params::get(int index)
{
	return game::Cmd_Argv(index);
}

std::string command::params::join(int index)
{
	std::string result = {};

	for (int i = index; i < this->size(); i++)
	{
		if (i > index) result.append(" ");
		result.append(this->get(i));
	}
	return result;
}

int command::params_sv::size()
{
	return game::SV_Cmd_Argc();
}

const char* command::params_sv::get(int index)
{
	return game::SV_Cmd_Argv(index);
}

std::string command::params_sv::join(int index)
{
	std::string result = {};

	for (int i = index; i < this->size(); i++)
	{
		if (i > index) result.append(" ");
		result.append(this->get(i));
	}
	return result;
}

void command::add_raw(const char* name, void (*callback)())
{
	game::Cmd_AddCommandInternal(name, callback, utils::memory::get_allocator()->allocate<game::cmd_function_s>());
}

void command::add(const char* name, const std::function<void(params&)>& callback)
{
	const auto command = utils::string::to_lower(name);

	if (handlers.find(command) == handlers.end())
		add_raw(name, main_handler);

	handlers[command] = callback;
}

void command::add(const char* name, const std::function<void()>& callback)
{
	command::add(name, [callback](params&)
	{
		callback();
	});
}

void command::add_sv(const char* name, std::function<void(int, params_sv&)> callback)
{
	// doing this so the sv command would show up in the console
	command::add_raw(name, nullptr);

	const auto command = utils::string::to_lower(name);

	if (handlers_sv.find(command) == handlers_sv.end())
		handlers_sv[command] = callback;
}

void command::main_handler()
{
	params params = {};

	const auto command = utils::string::to_lower(params[0]);
	if (handlers.find(command) != handlers.end())
	{
		handlers[command](params);
	}
}

utils::hook::detour client_command_hook;

void command::client_command(int clientNum, void* a2)
{
	params_sv params = {};

	const auto command = utils::string::to_lower(params[0]);
	if (handlers_sv.find(command) != handlers_sv.end())
	{
		handlers_sv[command](clientNum, params);
	}

	client_command_hook.invoke<void>(clientNum, a2);
}

void command::add_sp_commands()
{
	command::add("noclip", [&]()
	{
		if (!game::SV_Loaded())
		{
			return;
		}

		game::sp::g_entities[0].client->flags ^= 1;
		game::CG_GameMessage(0, utils::string::va("noclip %s",
		                                          game::sp::g_entities[0].client->flags & 1 ? "^2on" : "^1off"));
	});

	command::add("ufo", [&]()
	{
		if (!game::SV_Loaded())
		{
			return;
		}

		game::sp::g_entities[0].client->flags ^= 2;
		game::CG_GameMessage(
			0, utils::string::va("ufo %s", game::sp::g_entities[0].client->flags & 2 ? "^2on" : "^1off"));
	});
}

void command::add_mp_commands()
{
	client_command_hook.create(0x1403929B0, &client_command);

	command::add_sv("noclip", [&](int clientNum, command::params_sv&)
	{
		if (!game::SV_Loaded())
		{
			return;
		}

		game::mp::g_entities[clientNum].client->flags ^= 1;
		game::SV_GameSendServerCommand(clientNum, 1,
		                               utils::string::va("f \"noclip %s\"",
		                                                 game::mp::g_entities[clientNum].client->flags & 1
			                                                 ? "^2on"
			                                                 : "^1off"));
	});

	command::add_sv("ufo", [&](int clientNum, command::params_sv&)
	{
		if (!game::SV_Loaded())
		{
			return;
		}

		game::mp::g_entities[clientNum].client->flags ^= 2;
		game::SV_GameSendServerCommand(clientNum, 1,
		                               utils::string::va("f \"ufo %s\"",
		                                                 game::mp::g_entities[clientNum].client->flags & 2
			                                                 ? "^2on"
			                                                 : "^1off"));
	});

	command::add_sv("setviewpos", [&](int clientNum, command::params_sv& params)
	{
		if (!game::SV_Loaded())
		{
			return;
		}

		game::mp::g_entities[clientNum].client->ps.origin[0] = std::strtof(params.get(1), NULL);
		game::mp::g_entities[clientNum].client->ps.origin[1] = std::strtof(params.get(2), NULL);
		game::mp::g_entities[clientNum].client->ps.origin[2] = std::strtof(params.get(3), NULL);
	});

	command::add_sv("setviewang", [&](int clientNum, command::params_sv& params)
	{
		if (!game::SV_Loaded())
		{
			return;
		}

		game::mp::g_entities[clientNum].client->ps.delta_angles[0] = std::strtof(params.get(1), NULL);
		game::mp::g_entities[clientNum].client->ps.delta_angles[1] = std::strtof(params.get(2), NULL);
		game::mp::g_entities[clientNum].client->ps.delta_angles[2] = std::strtof(params.get(3), NULL);
	});
}

void command::post_unpack()
{
	if (game::environment::is_mp())
	{
		add_mp_commands();
	}
	else if (game::environment::is_sp())
	{
		add_sp_commands();
	}
}

REGISTER_MODULE(command);
