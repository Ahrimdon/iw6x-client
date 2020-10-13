#pragma once
#include "loader/module_loader.hpp"

class filesystem final : public module
{
public:
	class file
	{
	public:
		file(std::string name);

		bool exists() const;
		const std::string& get_buffer() const;
		const std::string& get_name() const;

	private:
		bool valid_ = false;
		std::string name_;
		std::string buffer_;
	};

	void post_unpack() override;
};
