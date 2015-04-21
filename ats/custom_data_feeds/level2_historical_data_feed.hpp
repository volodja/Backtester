#ifndef LEVEL2_HISTORICAL_DATA_FEED_HPP
#define LEVEL2_HISTORICAL_DATA_FEED_HPP

#include <ats/data_feed/historical/historical_data_feed.hpp>
#include <ats/portfolio/portfolio_base.hpp>

namespace ats
{
	class level2_historical_data_feed : public ats::historical_data_feed
	{
	public:
		level2_historical_data_feed(ats::portfolio_base* universe)
			: ats::historical_data_feed(universe)
		{
			add_event_handler(&level2_historical_data_feed::handler, this);
		}

	private:
		void handler(const ats::level2_message& message)
		{
	//		send_instrument_message(message);
		}
	};
}

#endif