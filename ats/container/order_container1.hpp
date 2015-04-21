#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <ats/order/order.hpp>
#include <ats/types.hpp>

namespace ats
{
	class order_container1
	{
		typedef std::shared_ptr<ats::order> order_ptr;
		typedef std::unordered_multimap<std::type_index, order_ptr> order_container_type;
		typedef order_container_type::iterator iterator;
		typedef order_container_type::const_iterator const_iterator;
		typedef std::unordered_map<ats::orderid_t, iterator> iterator_container_type;
	public:
/*		order_container& operator+=(const order_ptr& order)
		{
			std::type_index index(typeid(*order.get()));
			auto it = orders_.insert(order_container_type::value_type(index, order));
			iterators_.insert(std::make_pair(order->id(), it));
			return *this;
		}*/

		// Add an order
		order_container1& operator+=(const ats::order& order)
		{
			std::type_index index(typeid(order));
			auto it = orders_.insert(order_container_type::value_type(index, std::make_shared<ats::order>(order)));
			iterators_.insert(std::make_pair(order.id(), it));
			return *this;
		}

		// Gets an order by its id
		iterator get(const ats::orderid_t& order_id)
		{
			auto find = iterators_.find(order_id);
			return find != iterators_.end() ? find->second : orders_.end();
		}

		// Gets all orders of a given type (say, all limit orders)
		std::pair<iterator, iterator> get(const std::type_index& index)
		{
			return orders_.equal_range(index);
		}

		void remove(const ats::orderid_t& order_id)
		{
			auto find = iterators_.find(order_id);
			if (find != iterators_.end())
			{
				auto it = find->second;
				iterators_.erase(find);
				orders_.erase(it);
			}
		}

		void remove(iterator it)
		{
			auto find = iterators_.find(it->second->id());
			if (find != iterators_.end())
			{
				iterators_.erase(find);
				orders_.erase(it);
			}
		}

	public:
		bool empty() const { return orders_.empty(); }
	public:
		iterator begin() { return orders_.begin(); }
		const_iterator cbegin() const { return orders_.cbegin(); }
		iterator end() { return orders_.end(); }
		const_iterator cend() const { return orders_.cend(); }
	private:
		order_container_type orders_;
		iterator_container_type iterators_;
	};
}
