#ifndef ORDER_TYPE_HPP
#define ORDER_TYPE_HPP

namespace ats
{
	enum class order_type
	{
		Market = 1,
		Limit,
		Stop,
		StopLimit
	};
}

#endif