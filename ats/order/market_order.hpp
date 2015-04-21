#ifndef MARKET_ORDER_HPP
#define MARKET_ORDER_HPP

#include <memory>
#include "order.hpp"
#include <ats/types.hpp>

namespace ats
{
	class market_order : public ats::order
	{
	public:
		market_order(const ats::orderid_t& id, const std::string& symbol,
				long quantity, ats::order_side side, ats::order_time_in_force time_in_force)
			: ats::order(id, symbol, quantity, side, time_in_force), fill_price(0) { }

		ats::price_t fill_price;
	};
}

#endif