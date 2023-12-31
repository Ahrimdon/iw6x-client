#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <iostream>

struct object {
	int value = 0;
};

int main(int, char*[]) {
	std::cout << "=== runtime_additions ===" << std::endl;

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<object>("object");

	// runtime additions: through the sol API
	lua["object"]["func"] = [](object& o) {
		++o.value;
		return o.value;
	};
	// runtime additions: through a lua script
	lua.script(R"(
function object:print () 
	print(self:func())
end
	)");

	// see it work
	lua.script(R"(
obj = object.new()
obj:print()
	)");

	object& obj = lua["obj"];
	SOL_ASSERT(obj.value == 1);

	return 0;
}
