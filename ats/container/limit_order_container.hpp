#ifndef LIMIT_ORDER_CONTAINER_HPP
#define LIMIT_ORDER_CONTAINER_HPP

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <ats/order/limit_order.hpp>
#include <ats/types.hpp>

namespace ats
{
	class limit_order_container
	{
		typedef std::shared_ptr<ats::limit_order> order_ptr;
		typedef std::unordered_multimap<ats::price_t, order_ptr> order_container_type;
		typedef order_container_type::iterator iterator;
		typedef order_container_type::const_iterator const_iterator;

/*		struct iterator_hash
		{
			size_t operator()(const_iterator it) const
			{
				return std::hash<ats::orderid_t>()(it->first);
			}
		};*/

		typedef std::unordered_map<ats::orderid_t, iterator> iterator_container_type;

	public:
		limit_order_container& operator+=(const order_ptr& order)
		{
			auto it = orders_.insert(std::make_pair(order->price(), order));
			iterators_.insert(std::make_pair(order->id(), it));
			return *this;
		}

		limit_order_container& operator+=(order_ptr&& order)
		{
			auto it = orders_.insert(std::make_pair(order->price(), std::move(order)));
			iterators_.insert(std::make_pair(it->second->id(), it));
			return *this;
		}

		limit_order_container& operator+=(const ats::limit_order& order)
		{
			return this->operator +=(std::make_shared<ats::limit_order>(order));
		}

/*		void add_order(order_ptr order)
		{
			auto it = orders_.insert(std::make_pair(order->price(), order));
			iterators_.insert(std::make_pair(order->id(), it));
		} */

		void delete_order(iterator it)
		{
			auto find = iterators_.find(it->second->id());
			iterators_.erase(find);
			orders_.erase(it);
		}

		void delete_order(const ats::orderid_t& id)
		{
			auto find = iterators_.find(id);
			if (find != iterators_.end())
			{
				auto it = find->second;
				iterators_.erase(find);
				orders_.erase(it);
			}
		}

		iterator get_order(const ats::orderid_t& id)
		{
			auto find = iterators_.find(id);
			return find != iterators_.end() ? find->second : orders_.end();
		}

		order_ptr get_order_ptr(const ats::orderid_t& id)
		{
			auto find = iterators_.find(id);
			return find != iterators_.end() ? find->second->second : nullptr;
		}


		std::pair<iterator, iterator> get_orders(ats::price_t price)
		{
			return orders_.equal_range(price);
		}

		void print_all_orders() const
		{
			for (auto it = orders_.cbegin(); it != orders_.cend(); ++it)
			{
				std::cout << "Order @" << it->second->price() << " id=" << it->second->id() << '\n';
			}
		}

		void print_all_orders(ats::price_t price)// const
		{
			auto range = get_orders(price);
			for (auto it = range.first; it != range.second; ++it)
			{
				std::cout << "Order @" << it->second->price() << " id=" << it->second->id() << '\n';
			}
		}

	public:
		iterator begin() { return orders_.begin(); }
		const_iterator cbegin() const { return orders_.cbegin(); }
		iterator end() { return orders_.end(); }
		const_iterator cend() const { return orders_.cend(); }
	private:
		order_container_type orders_;
		iterator_container_type iterators_;
	};
















	class limit_order_container1
	{
		typedef std::unordered_map<ats::orderid_t, ats::limit_order> order_container_type;
		typedef order_container_type::iterator iterator;
		typedef order_container_type::const_iterator const_iterator;

		struct iterator_hash
		{
			size_t operator()(const_iterator it) const
			{
				return std::hash<ats::orderid_t>()(it->first);
			}
		};

		typedef std::unordered_set<const_iterator, iterator_hash> iterator_container_type;

	public:
		void add_order(const ats::limit_order& order)
		{
			auto it = orders_.insert(std::make_pair(order.id(), order)).first;
			auto find = iterators_.find(order.price());
			if (find != iterators_.end())
				find->second.insert(it);
			else
				iterators_.insert(std::make_pair(order.price(), iterator_container_type{ it }));
		}

		/*		void cancel_order(const ats::orderid_t& id)
			{
				auto it = orders_.find(id);
				if (it != orders_.end())
				{
					auto range = iterators_.equal_range(it->second.price());
					for (auto r_it = range.first; r_it != range.second; ++r_it)
					{
						auto p = r_it->second;
						if (p->second.id() == id)
						{
							iterators_.erase(r_it);
							orders_.erase(it);
							break;
						}
					}
				}
			}*/

		void delete_order(iterator it)
		{
			auto& level = iterators_.find(it->second.id())->second;
			level.erase(it);
			orders_.erase(it);
		}

		void delete_order(const ats::orderid_t& id)
		{
			auto it = orders_.find(id);
			if (it != orders_.end())
			{
				auto& level = iterators_.find(it->second.id())->second;
				level.erase(level.find(it));
				orders_.erase(it);
				//delete_order(it);
			}
		}

		iterator get_order(const ats::orderid_t& id)
		{
			return orders_.find(id);
			//			return find != orders_.end() ? &find->second : nullptr;
		}

		iterator_container_type* get_orders(ats::price_t price)
		{
			auto find = iterators_.find(price);
			return find != iterators_.end() ? &find->second : nullptr;
		}

		void print_all_orders() const
		{
			for (auto it = orders_.cbegin(); it != orders_.cend(); ++it)
			{
				std::cout << "Order @" << it->second.price() << " id=" << it->second.id() << '\n';
			}
		}

		void print_all_orders(ats::price_t price)// const
		{
			auto find = iterators_.find(price);
			if (find == iterators_.end()) return;

			for (auto it = find->second.cbegin(); it != find->second.cend(); ++it)
			{
				//std::cout << "Order @" << (*it)->second->price() << " id=" << (*it)->second->id() << '\n';
			}
		}

	public:
		iterator begin() { return orders_.begin(); }
		iterator end() { return orders_.end(); }
	private:
		order_container_type orders_;
		//		std::unordered_multimap<ats::price_t, iterator> iterators_;
		std::unordered_map<ats::price_t, iterator_container_type> iterators_;
	};
}

#endif