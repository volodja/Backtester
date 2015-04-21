#ifndef METAINFO_HPP
#define METAINFO_HPP

#include <ats/types.hpp>

namespace ats
{
	struct metainfo
	{
		ats::price_t tick_size = 10;
		double tick_value = 10.0;

		metainfo(ats::price_t ticksize = 10, double tickvalue = 10.0)
			: tick_size(ticksize), tick_value(tickvalue) { }
	};
}

#endif