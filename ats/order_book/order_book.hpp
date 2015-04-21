#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <unordered_map>
#include "exchange_order_book.hpp"
#include <ats/message/level2_message.hpp>
#include <ats/types.hpp>

namespace ats
{
	class order_book
	{
	public:
		order_book(const ats::symbol_key& symbol) : symbol_(symbol) { }

		const ats::symbol_key& symbol() const { return symbol_; }

		void add_order_book(const std::string& exchange, size_t book_depth)
		{
			orderbooks_.insert(std::make_pair(exchange, ats::exchange_order_book(symbol_, exchange, book_depth)));
		}

		ats::exchange_order_book* get(const std::string& exchange)
		{
			auto it = orderbooks_.find(exchange);
			return it != orderbooks_.cend() ? &it->second : nullptr;
		}

		const ats::exchange_order_book* get(const std::string& exchange) const
		{
			auto it = orderbooks_.find(exchange);
			return it != orderbooks_.cend() ? &it->second : nullptr;
		}

		void update(const ats::level2_message& msg)
		{
			ats::exchange_order_book* book = get(msg.exchange);
			if (book != nullptr)
				book->update(msg);
		}

		void update2(ats::level2_message& msg)
		{
			ats::exchange_order_book* book = get(msg.exchange);
			if (book != nullptr)
				book->update2(msg);
		}
	private:
		ats::symbol_key symbol_;
		std::unordered_map<std::string, ats::exchange_order_book> orderbooks_;
	};
}

#endif