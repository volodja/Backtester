#ifndef ORDER_CONTAINER_HPP
#define ORDER_CONTAINER_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <ats/order/order.hpp>
#include <ats/types.hpp>

namespace ats
{
	class order_container
	{
		typedef std::pair<std::string, std::string> key_type;
		typedef std::shared_ptr<ats::order> order_ptr;

		// this hash will be needed for having a pair-based hashmap:
		struct key_hash
		{
			size_t operator()(const key_type& key) const
			{
				return std::hash<std::string>()(key.first) ^ std::hash<std::string>()(key.second);
			}
		};

		typedef std::unordered_multimap<key_type, order_ptr, key_hash> order_container_type;
		typedef order_container_type::iterator iterator;
		typedef order_container_type::const_iterator const_iterator;
		typedef std::unordered_map<ats::orderid_t, iterator> iterator_container_type;
	public:

		// Add an order
		order_container& operator+=(const ats::order& order)
		{
			key_type key(order.symbol(), order.exchange);
			auto it = orders_.insert(std::make_pair(key, std::make_shared<ats::order>(order)));
			iterators_.insert(std::make_pair(order.id(), it));
			return *this;
		}

		// Add an order
		order_container& operator+=(const order_ptr& order)
		{
			key_type key(order->symbol(), order->exchange);
			auto it = orders_.insert(std::make_pair(key, order));
			iterators_.insert(std::make_pair(order->id(), it));
			return *this;
		}

		// Add an order
		order_container& operator+=(order_ptr&& order)
		{
			key_type key(order->symbol(), order->exchange);
			auto it = orders_.insert(std::make_pair(key, std::move(order)));
			iterators_.insert(std::make_pair(it->second->id(), it));
			return *this;
		}

		// Get an order by its id
		iterator get(const ats::orderid_t& order_id)
		{
			auto find = iterators_.find(order_id);
			return find != iterators_.end() ? find->second : orders_.end();
		}

		// Get an order by its id
		const_iterator get(const ats::orderid_t& order_id) const
		{
			auto find = iterators_.find(order_id);
			return find != iterators_.cend() ? find->second : orders_.cend();
		}

		// Get all orders for a given security trading on a given exchange
		std::pair<iterator, iterator> get(const std::string& symbol, const std::string& exchange)
		{
			return orders_.equal_range(std::make_pair(symbol, exchange));
		}

		// Get all orders for a given security trading on a given exchange
		std::pair<iterator, iterator> get(const key_type& key)
		{
			return orders_.equal_range(key);
		}

		// Remove an order
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

		// Add an order
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

#endif