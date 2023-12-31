#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <iostream>

int main(int, char**) {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	// Environments can set on functions (scripts), userdata and
	// threads let's look at functions

	lua.script("f = function() return test end");
	sol::function f = lua["f"];

	sol::environment env_f(lua, sol::create);
	env_f["test"] = 31;
	bool env_f_set_success = sol::set_environment(env_f, f);
	SOL_ASSERT(env_f_set_success);

	// the function returns the value from the environment table
	int result = f();
	SOL_ASSERT(result == 31);


	// You can also protect from variables
	// being set without the 'local' specifier
	lua.script("g = function() test = 5 end");
	sol::function g = lua["g"];
	sol::environment env_g(lua, sol::create);
	bool env_g_set_success
	     = env_g.set_on(g); // same as set_environment
	SOL_ASSERT(env_g_set_success);

	g();
	// the value can be retrieved from the env table
	int test = env_g["test"];
	SOL_ASSERT(test == 5);


	// the global environment
	// is not polluted at all, despite both functions being used
	// and set
	sol::object global_test = lua["test"];
	SOL_ASSERT(!global_test.valid());


	// You can retrieve environments in C++
	// and check the environment of functions
	// gotten from Lua

	// get the environment from any sol::reference-styled type,
	// including sol::object, sol::function, sol::table,
	// sol::userdata ...
	lua.set_function("check_f_env",
	     // capture necessary variable in C++ lambda
	     [&env_f](sol::object target) {
		     // pull out the environment from func using
		     // sol::env_key constructor
		     sol::environment target_env(sol::env_key, target);
		     int test_env_f = env_f["test"];
		     int test_target_env = target_env["test"];
		     // the environment for f the one gotten from
		     // `target` are the same
		     SOL_ASSERT(test_env_f == test_target_env);
		     SOL_ASSERT(test_env_f == 31);
		     SOL_ASSERT(env_f == target_env);
	     });
	lua.set_function(
	     "check_g_env", [&env_g](sol::function target) {
		     // equivalent:
		     sol::environment target_env
		          = sol::get_environment(target);
		     int test_env_g = env_g["test"];
		     int test_target_env = target_env["test"];
		     SOL_ASSERT(test_env_g == test_target_env);
		     SOL_ASSERT(test_env_g == 5);
		     SOL_ASSERT(env_g == target_env);
	     });

	lua.script("check_f_env(f)");
	lua.script("check_g_env(g)");

	return 0;
}
