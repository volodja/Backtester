#ifndef BRACKET_ORDER_HPP
#define BRACKET_ORDER_HPP

#include "limit_order.hpp"
#include "stop_order.hpp"

namespace ats
{
	class bracket_order : public order
	{
	public:
		bracket_order(const ats::orderid_t& id, ats::limit_order& lmt_order, ats::stop_order& stp_order)
			: order(id, lmt_order.symbol(), lmt_order.quantity(), lmt_order.side(), lmt_order.time_in_force())
		{
			lmt_order.parent_id = id;
			stp_order.parent_id = id;
		}

		const ats::orderid_t& limit_order_id() const { return limit_order_id_; }
		const ats::orderid_t& stop_order_id() const { return stop_order_id_; }

	private:
		ats::orderid_t limit_order_id_;
		ats::orderid_t stop_order_id_;
	};
}

#endif