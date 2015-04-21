#pragma once

#include <string>
#include <memory>
#include <functional>
#include <list>
#include <typeindex>
#include <ats/order_book/order_book.hpp>
#include <ats/position/position.hpp>
#include <ats/order/order.hpp>
#include <ats/event_handler/event_handler.hpp>
#include <ats/types.hpp>
#include "handler_types.hpp"

#include <ats/portfolio/portfolio_base.hpp>

namespace ats
{
	class order_processor
	{
		typedef std::shared_ptr<ats::order> order_ptr;
	public:
		order_processor(ats::portfolio_base* portfolio)
			: portfolio_(portfolio) { }

		const ats::order_book& get_order_book(const ats::symbol_key& symbol) const	{ return portfolio_->get_order_book(symbol); }
		const ats::order_book& get_position(const ats::symbol_key& symbol) const { return positions_[symbol.index];	}
		const std::vector<ats::position>& get_positions() const { return positions_; }
		std::vector<const ats::position*> get_long_positions() const
		{
			std::vector<const ats::position*> result;
			for (const ats::position& pos : positions_)
				if (pos.quantity() > 0)
					result.push_back(&pos);
			return result;
		}
		std::vector<const ats::position*> get_short_positions() const
		{
			std::vector<const ats::position*> result;
			for (const ats::position& pos : positions_)
				if (pos.quantity() < 0)
					result.push_back(&pos);
			return result;
		}

		const ats::order* get_order(const ats::orderid_t& order_id) const
		{
			auto it = orders_.find(order_id);
			return it != orders_.cend() ? it->second.get() : nullptr;
		}
/*		const std::unordered_map<ats::orderid_t, order_ptr>& get_orders() const
		{
			return orders_;
		}*/

		std::vector<order_ptr> get_orders(const std::string& symbol) const
		{
			std::vector<order_ptr> result;
			for (auto it = orders_.cbegin(); it != orders_.cend(); ++it)
				if (it->second->symbol() == symbol)
					result.push_back(it->second);
			return result;
		}

		std::vector<order_ptr> get_pending_orders() const
		{
			std::vector<order_ptr> result;
			for (auto it = orders_.cbegin(); it != orders_.cend(); ++it)
				if (it->second->is_pending())
					result.push_back(it->second);
			return result;
		}
		std::vector<order_ptr> get_pending_orders(const std::string& symbol) const
		{
			std::vector<order_ptr> result;
			for (auto it = orders_.cbegin(); it != orders_.cend(); ++it)
				if (it->second->is_pending() && it->second->symbol() == symbol)
					result.push_back(it->second);
			return result;

		}

		ats::orderid_t generate_order_id() const
		{
			static ats::orderid_t id = 1;
			return id++;
		}

		void clear_orders()
		{
			orders_.clear();
		}

	public:
		void add_execution_to_position(const ats::symbol_key& symbol, ats::order_side side, ats::quantity_t quantity,
				ats::price_t price, const ats::timestamp_t& time)
		{
			ats:position& pos = positions_[symbol.index];
			long original_qty = pos.quantity();
			pos.add_execution(side, quantity, price, time);

			if (original_qty != pos.quantity())
			{
				for (auto& h : position_size_change_handlers_)
					h(pos);

				for (auto& h : position_change_handlers_)
					h(pos);
			}
		}

	// Working with orders:
	public:
		void cancel_order(const ats::orderid_t& order_id);
		void cancel_pending_orders();
		void cancel_pending_orders(const std::string& symbol);
//		void cancel_replace_order(const ats::order& original_order, const ats::order& replacement_order);
		void send_order(const ats::order& order);

	public:
		void add_order_status_listener(const ats::order_status_handler& handler)
		{
			order_status_handlers_ += handler;
		}
//		void add_order_cancel_rejected_listener();

		void add_position_size_change_listener(const ats::position_change_handler& handler)
		{
			position_size_change_handlers_.push_back(handler);
		}

		void add_position_change_listener(const ats::position_change_handler& handler)
		{
			position_change_handlers_.push_back(handler);
		}

	private:
		void remove_inactive_orders()
		{
			for (auto it = orders_.begin(); it != orders_.end();)
			{
				if (!it->second->is_pending())
					orders_.erase(it++);
				else
					++it;
			}
		}

		void remove_orders_by_tif(const ats::order_time_in_force& tif)
		{
			for (auto it = orders_.begin(); it != orders_.end();)
			{
				if (it->second->time_in_force() == tif)
					orders_.erase(it++);
				else
					++it;
			}
		}

		const ats::timestamp_t& current_time() const { return portfolio_->current_time(); }

	public:
		// If true inactive orders will be deleted from memory immediately after final state event is sent
		bool auto_clear_inactive_orders = false;

	private:
		ats::execution_engine* get_execution_engine(const std::string& exchange)
		{
			return portfolio_->get_execution_engine(exchange);
		}
		void send_market_order(const ats::market_order& order);
		void send_stop_order(const ats::stop_order& order);

	private:
		ats::portfolio_base* portfolio_;

//		std::vector<ats::order_book> order_books_;
		std::vector<ats::position> positions_;
		std::unordered_map<ats::orderid_t, order_ptr> orders_;

		// Handlers
		ats::event_handler<void(const ats::order_status_message&)> order_status_handlers_;
//		std::list<ats::order_status_handler> order_status_handlers_;
		std::list<ats::position_change_handler> position_size_change_handlers_;
		std::list<ats::position_change_handler> position_change_handlers_;
	};
}


namespace ats
{
	inline void order_processor::cancel_order(const ats::orderid_t& order_id)
	{
		auto it = orders_.find(order_id);
		if (it == orders_.cend()) return;

		ats::execution_engine* engine = get_execution_engine(it->second->exchange());
		engine->cancel_order(order_id);

		ats::order_status_cancelled_message msg(order_id, current_time(), "");

		if (auto_clear_inactive_orders)
			orders_.erase(it);
		else
			it->second->set_status(ats::order_status::Canceled);

		order_status_handlers_(msg);
	}

	inline void order_processor::cancel_pending_orders()
	{
		ats::execution_engine* engine;
		for (auto it = orders_.begin(); it != orders_.end();)
		{
			order_ptr& order = it->second;
			if (order->is_pending())
			{
				engine = get_execution_engine(order->exchange());
				engine->cancel_order(order->id());

				ats::order_status_cancelled_message msg(order->id(), current_time(), "");

				if (auto_clear_inactive_orders)
					orders_.erase(it++);
				else
				{
					order->set_status(ats::order_status::Canceled);
					++it;
				}

				order_status_handlers_(msg);
			}
		}
	}

	inline void order_processor::cancel_pending_orders(const std::string& symbol)
	{
		ats::execution_engine* engine;
		for (auto it = orders_.begin(); it != orders_.end();)
		{
			order_ptr& order = it->second;
			if (order->is_pending() && order->symbol() == symbol)
			{
				engine = get_execution_engine(order->exchange());
				engine->cancel_order(order->id());

				ats::order_status_cancelled_message msg(order->id(), ats::timestamp_t::now(), "");

				if (auto_clear_inactive_orders)
					orders_.erase(it++);
				else
				{
					order->set_status(ats::order_status::Canceled);
					++it;
				}

				order_status_handlers_(msg);
			}
		}
	}

	inline void order_processor::send_market_order(const ats::market_order& order)
	{
		auto it = orders_.insert(std::make_pair(order.id(), std::make_shared<ats::order>(order))).first;
		order_status_handlers_(ats::order_status_pending_new_message(order.id(), current_time()));

		const ats::symbol_key& symbol = portfolio_->get_symbol_key(order.symbol());
		const ats::exchange_order_book* book = get_order_book(symbol).get(order.exchange());
		if (book == nullptr)
		{
			std::string reason = "Order book for symbol '" << order.symbol() << "' and exchange '" +
					order.exchange() + "' does not exist.";

			if (!auto_clear_inactive_orders)
				it->second->set_status(ats::order_status::Rejected);
			else
				orders_.erase(it);

			order_status_handlers_(ats::order_status_rejected_message(order.id(), current_time(), reason));

			return;
		}

		if (order.side() == ats::order_side::Buy || order.side() == ats::order_side::BuyCover)
		{
			if (!book->asks().empty())
			{
				if (!auto_clear_inactive_orders)
					it->second->set_status(ats::order_status::Filled);
				else
					orders_.erase(it);

				order_status_handlers_(ats::order_status_filled_message(order.id(), current_time(), book->best_ask_price(), order.quantity()));
			}
			else
			{
				if (!auto_clear_inactive_orders)
					it->second->set_status(ats::order_status::Rejected);
				else
					orders_.erase(it);

				order_status_handlers_(ats::order_status_rejected_message(order.id(), current_time(), "No asks in order book"));
			}
		}
		else
		{
			if (!book->bids().empty())
			{
				if (!auto_clear_inactive_orders)
					it->second->set_status(ats::order_status::Filled);
				else
					orders_.erase(it);

				order_status_handlers_(ats::order_status_filled_message(order.id(), current_time(), book->best_bid_price(), order.quantity()));
			}
			else
			{
				if (!auto_clear_inactive_orders)
					it->second->set_status(ats::order_status::Rejected);
				else
					orders_.erase(it);

				order_status_handlers_(ats::order_status_rejected_message(order.id(), current_time(), "No bids in order book"));
			}
		}
	}

	inline void order_processor::send_stop_order(const ats::stop_order& order)
	{

	}

	inline void order_processor::send_order(const ats::order& order)
	{
		std::type_index order_type(typeid(order));

/*		switch (order_type)
		{
		case typeid(ats::market_order):
			send_market_order(static_cast<const ats::market_order&>(order));
			break;
		default:
			break;
		}*/

		if (order_type == typeid(ats::market_order))
			send_market_order(static_cast<const ats::market_order&>(order));
		else if (order_type == typeid(ats::stop_order))
			send_stop_order(static_cast<const ats::stop_order&>(order));
	}
}
