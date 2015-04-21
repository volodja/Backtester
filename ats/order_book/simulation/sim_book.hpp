#ifndef SIM_BOOK_HPP
#define SIM_BOOK_HPP

#include <unordered_map>
#include "sim_book_price_levels.hpp"
#include <ats/message/order_status_message.hpp>

namespace ats {
namespace sim
{
	class sim_book
	{
	public:
		typedef price_levels<std::greater<ats::price_t>> bid_container;
		typedef price_levels<std::less<ats::price_t>> ask_container;
		typedef std::unordered_map<ats::orderid_t, price_level::iterator> order_container;

		void add_order(const ats::limit_order& order);
		void insert_order(const ats::limit_order& order);
		void cancel_order(const ats::orderid_t& id, const ats::timestamp_t& time);

		void process_trade(ats::price_t price, long quantity, const ats::timestamp_t& time);

		void add_order_status_listener(const ats::order_status_handler& listener)
		{
			order_status_listener_ = listener;
			bids_.add_order_status_listener(listener);
			asks_.add_order_status_listener(listener);
		}

		const price_level* best_bid() const { return bids_.top_level(); }
		price_level* best_bid() { return bids_.top_level(); }
		const price_level* best_ask() const { return asks_.top_level(); }
		price_level* best_ask() { return asks_.top_level(); }

		const price_level* get_level(ats::price_t price, bool is_bid) const
		{ return is_bid ? bids_.get_level(price) : asks_.get_level(price); }

		const order_container& get_sim_orders() const { return sim_orders_; }

		// Remove orders that are not mine
		void clean_level(ats::price_t, bool is_bid);
		void execute_all_orders(ats::price_t price, const ats::timestamp_t& time, bool is_bid);
		void execute_orders(ats::price_t price, long quantity, const ats::timestamp_t& time, bool is_bid);
		void execute_crosses(const ats::price_t* bid, const ats::price_t* ask, const ats::timestamp_t& time);

//	private:
		void process_change_msg(const ats::level2_message& msg);
		void process_delete_msg(const ats::level2_message& msg);
		void process_trade_msg(const ats::level2_message& msg);
		void process_insert_msg(const ats::level2_message& msg);
		void process_level2_msg(const ats::level2_message& msg);

	private:
		bid_container bids_;
		ask_container asks_;
		order_container sim_orders_;
		ats::order_status_handler order_status_listener_ = nullptr;
	};


	inline void sim_book::add_order(const ats::limit_order& order)
	{
		// First, check for crosses, then add if there still is a quantity left
		price_level::iterator order_pos;
		if (order.side() == ats::order_side::Buy || order.side() == ats::order_side::BuyCover)
		{
			if (best_ask() != nullptr && order.price() >= best_ask()->price())
			{
				long unexecuted_qty = order.quantity();
				while (best_ask() != nullptr && order.price() >= best_ask()->price() && unexecuted_qty > 0)
				{
					if (unexecuted_qty <= best_ask()->quantity)
					{
						ats::order_status_filled_message msg(order.id(), order.transact_time, best_ask()->price(), unexecuted_qty);
						execute_orders(best_ask()->price(), unexecuted_qty, order.transact_time, false);

						if (order_status_listener_ != nullptr)
							order_status_listener_(msg);
						return;
					}
					else
					{
						ats::order_status_partially_filled_message msg(order.id(), order.transact_time, best_ask()->price(), best_ask()->quantity);
						unexecuted_qty -= best_ask()->quantity;
						execute_all_orders(best_ask()->price(), order.transact_time, false);

						if (order_status_listener_ != nullptr)
							order_status_listener_(msg);
					}
				}

				if (unexecuted_qty > 0)
				{
					ats::limit_order reduced_order(order.id(), order.symbol(), unexecuted_qty, order.side(),
							order.time_in_force(), order.price());
					order_pos = bids_.add_order(reduced_order);
				}
			}
			else
				order_pos = bids_.add_order(order);
		}
		else
		{
			if (best_bid() != nullptr && order.price() <= best_bid()->price())
			{
				long unexecuted_qty = order.quantity();
				while (best_bid() != nullptr && order.price() <= best_bid()->price() && unexecuted_qty > 0)
				{
					if (unexecuted_qty <= best_bid()->quantity)
					{
						ats::order_status_filled_message msg(order.id(), order.transact_time, best_bid()->price(), unexecuted_qty);
						execute_orders(best_bid()->price(), unexecuted_qty, order.transact_time, true);

						if (order_status_listener_ != nullptr)
							order_status_listener_(msg);
						return;
					}
					else
					{
						ats::order_status_partially_filled_message msg(order.id(), order.transact_time, best_bid()->price(), best_bid()->quantity);
						unexecuted_qty -= best_bid()->quantity;
						execute_all_orders(best_bid()->price(), order.transact_time, true);

						if (order_status_listener_ != nullptr)
							order_status_listener_(msg);
					}
				}

				if (unexecuted_qty > 0)
				{
					ats::limit_order reduced_order(order.id(), order.symbol(), unexecuted_qty, order.side(),
							order.time_in_force(), order.price());
					order_pos = asks_.add_order(reduced_order);
				}
			}
			else
				order_pos = asks_.add_order(order);
		}

		// For bookkeeping
		if (order.id() != 0)
			sim_orders_.insert(std::make_pair(order.id(), order_pos));
	}

	inline void sim_book::insert_order(const ats::limit_order& order)
	{
		price_level::iterator order_pos;
		if (order.side() == ats::order_side::Buy || order.side() == ats::order_side::BuyCover)
			order_pos = bids_.insert_order(order);
		else
			order_pos = asks_.insert_order(order);

		if (order.id() != 0)
			sim_orders_.insert(std::make_pair(order.id(), order_pos));
	}

	inline void sim_book::cancel_order(const ats::orderid_t& id, const ats::timestamp_t& time)
	{
		auto find = sim_orders_.find(id);
		if (find == sim_orders_.end()) return;

		auto pos = find->second;
		if (pos->side() == ats::order_side::Buy || pos->side() == ats::order_side::BuyCover)
			bids_.cancel_order(pos, time);
		else
			asks_.cancel_order(pos, time);

		sim_orders_.erase(find);

		if (order_status_listener_ != nullptr)
			order_status_listener_(ats::order_status_cancelled_message(id, time, "Canceled by trader"));
	}

	inline void sim_book::process_trade(price_t price, long quantity, const ats::timestamp_t& time)
	{
		// Check for crosses
		while (best_bid() != nullptr && price < best_bid()->price())
			execute_all_orders(best_bid()->price(), time, true);

		while (best_ask() != nullptr && price > best_ask()->price())
			execute_all_orders(best_ask()->price(), time, false);

		if (best_bid() != nullptr && price == best_bid()->price())
		{

		}
		else if (best_ask() != nullptr && price == best_ask()->price())
		{

		}
	}

	inline void sim_book::clean_level(ats::price_t price, bool is_bid)
	{
		if (is_bid)
			bids_.clean(price);
		else
			asks_.clean(price);
	}

	inline void sim_book::execute_all_orders(ats::price_t price, const ats::timestamp_t& time, bool is_bid)
	{
		if (is_bid)
			bids_.execute_all_orders(price, time, sim_orders_);
		else
			asks_.execute_all_orders(price, time, sim_orders_);
	}

	inline void sim_book::execute_orders(ats::price_t price, long quantity, const ats::timestamp_t& time, bool is_bid)
	{
		if (is_bid)
			bids_.execute_orders(price, quantity, time, sim_orders_);
		else
			asks_.execute_orders(price, quantity, time, sim_orders_);
	}

	inline void sim_book::execute_crosses(const ats::price_t* bid, const ats::price_t* ask, const ats::timestamp_t& time)
	{
		if (bid != nullptr)
		{
			while (best_ask() != nullptr && best_ask()->price() <= *bid)
				execute_all_orders(best_ask()->price(), time, false);
		}

		if (ask != nullptr)
		{
			while (best_bid() != nullptr && best_bid()->price() >= *ask)
				execute_all_orders(best_bid()->price(), time, true);
		}
	}


	inline void sim_book::process_change_msg(const ats::level2_message& msg)
	{
		if (msg.entry_type == ats::entry_type::Bid)
		{
			while (best_ask() != nullptr && msg.price >= best_ask()->price())
				execute_all_orders(best_ask()->price(), msg.time, false);

			bids_.process_change_msg(msg, sim_orders_);
		}
		else
		{
			while (best_bid() != nullptr && msg.price < best_bid()->price())
				execute_all_orders(best_bid()->price(), msg.time, true);

			asks_.process_change_msg(msg, sim_orders_);
		}
	}

	inline void sim_book::process_delete_msg(const ats::level2_message& msg)
	{
		if (msg.entry_type == ats::entry_type::Bid)
			bids_.process_delete_msg(msg, sim_orders_);
		else
			asks_.process_delete_msg(msg, sim_orders_);
	}

	inline void sim_book::process_trade_msg(const ats::level2_message& msg)
	{
		process_trade(msg.price, msg.quantity, msg.time);

		if (best_bid() != nullptr && msg.price == best_bid()->price())
			best_bid()->traded_quantity = msg.quantity;
		else if (best_ask() != nullptr && msg.price == best_ask()->price())
			best_ask()->traded_quantity = msg.quantity;
	}

	inline void sim_book::process_insert_msg(const ats::level2_message& msg)
	{
		if (msg.entry_type == ats::entry_type::Bid)
			bids_.process_insert_msg(msg);
		else
			asks_.process_insert_msg(msg);
	}

	inline void sim_book::process_level2_msg(const ats::level2_message& msg)
	{
		if (msg.entry_type == ats::entry_type::Trade)
			process_trade_msg(msg);
		else
		{
			switch (msg.update_action)
			{
			case ats::update_action::Change:
				process_change_msg(msg);
				break;
			case ats::update_action::Delete:
				process_delete_msg(msg);
				break;
			case ats::update_action::New:
				process_insert_msg(msg);
				break;
			default:
				break;
			}
		}
	}
}
}

#endif