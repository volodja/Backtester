#ifndef PRICE_LEVELS_HPP
#define PRICE_LEVELS_HPP

#include <map>
#include <stdexcept>
#include <ats/message/level2_message.hpp>
#include <ats/order_book/detail/price_level.hpp>
#include <ats/types.hpp>

namespace ats {
namespace order_book_detail
{
	template<typename compare = std::less<ats::price_t>>
	class price_levels
	{
	public:
		typedef typename ats::order_book_detail::price_level price_level_type;
		typedef typename std::map<ats::price_t, price_level_type, compare> container_type;
		typedef typename container_type::iterator iterator;
		typedef typename container_type::const_iterator const_iterator;
		typedef typename container_type::reverse_iterator reverse_iterator;
		typedef typename container_type::const_reverse_iterator const_reverse_iterator;

		price_levels(size_t max_levels)
			: max_levels_(max_levels), levels_() { }

		void update(const ats::level2_message& msg)
		{
			// Trade messages are not used to update order book
			if (msg.entry_type == ats::entry_type::Trade) return;

			switch (msg.update_action)
			{
			case ats::update_action::New:
				insert_level(msg.price, price_level_type(msg.price, msg.quantity, msg.order_count));
				break;
			case ats::update_action::Change:
				change_level(msg.price, msg.quantity, msg.order_count);
				break;
			case ats::update_action::Delete:
				delete_level(msg.price);
				break;
			default:
				break;
			}
		}

		void update2(ats::level2_message& msg)
		{
			// Trade messages are not used to update order book
			if (msg.entry_type == ats::entry_type::Trade) return;

			if (msg.update_action == ats::update_action::New)
				insert_level(msg.price, price_level_type(msg.price, msg.quantity, msg.order_count));
			else if (msg.update_action == ats::update_action::Change)
			{
				auto it = levels_.find(msg.price);
				if (it != levels_.end())
				{
					long qty_delta = msg.quantity - it->second.quantity;
					long oct_delta = msg.order_count - it->second.order_count;

					it->second.quantity = msg.quantity;
					it->second.order_count = msg.order_count;

					msg.quantity = qty_delta;
					msg.order_count = oct_delta;
				}
				else
				{
					// this may be dangerous as we insert if we can't find:
					insert_level(msg.price, price_level_type(msg.price, msg.quantity, msg.order_count));
				}
			}
			else if (msg.update_action == ats::update_action::Delete)
			{
				delete_level(msg.price);
				msg.quantity = -msg.quantity;
			}
		}

	private:
		/// @brief change price level
		void change_level(ats::price_t price, long new_quantity, uint16_t new_order_count)
		{
			auto it = levels_.find(price);
			if (it != levels_.end())
			{
				it->second.quantity = new_quantity;
				it->second.order_count = new_order_count;
			}
			else
			{
				// this may be dangerous as we insert if we can't find:
				insert_level(price, price_level_type(price, new_quantity, new_order_count));
			}
		}

		/// @brief insert price level
		void insert_level(ats::price_t price, const price_level_type& level)
		{
			levels_.insert(std::make_pair(price, level));
			if (levels_.size() > max_levels_)
				levels_.erase(--levels_.cend());
		}

		/// @brief delete price level
		void delete_level(ats::price_t price)
		{
			auto it = levels_.find(price);
			if (it != levels_.cend())
				levels_.erase(it);
/*
			if (!levels_.empty())
			{
				// because the first and the last levels is more likely to be deleted,
				// consider separate cases to avoid search and improve performance
				if (price == levels_.cbegin()->first)
					levels_.erase(levels_.cbegin());
				else if (price == levels_.crbegin()->first)
					levels_.erase(--levels_.cend());
				else
				{
					auto it = levels_.find(price);
					if (it != levels_.end())
						levels_.erase(it);
				}
			}
			else
			{
				//const char* text = "Cannot delete from empty price ladder";
				//throw std::out_of_range(text);
			}*/
		}

	public:
		// iterators
		iterator begin() { return levels_.begin(); }
		const_iterator cbegin() const { return levels_.cbegin(); }
		iterator end() { return levels_.end(); }
		const_iterator cend() const { return levels_.cend(); }
		reverse_iterator rbegin() { return levels_.rbegin(); }
		const_reverse_iterator crbegin() const { return levels_.crbegin(); }
		reverse_iterator rend() { return levels_.rend(); }
		const_reverse_iterator crend() { return levels_.crend(); }

		price_level_type& operator[](ats::price_t price) { return levels_[price]; }
		const price_level_type& operator[](ats::price_t price) const { return levels_[price]; }

		price_level_type& at(ats::price_t price) { return levels_.at(price); }
		const price_level_type& at(ats::price_t price) const { return levels_.at(price); }

		bool empty() const { return levels_.empty(); }

		void clear() { levels_.clear(); }

		iterator find(ats::price_t price)
		{
			return levels_.find(price);
		}

		const_iterator find(ats::price_t price) const
		{
			return levels_.find(price);
		}

		size_t size() const { return levels_.size(); }
		size_t displayed_depth() const { return max_levels_; }

		iterator get_level(size_t index)
		{
			if (index > levels_.size())
				return levels_.end();
			else
			{
				iterator it = levels_.begin();
				for (size_t i = 1; i < index; ++i) ++it;
				return it;
			}
		}

		const_iterator get_level(size_t index) const
		{
			if (index > levels_.size())
				return levels_.cend();
			else
			{
				iterator it = levels_.cbegin();
				for (size_t i = 1; i < index; ++i) ++it;
				return it;
			}
		}

		container_type& levels() { return levels_; }
	
	private:
		size_t max_levels_;
		container_type levels_;
	};
}
}

#endif