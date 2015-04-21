#ifndef DOUBLE_KEY_LOOKUP_HPP
#define DOUBLE_KEY_LOOKUP_HPP

#include <unordered_map>

namespace ats
{
	template<typename UniqueKey, typename Key, typename Value,
			class UniqueHash = std::hash<UniqueKey>, class Hash = std::hash<Key>>
	class double_key_lookup
	{
	public:
		typedef typename std::unordered_multimap<Key, Value, Hash> container_type;
		typedef typename container_type::iterator iterator;
		typedef typename container_type::const_iterator const_iterator;
		typedef typename std::unordered_map<UniqueKey, iterator, UniqueHash> iterator_container_type;
	public:
		iterator begin() { return values_.begin(); }
		iterator end() { return values_.end(); }
		const_iterator cbegin() const { return values_.cbegin(); }
		const_iterator cend() const { return values_.cend(); }
		size_t size() const { return values_.size(); }
		bool empty() const { return values_.empty(); }
	public:
		iterator find(const UniqueKey& unique_key)
		{
			auto find = iterators_.find(unique_key);
			return find != iterators_.end() ? find->second : values_.end();
		}

		std::pair<iterator, iterator> equal_range(const Key& key)
		{
			return values_.equal_range(key);
		}

		iterator insert(const UniqueKey& unique_key, const Key& key, const Value& value)
		{
			auto it = values_.insert(typename container_type::value_type(key, value));
			iterators_.insert(typename iterator_container_type::value_type(unique_key, it));
			return it;
		}

		iterator erase(const UniqueKey& unique_key)
		{
			auto find = iterators_.find(unique_key);
			if (find != iterators_.end())
			{
				auto it = values_.erase(find->second);
				iterators_.erase(find);
				return it;
			}
			else
				return values_.end();
		}

		void clear()
		{
			iterators_.clear();
			values_.clear();
		}

	private:
		container_type values_;
		iterator_container_type iterators_;
	};
}

#endif