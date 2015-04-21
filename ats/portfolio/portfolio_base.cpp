#include "portfolio_base.hpp"

namespace ats
{
	void portfolio_base::send_order(const order& order)
	{
		auto engine_it = execution_engines_.find(order.exchange);
		ats::execution_engine* engine;

		if (engine_it != execution_engines_.cend())
		{
			const ats::symbol_key* symbol = get_symbol_key(order.symbol());
			const std::string& venue_name = engine_it->second->name();
			//if (symbol != nullptr && order_books_[symbol->index].get(venue_name) != nullptr)
			if (symbol != nullptr && securities_[symbol->index]->order_book().get(venue_name) != nullptr)
			{
				//orders_ += order;
				orders_.insert(std::make_pair(order.id(), std::make_shared<ats::order>(order)));
				engine = engine_it->second;
				//					venue_it->second->send_order(order);
			}
			else
			{
				std::cout << "ERROR: Exchange '" << venue_name
						<< "' has no subscription for security '" << order.symbol() << '\n';
				return;
			}
		}
		else
		{
			std::cout << "ERROR: Cannot find exchange '" << order.exchange << "'\n";
			return;
		}

		//			venue->send_order(static_cast<decltype(order)>(order));

		std::type_index order_type(typeid(order));
		if (order_type == typeid(ats::market_order))
			engine->send_order(static_cast<const ats::market_order&>(order));
		else if (order_type == typeid(ats::limit_order))
			engine->send_order(static_cast<const ats::limit_order&>(order));
		else if (order_type == typeid(ats::stop_order))
			engine->send_order(static_cast<const ats::stop_order&>(order));
	}

	
	void portfolio_base::process_order_status_message(const order_status_message& msg)
	{
		on_time_update(msg.time);

//		auto it = orders_.get(msg.order_id);
		auto it = orders_.find(msg.order_id);
		if (it == orders_.end()) return;
		auto& ord_ptr = it->second;

		const ats::symbol_key* symbol = get_symbol_key(ord_ptr->symbol());
		if (symbol == nullptr)
		{
			std::cout << "ERROR: (Security, Exchange)=(" << ord_ptr->symbol() << "," << ord_ptr->exchange
				<< ") doesn't exist\n";
			return;
		}

		security_base_ptr& sec = securities_[symbol->index];
		ats::position& pos = positions_[symbol->index]; //sec->get_position();

		if (msg.order_status == ats::order_status::Filled)
		{
			// Modify the security position and remove the order
			const auto& m = static_cast<const ats::order_status_filled_message&>(msg);
			process_execution(*symbol, ord_ptr->side(), m.quantity, m.price, m.time);
			ord_ptr->executed_quantity = ord_ptr->quantity();
			ord_ptr->set_status(ats::order_status::Filled);
			//orders_.remove(it);
			orders_.erase(it);
		}
		else if (msg.order_status == ats::order_status::PartiallyFilled)
		{
			// Modify the security position and the order quantity
			const auto& m = static_cast<const ats::order_status_filled_message&>(msg);
			process_execution(*symbol, ord_ptr->side(), m.quantity, m.price, m.time);
			ord_ptr->set_quantity(ord_ptr->quantity() - m.quantity);
			ord_ptr->executed_quantity += m.quantity;
			ord_ptr->set_status(ats::order_status::PartiallyFilled);
		}
		else if (msg.order_status == ats::order_status::Canceled ||
			msg.order_status == ats::order_status::Rejected)
		{
			orders_.erase(it);
			if (msg.order_status == ats::order_status::Rejected)
				std::cout << "Order cancelled: symbol=" << sec->symbol().to_string() << '\n';
			if (msg.order_status == ats::order_status::Rejected)
				std::cout << "Rejected: symbol=" << sec->symbol().to_string()
				<< ", reason=" << static_cast<const ats::order_status_rejected_message&>(msg).rejection_reason << '\n';
		}

		// Pass the message to the related security
		sec->on_order_status_changed(msg);
	}
}
