#ifndef STOP_ORDER_HPP
#define STOP_ORDER_HPP

#include "order.hpp"
#include <ats/types.hpp>

namespace ats
{
	class stop_order : public ats::order
	{
	public:
		stop_order(const ats::orderid_t& id, const std::string& symbol,
				long quantity, ats::order_side side, ats::order_time_in_force time_in_force,
				ats::price_t stop_price)
			: ats::order(id, symbol, quantity, side, time_in_force), price_(stop_price) { }

		ats::price_t price() const { return price_; }
	private:
		ats::price_t price_;
	};
}

#endif