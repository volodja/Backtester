#ifndef ORDERBOOK_MANAGER_HPP
#define ORDERBOOK_MANAGER_HPP

#include <vector>
#include "order_book.hpp"
#include <ats/message/level2_message.hpp>
#include <ats/types.hpp>

namespace ats
{
	class order_book_manager
	{
	public:
		const std::vector<ats::order_book>& get_order_books() const { return books_; }
		std::vector<ats::order_book>& get_order_books() { return books_; }
		const ats::order_book& get_order_book(const ats::symbol_key& symbol) const { return books_[symbol.index]; }
		ats::order_book& get_order_book(const ats::symbol_key& symbol) { return books_[symbol.index]; }

		void create_order_book(const ats::symbol_key& symbol)
		{
			if (symbol.index == books_.size())
				books_.push_back(ats::order_book(symbol));
			else
				throw std::invalid_argument("Cannot create an order book");
		}

		void update_order_book(const ats::symbol_key& symbol, const ats::level2_message& msg)
		{
			books_[symbol.index].update(msg);
		}
	private:
		std::vector<ats::order_book> books_;
	};
}

#endif