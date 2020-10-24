#pragma once

namespace scheduler
{
	enum class pipeline
	{
		// Asynchronuous pipeline, disconnected from the game
		async,

		// The game's rendering pipeline
		renderer,

		// The game's server thread
		server,

		// The game's main thread
		main,
	};

	static const bool cond_continue = false;
	static const bool cond_end = true;

	void schedule(const std::function<bool()>& callback, pipeline type = pipeline::async,
	              std::chrono::milliseconds delay = 0ms);
	void loop(const std::function<void()>& callback, pipeline type = pipeline::async,
	          std::chrono::milliseconds delay = 0ms);
	void once(const std::function<void()>& callback, pipeline type = pipeline::async,
	          std::chrono::milliseconds delay = 0ms);
	void on_dw_init(const std::function<void()>& callback, pipeline type = pipeline::async,
	          std::chrono::milliseconds delay = 0ms);
}
