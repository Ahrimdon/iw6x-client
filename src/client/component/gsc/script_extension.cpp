#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "game/scripting/execution.hpp"
#include "game/scripting/function.hpp"
#include "game/scripting/lua/error.hpp"

#include <utils/hook.hpp>

#include "component/console.hpp"
#include "component/scripting.hpp"
#include "component/notifies.hpp"
#include "component/game_log.hpp"

#include <xsk/gsc/types.hpp>
#include <xsk/resolver.hpp>

namespace gsc
{
	std::uint16_t function_id_start = 0x25D;
	std::uint16_t method_id_start = 0x8429;

	void* func_table[0x1000];

	namespace
	{
		#define RVA(ptr) static_cast<std::uint32_t>(reinterpret_cast<std::size_t>(ptr) - 0x140000000)

		std::unordered_map<std::uint16_t, game::BuiltinFunction> functions;

		unsigned int scr_get_function_stub(const char** p_name, int* type)
		{
			const auto result = utils::hook::invoke<unsigned int>(0x1403CD9F0, p_name, type);

			for (const auto& [name, func] : functions)
			{
				game::Scr_RegisterFunction(func, 0, name);
			}

			return result;
		}

		void add_function(const std::string& name, game::BuiltinFunction function)
		{
			++function_id_start;
			functions[function_id_start] = function;
			xsk::gsc::iw6::resolver::add_function(name, function_id_start);
		}

		scripting::script_value get_argument(int index)
		{
			if (static_cast<std::uint32_t>(index) >= game::scr_VmPub->outparamcount)
			{
				return {};
			}

			return {game::scr_VmPub->top[-index]};
		}

		void inc_in_param()
		{
			game::Scr_ClearOutParams();

			if (game::scr_VmPub->top == game::scr_VmPub->maxstack)
			{
				game::Sys_Error("Internal script stack overflow");
			}

			++game::scr_VmPub->top;
			++game::scr_VmPub->inparamcount;
		}

		void add_code_pos(const char* pos)
		{
			inc_in_param();
			game::scr_VmPub->top->type = game::SCRIPT_FUNCTION;
			game::scr_VmPub->top->u.codePosValue = pos;
		}

		void scr_print()
		{
			for (auto i = 0u; i < game::Scr_GetNumParam(); ++i)
			{
				console::info("%s", game::Scr_GetString(i));
			}
		}

		void scr_print_ln()
		{
			for (auto i = 0u; i < game::Scr_GetNumParam(); ++i)
			{
				console::info("%s", game::Scr_GetString(i));
			}

			console::info("\n");
		}

		void gscr_log_print()
		{
			char buf[1024]{};
			std::size_t out_chars = 0;

			for (auto i = 0u; i < game::Scr_GetNumParam(); ++i)
			{
				const auto* value = game::Scr_GetString(i);
				const auto len = std::strlen(value);

				out_chars += len;
				if (out_chars >= sizeof(buf))
				{
					break;
				}

				strncat_s(buf, value, _TRUNCATE);
			}

			game_log::g_log_printf("%s", buf);
		}
	}

	class extension final : public component_interface
	{
	public:
		void post_unpack() override
		{
			utils::hook::set<void(*)()>(SELECT_VALUE(0x14086F468, 0x1409E6CE8), scr_print);
			utils::hook::set<void(*)()>(SELECT_VALUE(0x14086F480, 0x1409E6D00), scr_print_ln);

			utils::hook::set<std::uint32_t>(SELECT_VALUE(0x1403D353C, 0x14042E33C), 0x1000); // Scr_RegisterFunction

			utils::hook::set<std::uint32_t>(SELECT_VALUE(0x1403D3542 + 4, 0x14042E342 + 4), RVA(&func_table)); // Scr_RegisterFunction
			utils::hook::set<std::uint32_t>(SELECT_VALUE(0x1403E0BDD + 3, 0x14043BBBE + 3), RVA(&func_table)); // VM_Execute_0
			utils::hook::inject(SELECT_VALUE(0x1403D38E4 + 3, 0x14042E734 + 3), &func_table); // Scr_BeginLoadScripts

			if (game::environment::is_sp())
			{
				return;
			}

			utils::hook::set<void(*)()>(0x1409E8A20, gscr_log_print);

			utils::hook::call(0x14042E76F, scr_get_function_stub);

			add_function("getfunction", []
			{
				const auto* filename = game::Scr_GetString(0);
				const auto* function = game::Scr_GetString(1);

				if (scripting::script_function_table[filename].contains(function))
				{
					const auto func = scripting::function{scripting::script_function_table[filename][function]};
					add_code_pos(func.get_pos());
				}

				game::Scr_ErrorInternal();
			});

			add_function("replacefunc", []
			{
				const auto what = get_argument(0).get_raw();
				const auto with = get_argument(1).get_raw();

				if (what.type != game::SCRIPT_FUNCTION || with.type != game::SCRIPT_FUNCTION)
				{
					game::Scr_ErrorInternal();
				}

				notifies::set_gsc_hook(what.u.codePosValue, with.u.codePosValue);
			});
		}
	};
}

REGISTER_COMPONENT(gsc::extension)
