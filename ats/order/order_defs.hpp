#ifndef ORDER_DEFS_HPP
#define ORDER_DEFS_HPP

namespace ats
{
	enum class order_side
	{
		Buy,
		Sell,
		BuyCover,
		SellShort
	};

	enum class order_time_in_force
	{
		Day,
		GTC,
		IOC,
		AtTheOpening,
		AtTheClose,
		FUNARI,
		FOK,
		GTX,
		Date
	};

	enum class order_status
	{
		New = 0,             // outstanding order with no executions
		PartiallyFilled,     // partially filled and is still working; sent on every order execution
		Filled,              // completely filled (final order status)
		DoneForDay,
		Canceled,            // cancelled (final order status)
		Replaced,
		PendingCancel,
		Rejected,            // rejected for some reason (final order status)
		Suspended,
		PendingNew,          // received by sell-side's system but not yet accepted for execution
		Calculated,
		Expired,
		AcceptedForBidding,
		PendingReplace
	};
}

#endif