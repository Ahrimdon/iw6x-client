#include <std_include.hpp>
#include "loader/module_loader.hpp"
#include "loader/loader.hpp"

class icon final : public module
{
public:
	void* load_import([[maybe_unused]] const std::string& module, const std::string& function) override
	{
		if (function == "LoadIconW")
		{
			return &load_icon_w;
		}

		return nullptr;
	}

private:
	static HICON __stdcall load_icon_w(const HINSTANCE instance, const wchar_t* name)
	{
		if (size_t(name) <= 300 && instance == loader::get_game_module())
		{
			return LoadIconA(loader::get_main_module(), MAKEINTRESOURCE(102));
		}

		return LoadIconW(instance, name);
	}
};

REGISTER_MODULE(icon)
