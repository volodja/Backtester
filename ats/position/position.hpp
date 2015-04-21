#ifndef POSITION_HPP
#define POSITION_HPP

#include <cstdint>
#include <stack>
#include <memory>
#include <ats/types.hpp>
#include <ats/message/order_status_message.hpp>

namespace ats
{
	struct position_offset_message : ats::message
	{
		std::string symbol;
		ats::price_t price;
		long quantity;
		ats::price_t profit_ticks;
		ats::timestamp_t close_time;
		bool is_long;
	};

	class position
	{
	private:
		struct prc_qty
		{
			prc_qty(price_t price, long quantity)
				: price(price), quantity(quantity) { }
			price_t price;
			long quantity;
		};
	public:
		position(const ats::symbol_key& symbol)
			: quantity_(0), is_long_(true), avg_price_(0.0), symbol_(symbol) { }

		long quantity() const { return quantity_; }
		const ats::timestamp_t& time() const { return time_; }
		bool is_long() const { return is_long_; }
		double price() const { return avg_price_; }
		long realized_pnl() const { return realized_pnl_; }
		long inventory() const
		{
			if (quantity_ == 0)
				return 0;
			else
				return is_long_ ? quantity_ : -quantity_;
		}

		void add_execution(ats::order_side side, long quantity, ats::price_t price, const ats::timestamp_t& time)
		{
			bool is_long = side == ats::order_side::Buy || side == ats::order_side::BuyCover;
			add(price, quantity, is_long);
		}

	private:
		void add(int price, uint16_t quantity, bool is_long)
		{
			if (open_pos_.empty())
			{
//				avg_price_ = price;
				open_pos_.push(prc_qty(price, quantity));
				is_long_ = is_long;
				quantity_ = quantity;
			}
			else if (is_long == is_long_)
			{
//				avg_price_ = (avg_price_ * open_pos_.size() + price) / (open_pos_.size() + 1);
				open_pos_.push(prc_qty(price, quantity));
				quantity_ += quantity;
			}
			else
			{
				short sgn = is_long_ ? 1 : -1;

				int qty_remained = quantity;
				while (!open_pos_.empty())
				{
					int prc = open_pos_.top().price;
					int qty = open_pos_.top().quantity;
					if (qty_remained <= qty)
					{
						realized_pnl_ += sgn * qty_remained * (price - prc);
						if (qty_remained == qty)
							open_pos_.pop();
						else
							open_pos_.top().quantity -= qty_remained;

//						quantity_ -= qty_remained;
						qty_remained = 0;
						break;
					}
					else
					{
						realized_pnl_ += sgn * qty * (price - prc);
						qty_remained -= qty;
						open_pos_.pop();
					}
				}

				if (qty_remained > 0)
				{
					open_pos_.push(prc_qty(price, qty_remained));
					is_long_ = is_long;
					quantity_ = qty_remained;
				}
				else
					quantity_ -= quantity;
			}
		}

	private:
		long quantity_;
		bool is_long_;
		double avg_price_;
		long realized_pnl_ = 0;
		long unrealized_pnl_ = 0;
		ats::timestamp_t time_;         // time when the position was updated

//		std::stack<std::pair<int, int>> open_pos_;
		std::stack<prc_qty> open_pos_;

//		std::unordered_map<price_t, long> open_positions_;
		ats::symbol_key symbol_;
	};
}

#endif