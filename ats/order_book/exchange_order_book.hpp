#ifndef EXCHANGE_ORDERBOOK_HPP
#define EXCHANGE_ORDERBOOK_HPP

#include <iosfwd>
#include <fstream>
#include <ats/order_book/detail/price_levels.hpp>
#include <ats/message/level2_message.hpp>
#include <ats/types.hpp>

namespace ats
{
	class exchange_order_book
	{
	public:
		typedef ats::order_book_detail::price_levels<std::greater<ats::price_t>> bids_type;
		typedef ats::order_book_detail::price_levels<std::less<ats::price_t>> asks_type;
		typedef bids_type::iterator bid_iterator;
		typedef bids_type::const_iterator bid_const_iterator;
		typedef asks_type::iterator ask_iterator;
		typedef asks_type::const_iterator ask_const_iterator;
		typedef ats::order_book_detail::price_level price_level_type;

		exchange_order_book(const ats::symbol_key& symbol, const std::string& exchange, size_t book_depth)
			: max_levels_(book_depth), bids_(book_depth), asks_(book_depth),
			  symbol_(symbol), exchange_(exchange) { }

		void update(const ats::level2_message& msg)
		{
			last_update_time_ = msg.time;

			if (msg.entry_type == ats::entry_type::Bid)
				bids_.update(msg);
			else if (msg.entry_type == ats::entry_type::Ask)
				asks_.update(msg);
		}

		void update2(ats::level2_message& msg)
		{
			last_update_time_ = msg.time;

			if (msg.entry_type == ats::entry_type::Bid)
				bids_.update2(msg);
			else if (msg.entry_type == ats::entry_type::Ask)
				asks_.update2(msg);
			else if (msg.entry_type == ats::entry_type::Trade && msg.aggressor_side == 0)
			{
				if (!asks_.empty() && msg.price >= asks_.cbegin()->first)
					msg.aggressor_side = 1;
				else if (!bids_.empty() && msg.price <= bids_.cbegin()->first)
					msg.aggressor_side = -1;
			}
		}

		bid_iterator begin_bid() { return bids_.begin(); }
		bid_const_iterator cbegin_bid() const { return bids_.cbegin(); }
		ask_iterator begin_ask() { return asks_.begin(); }
		ask_const_iterator cbegin_ask() const { return asks_.cbegin(); }
		bid_iterator end_bid() { return bids_.end(); }
		bid_const_iterator cend_bid() const { return bids_.cend(); }
		ask_iterator end_ask() { return asks_.end(); }
		ask_const_iterator cend_ask() const { return asks_.cend(); }

//		ats::price_t best_bid_price() const { return cbegin_bid()->first; }
//		ats::price_t best_ask_price() const { return cbegin_ask()->first; }
//		const price_level_type& best_bid_depth() const { return cbegin_bid()->second; }
//		const price_level_type& best_ask_depth() const { return cbegin_ask()->second; }

		size_t displayed_depth() const { return max_levels_; }

		void clear() { bids_.clear(); asks_.clear(); }

		int bid_ask_spread() const { return asks_.cbegin()->first - bids_.cbegin()->first; }

/*		const level_depth& depth_at(const price_t& price, const book_side& side) const
		{
			return (side == book_side::bid) ? *bids.find(price) : *asks.find(price);
		}*/

		const price_level_type* best_bid() const
		{
			return !bids_.empty() ? &bids_.cbegin()->second : nullptr;
		}

		const price_level_type* best_ask() const
		{
			return !asks_.empty() ? &asks_.cbegin()->second : nullptr;
		}

		const price_level_type* bid_at(ats::price_t price) const
		{
			auto it = bids_.find(price);
			return it != bids_.cend() ? &it->second : nullptr;
		}

		const price_level_type* ask_at(ats::price_t price) const
		{
			auto it = asks_.find(price);
			return it != asks_.cend() ? &it->second : nullptr;
		}


		double midpoint() const
		{
			return !bids_.empty() && !asks_.empty() ? (double)(bids_.cbegin()->first + asks_.cbegin()->first) / 2.0 : 0.0;
		}

		friend std::ostream& operator<<(std::ostream& os, const ats::exchange_order_book& book)
		{
			std::vector<std::string> bids;
			std::vector<std::string> asks;

			std::stringstream ss;
			size_t max_bid_len = 0;
			for (auto it = book.bids().cbegin(); it != book.bids().cend(); ++it)
			{
				ss << it->first << "(" << it->second.quantity << ")";
				std::string text = ss.str();
				if (text.length() > max_bid_len)
					max_bid_len = text.length();
				bids.push_back(text);
				ss.str("");
				ss.clear();
			}

			for (auto it = book.asks().cbegin(); it != book.asks().cend(); ++it)
			{
				ss << it->first << "(" << it->second.quantity << ")";
				asks.push_back(ss.str());
				ss.str("");
				ss.clear();
			}

			auto it_b = bids.begin();
			auto it_a = asks.begin();
			for (; it_b != bids.end() && it_a != asks.end(); ++it_b, ++it_a)
			{
				os << *it_b;
				size_t dl = max_bid_len - it_b->length();
				for (size_t i = 0; i < dl; ++i)
					os << " ";
				os << " | " << *it_a << '\n';
			}

			for (; it_b != bids.end(); ++it_b)
			{
				os << *it_b;
				size_t dl = max_bid_len - it_b->length();
				for (size_t i = 0; i < dl; ++i)
					os << " ";
				os << " |\n";
			}

			for (; it_a != asks.end(); ++it_a)
			{
				for (size_t i = 0; i < max_bid_len; ++i)
					os << " ";
				os << " | " << *it_a << '\n';
			}

			return os;
		}

		bids_type& bids() { return bids_; }
		const bids_type& bids() const { return bids_; }
		asks_type& asks() { return asks_; }
		const asks_type& asks() const { return asks_; }
		const ats::symbol_key& symbol() const { return symbol_; }
		const std::string& exchange() const { return exchange_; }
		const ats::timestamp_t& last_update_time() const { return last_update_time_; }

	private:
		size_t max_levels_;
		bids_type bids_;
		asks_type asks_;
		ats::symbol_key symbol_;
		std::string exchange_;
		ats::timestamp_t last_update_time_;
	};
}

#endif