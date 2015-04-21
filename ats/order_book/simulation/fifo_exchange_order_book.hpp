#ifndef FIFO_EXCHANGE_ORDERBOOK_HPP
#define FIFO_EXCHANGE_ORDERBOOK_HPP

#include <string>
#include "sim_book.hpp"
#include <ats/order_book/exchange_order_book.hpp>

namespace ats {
namespace sim
{
	class fifo_exchange_order_book
	{
	public:
		typedef std::unordered_map<ats::orderid_t, price_level::iterator> order_container;

		fifo_exchange_order_book(const ats::symbol_key& symbol, const std::string& exchange, size_t book_depth)
			: book_(symbol, exchange, book_depth) { }

		void add_order(const ats::limit_order& order);
		void cancel_order(const ats::orderid_t& id, const ats::timestamp_t& time)
		{ sim_book_.cancel_order(id, time); }

		void update(const ats::level2_message& msg);

		void add_order_status_listener(const ats::order_status_handler& listener)
		{
			order_status_listener_ = listener;
			sim_book_.add_order_status_listener(listener);
		}

		const ats::sim::price_level* get_level(ats::price_t price, bool is_bid) const
		{ return sim_book_.get_level(price, is_bid); }

		const ats::exchange_order_book& get_order_book() const { return book_; }

		const order_container& get_sim_orders() const { return sim_book_.get_sim_orders(); }

//	private:
		void process_change_msg(const ats::level2_message& msg);
		void process_delete_msg(const ats::level2_message& msg);
		void process_trade_msg(const ats::level2_message& msg) { sim_book_.process_trade_msg(msg); }
		void process_insert_msg(const ats::level2_message& msg) { sim_book_.process_insert_msg(msg); }
	private:
		ats::exchange_order_book book_;
		ats::sim::sim_book sim_book_;
		ats::order_status_handler order_status_listener_;
	};


	inline void fifo_exchange_order_book::add_order(const ats::limit_order& order)
	{
		if (order.side() == ats::order_side::Buy || order.side() == ats::order_side::BuyCover)
		{
			if (book_.best_ask() != nullptr && order.price() >= book_.best_ask()->price)
			{
				ats::order_status_filled_message msg(order.id(), order.transact_time,
						book_.best_ask()->price, order.quantity());
				order_status_listener_(msg);
			}
			else
			{
				const auto* true_l = book_.bid_at(order.price());
				const ats::sim::price_level* l = sim_book_.get_level(order.price(), true);
				if (true_l == nullptr && !book_.bids().empty() && order.price() <= book_.best_bid()->price
						&& order.price() >= book_.bids().crbegin()->first)
					sim_book_.insert_order(order);
				else
				{
					if (true_l != nullptr && (l == nullptr || !l->is_defined()))
					{
						ats::limit_order ord(0, order.symbol(), true_l->quantity, order.side(),
								ats::order_time_in_force::GTC, order.price());
						sim_book_.insert_order(ord);
					}
					sim_book_.add_order(order);
				}
			}
		}
		else
		{
			if (book_.best_bid() != nullptr && order.price() <= book_.best_bid()->price)
			{
				ats::order_status_filled_message msg(order.id(), order.transact_time,
						book_.best_bid()->price, order.quantity());
				order_status_listener_(msg);
			}
			else
			{
				const auto* true_l = book_.ask_at(order.price());
				const ats::sim::price_level* l = sim_book_.get_level(order.price(), false);
				if (true_l == nullptr && !book_.asks().empty() && order.price() >= book_.best_ask()->price
						&& order.price() <= book_.asks().crbegin()->first)
					sim_book_.insert_order(order);
				else
				{
					if (true_l != nullptr && (l == nullptr || !l->is_defined()))
					{
						ats::limit_order ord(0, order.symbol(), true_l->quantity, order.side(),
								ats::order_time_in_force::GTC, order.price());
						sim_book_.insert_order(ord);
					}
					sim_book_.add_order(order);
				}
			}
		}
	}

	inline void fifo_exchange_order_book::process_change_msg(const ats::level2_message& msg)
	{
		sim_book_.process_change_msg(msg);
	}

	inline void fifo_exchange_order_book::process_delete_msg(const ats::level2_message& msg)
	{
		sim_book_.process_delete_msg(msg);
	}

	inline void fifo_exchange_order_book::update(const ats::level2_message& msg)
	{
		// Create a "delta" message
		ats::level2_message msg_delta = msg;
		if (msg.update_action == ats::update_action::Change)
		{
			const ats::exchange_order_book::price_level_type* lptr = msg.entry_type == ats::entry_type::Bid ?
					book_.bid_at(msg.price) : book_.ask_at(msg.price);

			if (lptr != nullptr)
				msg_delta.quantity = msg.quantity - lptr->quantity;
		}
		// this is not used (at least for now):
		else if (msg.entry_type == ats::entry_type::Trade)
		{
			if (book_.best_ask() != nullptr && msg.price >= book_.best_ask()->price)
				msg_delta.aggressor_side = 1;
			else if (book_.best_bid() != nullptr && msg.price <= book_.best_bid()->price)
				msg_delta.aggressor_side = -1;
			else
				msg_delta.aggressor_side = 0;
		}

		book_.update(msg);

		const ats::price_t* bid = book_.best_bid() == nullptr ? nullptr : &book_.best_bid()->price;
		const ats::price_t* ask = book_.best_ask() == nullptr ? nullptr : &book_.best_ask()->price;
		sim_book_.execute_crosses(bid, ask, msg.time);

		sim_book_.process_level2_msg(msg_delta);
	}
}
}

#endif