#ifndef TYPES_HPP
#define TYPES_HPP

#include <chrono>
#include <cstdint>
#include <string>
#include <ostream>
#include <ats/date_time/date_time.hpp>

namespace ats
{
	using price_t = int;
	using timestamp_t = ats::date_time::date_time;//uint64_t;
	using orderid_t = uint64_t;
//	using quantity_t = uint64_t;

	struct symbol_key
	{
		std::string name;
		size_t index;

		symbol_key(const std::string& symbol, size_t index)
			: name(symbol), index(index) { }

		const std::string& to_string() const { return name; }

		friend std::ostream& operator<<(std::ostream& os, const ats::symbol_key& key)
		{
			os << key.name;
			return os;
		}
	};

	enum class subscription
	{
		Level2,
		Level1,
		TimeAndSales,
		Bar,
		Daily,
		Custom
	};
}

#endif