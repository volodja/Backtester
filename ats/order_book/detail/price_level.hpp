#ifndef PRICE_LEVEL_HPP
#define PRICE_LEVEL_HPP

#include <ats/types.hpp>

namespace ats {
namespace order_book_detail
{
	/// @brief price level in an order book
	struct price_level
	{
		ats::price_t price;
		long quantity; // accumulated (limit orders) size at the level
		unsigned int order_count; // number of (limit) orders outstanding at the level

		price_level()
			: price(0), quantity(0), order_count(0) { }
		price_level(ats::price_t price, long quantity, unsigned int order_count)
			: price(price), quantity(quantity), order_count(order_count) { }

		void set(ats::price_t price, long quantity, unsigned int order_count)
		{
			this->price = price;
			this->quantity = quantity;
			this->order_count = order_count;
		}
	};
}
}

#endif