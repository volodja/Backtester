#ifndef SIM_BOOK_PRICE_LEVELS_HPP
#define SIM_BOOK_PRICE_LEVELS_HPP

#include <map>
#include "sim_book_price_level.hpp"

namespace ats
{
	namespace sim
	{
		template<typename comp = std::less<ats::price_t>>
		class price_levels
		{
			typedef std::unordered_map<ats::orderid_t, price_level::iterator> order_container;
		public:
			typedef typename std::map<ats::price_t, ats::sim::price_level, comp> container_type;
			typedef typename container_type::iterator iterator;

			void add_order_status_listener(const ats::order_status_handler& listener)
			{
				order_status_listener_ = listener;
			}

			price_level::iterator add_order(const ats::limit_order& order);
			price_level::iterator insert_order(const ats::limit_order& order);
			void cancel_order(price_level::iterator position, const ats::timestamp_t& time);
			bool erase_order(price_level::iterator position);
			void erase_level(ats::price_t price);

			const price_level* top_level() const
			{
				return levels_.empty() ? nullptr : &levels_.cbegin()->second;
			}

			price_level* top_level()
			{
				return levels_.empty() ? nullptr : &levels_.begin()->second;
			}

			const price_level* get_level(ats::price_t price) const
			{
				auto it = levels_.find(price);
				return it == levels_.cend() ? nullptr : &it->second;
			}

			iterator begin() { return levels_.begin(); }
			iterator end() { return levels_.end(); }
			bool empty() const { return levels_.empty(); }

			void clean(ats::price_t price);
			void execute_all_orders(ats::price_t price, const ats::timestamp_t& time, order_container& orders);
			void execute_orders(ats::price_t price, long quantity, const ats::timestamp_t& time, order_container& orders);

			void process_change_msg(const ats::level2_message& msg, order_container& orders);
			void process_delete_msg(const ats::level2_message& msg, order_container& orders);
			void process_insert_msg(const ats::level2_message& msg);

		private:
			container_type levels_;
			ats::order_status_handler order_status_listener_;
		};

		template<typename comp>
		price_level::iterator price_levels<comp>::add_order(const ats::limit_order& order)
		{
			auto it = levels_.find(order.price());
			if (it != levels_.end())
				return it->second.add_order(order);
			else
			{
				price_level l(order.price());
				l.add_order(order);
				auto ins = levels_.insert(std::make_pair(order.price(), l)).first;
				return std::prev(ins->second.end());
			}
		}

		template<typename comp>
		price_level::iterator price_levels<comp>::insert_order(const ats::limit_order& order)
		{
			auto it = levels_.find(order.price());
			if (it != levels_.end())
				return it->second.insert_order(order);
			else
			{
				price_level l(order.price());
				l.insert_order(order);
				auto inserted = levels_.insert(std::make_pair(order.price(), l)).first;
				return inserted->second.begin();
			}
		}

		template<typename comp>
		void price_levels<comp>::cancel_order(price_level::iterator position, const ats::timestamp_t& time)
		{
			auto it = levels_.find(position->price());
			if (it != levels_.end())
			{
				it->second.erase_order(position);
				if (it->second.sim_quantity == 0)
					levels_.erase(it);
			}
		}

		template<typename comp>
		bool price_levels<comp>::erase_order(price_level::iterator position)
		{
			auto it = levels_.find(position->price());
			if (it != levels_.end())
				return it->second.erase_order(position);
			else
				return false;
		}

		template<typename comp>
		void price_levels<comp>::erase_level(ats::price_t price)
		{
			auto it = levels_.find(price);
			if (it != levels_.end())
				levels_.erase(it);
		}

		template<typename comp>
		void price_levels<comp>::clean(ats::price_t price)
		{
			auto it = levels_.find(price);
			if (it != levels_.end())
				it->second.clean();
		}

		template<typename comp>
		void price_levels<comp>::execute_all_orders(ats::price_t price, const ats::timestamp_t& time, order_container& orders)
		{
			auto it = levels_.find(price);
			if (it != levels_.end())
			{
				it->second.execute_all_orders(time, order_status_listener_, orders);
				levels_.erase(it);
			}
		}

		template<typename comp>
		void price_levels<comp>::execute_orders(ats::price_t price, long quantity, const ats::timestamp_t& time, order_container& orders)
		{
			auto it = levels_.find(price);
			if (it != levels_.end())
			{
				it->second.execute_orders(quantity, time, order_status_listener_, orders);
				if (it->second.sim_quantity == 0)
					levels_.erase(it);
			}
		}

		template<typename comp>
		void price_levels<comp>::process_change_msg(const ats::level2_message& msg, order_container& orders)
		{
			auto it = levels_.find(msg.price);
			if (it != levels_.cend())
			{
				it->second.process_change_msg(msg, order_status_listener_, orders);

				if (it->second.sim_quantity == 0)
					levels_.erase(it);
			}
		}

		template<typename comp>
		void price_levels<comp>::process_delete_msg(const ats::level2_message& msg, order_container& orders)
		{
			auto it = levels_.find(msg.price);
			if (it != levels_.cend())
			{
				if (it->second.traded_quantity == msg.quantity)
					execute_all_orders(msg.price, msg.time, orders);
				else
				{
					it->second.clean();
					it->second.traded_quantity = 0;
				}
			}
		}

		template<typename comp>
		void price_levels<comp>::process_insert_msg(const ats::level2_message& msg)
		{
			auto it = levels_.find(msg.price);
			if (it != levels_.cend())
			{
				ats::sim::price_level& level = it->second;
				ats::order_side side = msg.entry_type == ats::entry_type::Bid ?
					ats::order_side::Buy : ats::order_side::SellShort;
				if (!level.is_defined())
				{
					ats::limit_order order(0, msg.symbol, msg.quantity, side,
						ats::order_time_in_force::GTC, msg.price);
					level.insert_order(order);
				}
				else
				{
					long delta = level.quantity - level.sim_quantity;
					if (msg.quantity > delta)
					{
						ats::limit_order order(0, msg.symbol, msg.quantity - delta, side,
							ats::order_time_in_force::GTC, msg.price);
						level.add_order(order);
					}
					else if (msg.quantity < delta)
						level.clean(delta - msg.quantity);
				}
			}
		}
	}
}

#endif