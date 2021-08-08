#pragma once

#include <string>
#include <vector>

namespace lib
{
	namespace db
	{
		class image
		{
		public:
			image() = default;

			std::string url;

			std::vector<char> data;
		};
	}
}
