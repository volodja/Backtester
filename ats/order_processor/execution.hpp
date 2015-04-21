#ifndef EXECUTION_HPP
#define EXECUTION_HPP

#include <string>
#include <ats/types.hpp>

namespace ats
{
	struct execution
	{
		std::string symbol;
		timestamp_t time;         // time of execution
		price_t price;            // execution price
		long quantity;            // execution quantity
		double commission = 0.0;
	};
}

#endif