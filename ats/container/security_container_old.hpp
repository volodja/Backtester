#pragma once

#include <ats/security/security_base.hpp>
#include <unordered_map>
#include <string>
#include <utility>

namespace ats
{
	// Container for securities. Allows for fast retrieval of securities based
	// on the unique (symbol, exchange) key and (non-unique) symbol key
	class security_container
	{
		// for fast lookup based on the (non-unique) symbol key
		typedef std::unordered_multimap<std::string, ats::security_base> container_type;
	public:
		typedef container_type::iterator iterator;
		typedef container_type::const_iterator const_iterator;
	private:
		typedef std::pair<std::string, std::string> key_type;
		// this hash will be needed for having a pair-based hashmap:
		struct key_hash
		{
			size_t operator()(const key_type& key) const
			{
				return std::hash<std::string>()(key.first) ^ std::hash<std::string>()(key.second);
			}
		};

		// for fast lookup based on the (unique) (symbol, exchange) key
		typedef std::unordered_map<key_type, iterator, key_hash> iterator_container;

	public:
		security_container& operator+=(const ats::security_base& sec)
		{
			auto it = securities_.insert(std::make_pair(sec.symbol(), sec));
			auto key = std::make_pair(sec.symbol(), sec.exchange());
			iterators_.insert(std::make_pair(key, it));
			return *this;
		}

		security_container& operator+=(ats::security_base&& sec)
		{
			auto key = std::make_pair(sec.symbol(), sec.exchange());
			auto it = securities_.insert(std::make_pair(key.first, std::move(sec)));
			iterators_.insert(std::make_pair(key, it));
			return *this;
		}

		// Gets a security with a given name and exchange
		iterator get(const std::string& symbol, const std::string& exchange)
		{
			auto find = iterators_.find(std::make_pair(symbol, exchange));
			return find != iterators_.end() ? find->second : securities_.end();
		}

		// Gets all securities with a given name (trading on different exchanges)
		std::pair<iterator, iterator> get(const std::string& symbol)
		{
			return securities_.equal_range(symbol);
		}

	public:
		iterator begin() { return securities_.begin(); }
		const_iterator cbegin() const { return securities_.cbegin(); }
		iterator end() { return securities_.end(); }
		const_iterator cend() const { return securities_.cend(); }

	private:
		container_type securities_;
		iterator_container iterators_;
	};
}
