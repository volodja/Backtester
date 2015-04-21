#ifndef DATA_FEED_HPP
#define DATA_FEED_HPP

#include <ats/event_handler/multievent_handler.hpp>
#include <ats/portfolio/portfolio_base.hpp>

namespace ats
{
	// Common interface of all data feeds
	class data_feed : public ats::multievent_handler
	{
	public:
		data_feed(ats::portfolio_base* universe)
			: universe_(universe) { }

		void set_trading_universe(ats::portfolio_base* universe) { universe_ = universe; }

	protected:
		// Send instrument specific message to the associated trading universe
/*		template<typename MessageT>
		void send_instrument_message(const MessageT& msg)
		{
			universe_->handle_instrument_message(msg);
		}

		// Send a portfolio-level message to the associated trading universe
		template<typename MessageT>
		void send_global_message(const MessageT& msg)
		{
			universe_->invoke(msg);
		}*/

	protected:
		ats::portfolio_base* universe_;  // data feed knows where to send messages
	};
}

#endif