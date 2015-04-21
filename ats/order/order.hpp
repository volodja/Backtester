#ifndef ORDER_HPP
#define ORDER_HPP

#include <string>
#include <memory>
#include <typeindex>
#include "order_defs.hpp"
#include "order_type.hpp"
#include <ats/types.hpp>

namespace ats
{
	// Base order class to be inherited from
	class order
	{
	public:
		order(const ats::orderid_t& id, const std::string& symbol, long quantity,
				ats::order_side side, ats::order_time_in_force time_in_force)
			: id_(id), symbol_(symbol), quantity_(quantity),
			  side_(side), time_in_force_(time_in_force), transact_time(ats::timestamp_t::now())
		{ }

	public:
		virtual ~order() { }

		const ats::orderid_t& id() const { return id_; }
		const std::string& symbol() const { return symbol_; }
		long quantity() const { return quantity_; }
		ats::order_side side() const { return side_; }
		ats::order_time_in_force time_in_force() const { return time_in_force_; }
		ats::order_status status() const { return status_; }

		void set_quantity(long new_quantity) { quantity_ = new_quantity; }
		void set_status(const ats::order_status& new_status) { status_ = new_status; }

		bool is_pending() const
		{
			return !(status_ == ats::order_status::Filled ||
					status_ == ats::order_status::Canceled ||
					status_ == ats::order_status::Rejected);
		}

		// True if of the same type, false otherwise
		bool compare(const order& another_order) const
		{
			std::type_index index(typeid(*this));
			return index == typeid(another_order);
		}

		// True if of the same type, false otherwise
		bool compare(const std::type_index& another_order) const
		{
			std::type_index index(typeid(*this));
			return index == another_order;
		}

		long parent_id = -1;
		std::string exchange = "";
		ats::timestamp_t transact_time;
		long executed_quantity = 0;

	private:
		ats::orderid_t id_;
		std::string symbol_;
		long quantity_;
		ats::order_side side_;
		ats::order_time_in_force time_in_force_;
		ats::order_status status_;
	};
}

#endif