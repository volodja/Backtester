#ifndef REPORT_ENGINE_HPP
#define REPORT_ENGINE_HPP

#include <vector>
#include <ats/types.hpp>

namespace ats
{
	struct pnl_item
	{
		ats::timestamp_t time;
		double profit;
	};

	struct performance_item_basic
	{
		ats::timestamp_t time_open;
		ats::timestamp_t time_close;
		double profit;
	};

	struct performance_item : performance_item_basic
	{
		ats::price_t price;
		long profit_ticks;
		long quantity;
	};

	class report_engine
	{
		typedef std::vector<pnl_item> performance_container;
		typedef performance_container::iterator iterator;
		typedef performance_container::const_iterator const_iterator;
	public:
		void add_pnl_item(const ats::pnl_item& item)
		{
			performance_.push_back(item);
		}

		iterator begin() { return performance_.begin(); }
		const_iterator cbegin() const { return performance_.cbegin(); }
		iterator end() { return performance_.end(); }
		const_iterator cend() const { return performance_.cend(); }

	private:
		performance_container performance_;
//		std::vector<pnl> portfolio_performance_;
	};
}

#endif