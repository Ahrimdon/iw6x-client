#include "std_include.hpp"
#include "context.hpp"
#include "error.hpp"

namespace scripting::lua
{
	scheduler::scheduler(sol::state& state)
	{
		auto task_handle_type = state.new_usertype<task_handle>("task_handle");

		task_handle_type["clear"] = [this](const task_handle& handle)
		{
			this->remove(handle);
		};

		state["ontimeout"] = [this](const std::function<void()>& callback, const long long milliseconds)
		{
			return this->add(callback, milliseconds, true);
		};

		state["oninterval"] = [this](const std::function<void()>& callback, const long long milliseconds)
		{
			return this->add(callback, milliseconds, false);
		};
	}

	void scheduler::run_frame()
	{
		for (auto task : this->tasks_)
		{
			const auto now = std::chrono::steady_clock::now();
			if ((now - task->last_execution) > task->delay)
			{
				task->last_execution = now;
				if (task->is_volatile)
				{
					this->tasks_.remove(task);
				}

				try
				{
					task->callback();
				}
				catch (std::exception& e)
				{
					handle_error(e);
				}
			}
		}
	}

	void scheduler::clear()
	{
		this->tasks_.clear();
	}

	task_handle scheduler::add(const std::function<void()>& callback, const long long milliseconds,
	                           const bool is_volatile)
	{
		return this->add(callback, std::chrono::milliseconds(milliseconds), is_volatile);
	}

	task_handle scheduler::add(const std::function<void()>& callback, const std::chrono::milliseconds delay,
	                           const bool is_volatile)
	{
		task task;
		task.is_volatile = is_volatile;
		task.callback = callback;
		task.delay = delay;
		task.last_execution = std::chrono::steady_clock::now();
		task.id = ++this->current_task_id_;

		this->tasks_.add(task);

		return {task.id};
	}

	void scheduler::remove(const task_handle& handle)
	{
		for (auto task : this->tasks_)
		{
			if (task->id == handle.id)
			{
				this->tasks_.remove(task);
				break;
			}
		}
	}
}
