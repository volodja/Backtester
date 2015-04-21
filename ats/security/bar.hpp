#ifndef BAR_HPP
#define BAR_HPP

namespace ats
{
	struct bar
	{
		ats::price_t open;
		ats::price_t high;
		ats::price_t low;
		ats::price_t close;
		long quantity;
		ats::timestamp_t time_open;
	};
}

#endif