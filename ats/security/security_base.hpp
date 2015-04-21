#ifndef SECURITY_BASE_HPP
#define SECURITY_BASE_HPP

#include <iosfwd>
#include <string>
#include <map>
#include <functional>
#include <memory>
#include <list>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

#include <boost/circular_buffer.hpp>

#include <ats/position/position.hpp>
#include <ats/order_book/order_book.hpp>
#include <ats/message/level2_message.hpp>
#include <ats/message/trade_message.hpp>
#include <ats/message/order_status_message.hpp>
#include "bar.hpp"

namespace ats
{
	class portfolio_base;

	class security_base
	{
	public:
		typedef boost::circular_buffer<ats::bar> bar_container;
		typedef std::function<void(const ats::timestamp_t&)> time_listener;
	public:
		security_base(const ats::symbol_key& symbol, portfolio_base* portfolio)
			: symbol_(symbol), order_book_(symbol), portfolio_(portfolio)
		{
			on_init();
		}

		security_base(const ats::symbol_key& symbol, portfolio_base* portfolio,
				const boost::posix_time::time_duration bar_periodicity,	size_t bars_to_store)
			: symbol_(symbol), order_book_(symbol), portfolio_(portfolio),
			  bars_(bars_to_store), bar_periodicity_(bar_periodicity)
		{
			on_init();
		}

		virtual ~security_base()
		{
			on_exit();
		}

	// Basic accessors
	public:
		std::ofstream& LOG();
		const ats::timestamp_t& current_time() const;
		const ats::position& get_position() const;
		const ats::order_book& get_order_book(const ats::symbol_key& symbol) const;

		const ats::symbol_key& symbol() const { return symbol_; }
		const ats::order_book& order_book() const { return order_book_; }
		const ats::exchange_order_book* exchange_order_book(const std::string& exchange) const { return order_book_.get(exchange); }

		const ats::timestamp_t& last_update_time() const { return last_update_time_; }
		const ats::price_t& last_price() const { return last_price_; }

		long get_inventory() const { return get_position().inventory(); }

	public:
		void process_time_update(const ats::timestamp_t& time)
		{
			// Notify all interested listeners about the reception of a new message
			for (const auto& l : time_listeners_)
				l(time);
		}

		void process_message(const ats::level2_message_packet& msg)
		{
			// Update the time of the last received message
			last_update_time_ = msg.time;

			// Transform the message into a "message with deltas"
			ats::level2_message_packet msg_delta = msg;
			for (auto& m : msg_delta.messages)
			{
				order_book_.update2(m);
				if (m.entry_type == ats::entry_type::Trade)
				{
					update_bars(m.time, m.price, m.quantity);
					last_price_ = m.price;
				}
			}

			process_time_update(msg.time);

			// Respond to the new message
			on_order_book_changed(msg_delta);
		}

		void create_order_book(const std::string& exchange, size_t book_depth)
		{
			order_book_.add_order_book(exchange, book_depth);
		}

	// Events
	public:
		virtual void on_init() { }
		virtual void on_exit() { }

		virtual void on_order_status_changed(const ats::order_status_message& msg) { }
//		virtual void on_order_book_changed(const ats::level2_message& msg) { }
		virtual void on_order_book_changed(const ats::level2_message_packet& msg) { }
		virtual void on_trade(const ats::trade_message& msg) { }

		virtual void on_bar_open(const ats::bar& bar) { }
		virtual void on_bar_close(const ats::bar& bar) { }

		const bar_container& bars() const { return bars_; }

		const ats::bar* current_bar() const
		{
			return bars_.size() != 0 ? &bars_[0] : nullptr;
		}

		void set_bar_parameters(const boost::posix_time::time_duration& bar_periodicity, size_t bars_to_store)
		{
			bar_periodicity_ = bar_periodicity;
			bars_.set_capacity(bars_to_store);
		}

		void update_bars(const ats::timestamp_t& time, ats::price_t price, long quantity);

		void add_time_listener(const time_listener& listener) { time_listeners_.push_back(listener); }

//	private:
	public:
		ats::symbol_key symbol_;
		ats::order_book order_book_;
//		std::string exchange_;
//		const ats::order_book* orderbook_;
//		ats::position position_;

		// To construct bars
		bar_container bars_;
		boost::posix_time::time_duration bar_periodicity_;

		ats::timestamp_t last_update_time_;
		ats::price_t last_price_ = 0;

		std::list<time_listener> time_listeners_;

	protected:
		ats::portfolio_base* portfolio_;
	};






	inline void security_base::update_bars(const ats::timestamp_t& time, ats::price_t price, long quantity)
	{
		if (bars_.capacity() == 0) return;

		// NOTE: New bars will always be pushed forward so that bars_[0] was the last bar

		if (bars_.empty())
		{
			ats::date_time::date_time time_close = time.date();
			while (time_close <= time)
				time_close += bar_periodicity_;

			ats::bar bar;
			bar.open = bar.high = bar.low = bar.close = price;
			bar.quantity = quantity;
			bar.time_open = time_close - bar_periodicity_;
			bars_.push_front(bar);

			on_bar_open(bar);
		}
		else
		{
			ats::bar& last_bar = *bars_.begin();
			if (time - last_bar.time_open < bar_periodicity_)
			{
				last_bar.quantity += quantity;
				last_bar.close = price;
				if (price < last_bar.low)
					last_bar.low = price;
				else if (price > last_bar.high)
					last_bar.high = price;
			}
			else
			{
				on_bar_close(last_bar);

				ats::bar new_bar;
				new_bar.open = new_bar.high = new_bar.low = new_bar.close = price;
				new_bar.quantity = quantity;
				new_bar.time_open = last_bar.time_open + bar_periodicity_;

				// Insert empty bars if there were no trades for a long time
				ats::date_time::date_time time_close(new_bar.time_open + bar_periodicity_);
				while (time_close < time)
				{
					ats::bar empty_bar;
					empty_bar.open = empty_bar.high = empty_bar.low = empty_bar.close = last_bar.close;
					empty_bar.quantity = 0;
					empty_bar.time_open = new_bar.time_open;
					bars_.push_front(empty_bar);

					on_bar_close(empty_bar);

					new_bar.time_open = time_close;
					time_close += bar_periodicity_;
				}

				bars_.push_front(new_bar);

				on_bar_open(new_bar);
			}
		}
	}
}

#endif