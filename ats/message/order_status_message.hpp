#ifndef ORDER_STATUS_MESSAGE_HPP
#define ORDER_STATUS_MESSAGE_HPP

#include <string>
#include <memory>
#include <ats/types.hpp>
#include <ats/order/order_defs.hpp>
#include <ats/message/message.hpp>
#include <ats/message/order_status_message.hpp>

namespace ats
{
	struct order_status_message : public ats::message
	{
		order_status_message(const ats::orderid_t& id, ats::order_status status, const ats::timestamp_t& time)
			: ats::message(time), order_id(id), order_status(status) { }

		ats::orderid_t order_id;         // id of the changed order
		ats::order_status order_status;  // current order status
		long replaces_order_id = -1;
		long replaced_by_order_id = -1;
		long parent_id = -1;
	};

	struct order_status_filled_message : public ats::order_status_message
	{
		order_status_filled_message(const ats::orderid_t& id, const ats::timestamp_t& time,
				ats::price_t price, long quantity)
			: ats::order_status_message(id, ats::order_status::Filled, time), price(price), quantity(quantity) { }

		ats::price_t price;
		long quantity;
		// execution execution;
	};

	struct order_status_partially_filled_message : public ats::order_status_message
	{
		order_status_partially_filled_message(const ats::orderid_t& id, const ats::timestamp_t& time,
			ats::price_t price, long quantity)
			: ats::order_status_message(id, ats::order_status::PartiallyFilled, time), price(price), quantity(quantity) { }

		ats::price_t price;
		long quantity;
		// execution execution;
	};

	struct order_status_cancelled_message : public ats::order_status_message
	{
		order_status_cancelled_message(const ats::orderid_t& id, const ats::timestamp_t& time, const std::string& cancel_reason = "")
			: ats::order_status_message(id, ats::order_status::Canceled, time), cancel_reason(cancel_reason) { }

		std::string cancel_reason;
	};

	struct order_status_rejected_message : public ats::order_status_message
	{
		order_status_rejected_message(const ats::orderid_t& id, const ats::timestamp_t& time, const std::string& rejection_reason = "")
			: ats::order_status_message(id, ats::order_status::Rejected, time), rejection_reason(rejection_reason) { }

		std::string rejection_reason;
	};

	struct order_status_pending_new_message : public ats::order_status_message
	{
		order_status_pending_new_message(const ats::orderid_t& id, const ats::timestamp_t& time)
		: ats::order_status_message(id, ats::order_status::PendingNew, time) { }
	};

	struct order_status_new_message : public ats::order_status_message
	{
		order_status_new_message(const ats::orderid_t& id, const ats::timestamp_t& time, const ats::timestamp_t& time_accepted)
			: ats::order_status_message(id, ats::order_status::New, time), time_accepted(time_accepted) { }

		ats::timestamp_t time_accepted;
	};
}

#endif