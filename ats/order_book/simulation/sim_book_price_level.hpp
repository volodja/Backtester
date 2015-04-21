#ifndef SIM_BOOK_PRICE_LEVEL_HPP
#define SIM_BOOK_PRICE_LEVEL_HPP

#include <list>
#include <unordered_map>
#include <string>
#include <sstream>
#include <ats/order/limit_order.hpp>
#include <ats/message/level2_message.hpp>
#include <ats/handler_types.hpp>

namespace ats
{
	namespace sim
	{
		class price_level
		{
		public:
			typedef std::list<ats::limit_order> orderqueue_type;
			typedef orderqueue_type::iterator iterator;
			typedef std::unordered_map<ats::orderid_t, iterator> order_container;

			price_level(ats::price_t price) : price_(price) { }

			iterator begin() { return queue_.begin(); }
			iterator end() { return queue_.end(); }

			ats::price_t price() const { return price_; }
			bool is_defined() const { return is_defined_; }

			iterator add_order(const ats::limit_order& order);
			iterator insert_order(const ats::limit_order& order);
			bool erase_order(iterator position);

			// Remove orders that are not mine
			void clean();
			void clean(long quantity);
			void execute_all_orders(const ats::timestamp_t& time, ats::order_status_handler& listener,
				order_container& orders);
			void execute_orders(long quantity, const ats::timestamp_t& time, ats::order_status_handler& listener,
				order_container& orders);

			std::string to_string() const;

			void process_change_msg(const ats::level2_message& msg, ats::order_status_handler& listener,
				order_container& orders);

		public:
			long quantity = 0;
			long sim_quantity = 0;
			long traded_quantity = 0;
		private:
			ats::price_t price_;
			orderqueue_type queue_;
			bool is_defined_ = false;
		};

		inline price_level::iterator price_level::add_order(const ats::limit_order& order)
		{
			queue_.push_back(order);
			if (order.id() != 0)
				sim_quantity += order.quantity();
			quantity += order.quantity();

			return std::prev(queue_.end());
		}

		inline price_level::iterator price_level::insert_order(const ats::limit_order& order)
		{
			queue_.push_front(order);
			if (order.id() != 0)
				sim_quantity += order.quantity();
			quantity += order.quantity();
			is_defined_ = true;
			return queue_.begin();
		}

		inline bool price_level::erase_order(iterator position)
		{
			if (position != queue_.end())
			{
				quantity -= position->quantity();
				if (position->id() != 0)
					sim_quantity -= position->quantity();
				queue_.erase(position);
				return true;
			}
			else
				return false;
		}

		inline void price_level::clean()
		{
			for (auto it = queue_.begin(); it != queue_.end();)
			{
				if (it->id() == 0)
					queue_.erase(it++);
				else
					++it;
			}

			quantity = 0;
		}

		inline void price_level::clean(long qty)
		{
			long remained_qty = qty;
			for (auto it = queue_.rbegin(); it != queue_.rend() && remained_qty > 0;)
			{
				if (it->id() != 0)
					++it;
				else if (it->quantity() > remained_qty)
				{
					it->set_quantity(it->quantity() - remained_qty);
					quantity -= remained_qty;
					break;
				}
				else
				{
					quantity -= it->quantity();
					remained_qty -= it->quantity();
					auto rm = --it.base();
					++it;
					queue_.erase(rm);
				}
			}
		}

		inline void price_level::execute_all_orders(const ats::timestamp_t& time, ats::order_status_handler& listener,
			order_container& orders)
		{
			for (auto it = queue_.begin(); it != queue_.end();)
			{
				if (it->id() != 0)
				{
					auto ord_it = orders.find(it->id());
					if (ord_it != orders.end())
						orders.erase(ord_it);

					ats::order_status_filled_message msg(it->id(), time, price_, it->quantity());
					listener(msg);
				}
				queue_.erase(it++);
			}

			quantity = 0;
			sim_quantity = 0;
		}

		inline void price_level::execute_orders(long quantity, const ats::timestamp_t& time,
			ats::order_status_handler& listener, order_container& orders)
		{
			long unexecuted_qty = quantity;
			for (auto it = queue_.begin(); it != queue_.end() && unexecuted_qty > 0;)
			{
				if (it->quantity() > unexecuted_qty)
				{
					it->set_quantity(it->quantity() - unexecuted_qty);
					quantity -= unexecuted_qty;
					if (it->id() != 0)
					{
						sim_quantity -= unexecuted_qty;
						ats::order_status_partially_filled_message msg(it->id(), time, it->price(), unexecuted_qty);
						listener(msg);
					}
					break;
				}
				else
				{
					quantity -= it->quantity();
					if (it->id() != 0)
					{
						auto ord_it = orders.find(it->id());
						if (ord_it != orders.end())
							orders.erase(ord_it);

						sim_quantity -= it->quantity();
						ats::order_status_filled_message msg(it->id(), time, it->price(), it->quantity());
						listener(msg);
					}
					unexecuted_qty -= it->quantity();
					queue_.erase(it++);
				}
			}
		}

		inline std::string price_level::to_string() const
		{
			std::stringstream ss;
			ss << "Price level " << price_ << "(qty=" << quantity << ",sim_qty=" << sim_quantity
				<< ",trd_qty=" << traded_quantity << ";" << is_defined_ << "): ";
			for (const auto& x : queue_)
				ss << "(id=" << x.id() << ",qty=" << x.quantity() << "), ";

			return ss.str();
		}

		inline void price_level::process_change_msg(const ats::level2_message& msg, ats::order_status_handler& listener,
			order_container& orders)
		{
			if (msg.quantity > 0)
			{
				ats::order_side side = msg.entry_type == ats::entry_type::Bid ?
					ats::order_side::Buy : ats::order_side::SellShort;
				ats::limit_order order(0, msg.symbol, msg.quantity, side, ats::order_time_in_force::GTC, msg.price);

				if (!is_defined_)
					insert_order(order);
				else
					add_order(order);
			}
			else if (traded_quantity == 0)
				clean(-msg.quantity);
			else
				execute_orders(-msg.quantity, msg.time, listener, orders);

			traded_quantity = 0;
		}
	}
}

#endif