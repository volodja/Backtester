#include "security_base.hpp"
#include <ats/portfolio/portfolio_base.hpp>

namespace ats
{
	std::ofstream& security_base::LOG()
	{
		return portfolio_->LOG();
	}

	const ats::timestamp_t& security_base::current_time() const
	{
		return portfolio_->current_time();
	}

	const ats::order_book& security_base::get_order_book(const ats::symbol_key& symbol) const
	{
		return portfolio_->get_order_book(symbol);
	}

	const ats::position& security_base::get_position() const
	{
		return portfolio_->get_position(symbol());
	}
}
