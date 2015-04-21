#ifndef ORDER_PROCESSOR_HPP
#define ORDER_PROCESSOR_HPP

#include <vector>
#include <ats/order_book/order_book.hpp>
#include <ats/position/position.hpp>
#include <ats/types.hpp>

namespace ats
{
	class order_processor
	{
	public:
		const ats::order_book& get_order_book(const ats::symbol_key& symbol) const
		{
			return order_books_[symbol.index];
		}

		const ats::position& get_position(const ats::symbol_key& symbol) const
		{
			return positions_[symbol.index];
		}

		const std::vector<ats::position>& get_all_positions() const { return positions_; }

	public:
		void create_order_book(const ats::symbol_key& symbol)
		{
			order_books_.push_back(ats::order_book(symbol));
		}

		void on_add_security(const ats::symbol_key& symbol, const std::string& exchange, size_t book_depth)
		{

		}

	private:
		std::vector<ats::order_book> order_books_;
		std::vector<ats::position> positions_;
	};
}

#endif