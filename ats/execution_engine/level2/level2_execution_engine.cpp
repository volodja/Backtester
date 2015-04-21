#include "level2_execution_engine.hpp"

namespace ats
{
	void level2_execution_engine::send_order(const ats::limit_order& order)
	{
		auto it = sim_books_.find(order.symbol());
		if (it != sim_books_.cend())
		{
			add_order(order);
			it->second.add_order(order);
		}
	}

	void level2_execution_engine::send_order(const ats::market_order& order)
	{
		auto it = sim_books_.find(order.symbol());
		const ats::exchange_order_book& book = it->second.get_order_book();

		on_order_status_changed(ats::order_status_pending_new_message(order.id(), current_time()));

		if (order.fill_price != 0)
		{
			on_order_status_changed(ats::order_status_filled_message(order.id(), current_time(), order.fill_price, order.quantity()));
			return;
		}

		if (order.side() == ats::order_side::Buy || order.side() == ats::order_side::BuyCover)
		{
			if (!book.asks().empty())
			{
				ats::price_t price = book.best_ask()->price;
				on_order_status_changed(ats::order_status_filled_message(order.id(), current_time(), price, order.quantity()));
			}
			else
				on_order_status_changed(ats::order_status_rejected_message(order.id(), current_time(),
						"level2_execution_engine: No asks in order book"));
		}
		else
		{
			if (!book.bids().empty())
			{
				ats::price_t price = book.best_bid()->price;
				on_order_status_changed(ats::order_status_filled_message(order.id(), current_time(), price, order.quantity()));
			}
			else
				on_order_status_changed(ats::order_status_rejected_message(order.id(), current_time(),
						"level2_execution_engine: No bids in order book"));
		}
	}

	void level2_execution_engine::send_order(const ats::stop_order& order)
	{
		auto it = sim_books_.find(order.symbol());
		const ats::exchange_order_book& book = it->second.get_order_book();

		on_order_status_changed(ats::order_status_pending_new_message(order.id(), current_time()));

		if (order.side() == ats::order_side::Buy || order.side() == ats::order_side::BuyCover)
		{
			if (!book.asks().empty() && order.price() <= book.best_ask()->price)
				on_order_status_changed(ats::order_status_filled_message(order.id(), current_time(), book.best_ask()->price, order.quantity()));
			else
			{
				stops_buy_.insert(std::make_pair(order.price(), order));
				add_order(order);
//				orders_.insert(std::make_pair(order.id(), std::make_shared<ats::stop_order>(order)));
			}
		}
		else
		{
			if (!book.bids().empty() && order.price() >= book.best_bid()->price)
				on_order_status_changed(ats::order_status_filled_message(order.id(), current_time(), book.best_bid()->price, order.quantity()));
			else
			{
				stops_sell_.insert(std::make_pair(order.price(), order));
				add_order(order);
//				orders_.insert(std::make_pair(order.id(), std::make_shared<ats::stop_order>(order)));
			}
		}
	}
}
