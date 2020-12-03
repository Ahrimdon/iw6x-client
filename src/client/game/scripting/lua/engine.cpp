#include <std_include.hpp>
#include "engine.hpp"
#include "context.hpp"

#include <utils/io.hpp>

namespace scripting::lua::engine
{
	namespace
	{
		std::vector<context>& get_scripts()
		{
			static std::vector<context> scripts{};
			return scripts;
		}

		void load_scripts()
		{
			const auto script_dir = "iw6x/scripts/"s;

			if (!utils::io::directory_exists(script_dir))
			{
				return;
			}

			const auto scripts = utils::io::list_files(script_dir);

			for (const auto& script : scripts)
			{
				if (script.size() > 4 && script.substr(script.find_last_of('.') + 1) == "lua")
				{
					get_scripts().emplace_back(script);
				}
			}
		}
	}

	void start()
	{
		// No SP until there is a concept
		if (game::environment::is_sp())
		{
			return;
		}

		try
		{
			load_scripts();
		}
		catch(std::exception&)
		{

		}
	}

	void stop()
	{
		get_scripts().clear();
	}

	void notify(const event& e)
	{
		for (auto& script : get_scripts())
		{
			script.notify(e);
		}
	}

	void run_frame()
	{
		for (auto& script : get_scripts())
		{
			script.run_frame();
		}
	}
}
