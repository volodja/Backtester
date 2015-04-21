#ifndef TOKENIZE_HPP
#define TOKENIZE_HPP

#include <string>
#include <array>

namespace ats
{
	// Tokenizes a string and returns the number of fields
	template<size_t num_cols>
	static size_t tokenize(const std::string& line, std::array<std::string, num_cols>& result, char sep = ',')
	{
		size_t i = 0U;
		std::string::const_iterator begin = line.cbegin();
		for (auto it = line.cbegin(); it != line.cend() && i < num_cols; ++it)
		{
			if (*it == sep)
			{
				result[i++].assign(begin, it);
				begin = it + 1;
			}
		}

		if (i == num_cols)
			return num_cols + 1;

		if (i < num_cols)
			result[i++].assign(begin, line.cend());

		return i;
	}
}

#endif