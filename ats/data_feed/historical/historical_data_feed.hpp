#ifndef HISTORICAL_DATA_FEED_HPP
#define HISTORICAL_DATA_FEED_HPP

#include <memory>
#include <vector>
#include <map>
#include <utility>
#include "message_reader.hpp"
#include <ats/data_feed/data_feed.hpp>
#include <ats/message/message.hpp>
#include <ats/portfolio/portfolio_base.hpp>

namespace ats
{
	// Historical data feed that can feed messages from different sources
	class historical_data_feed : public ats::data_feed
	{
		using msg_reader_ptr = std::shared_ptr<ats::message_reader>;
	public:
		historical_data_feed(ats::portfolio_base* universe)
			: ats::data_feed(universe) { }

		virtual ~historical_data_feed() { }

		void add_message_reader(const msg_reader_ptr& reader) { readers_.push_back(reader); }
		void add_message_reader(msg_reader_ptr&& reader) { readers_.push_back(std::move(reader)); }
/*		void add_message_reader(const ats::message_reader& reader)
		{
			readers_.push_back(std::make_shared<ats::message_reader>(reader));
		}*/

		// Read messages from historical data sources in a synchronized manner
		bool read()
		{
			if (indices_.empty())
			{
				for (size_t i = 0; i < readers_.size(); ++i)
				{
					if (readers_[i]->read())
					{
						const ats::message& msg = readers_[i]->get_last_message();
						indices_.insert(std::make_pair(msg.time, i));
					}
				}
			}
			else
			{
				size_t i = indices_.begin()->second;
				indices_.erase(indices_.begin());
				if (readers_[i]->read())
				{
					const ats::message& msg = readers_[i]->get_last_message();
					indices_.insert(std::make_pair(msg.time, i));
				}
			}

			return !indices_.empty();
		}

		// Send message to the associated trading universe
		void send_message()
		{
			if (!indices_.empty())
			{
				size_t i = indices_.begin()->second;
				readers_[i]->send_message(this->universe_);
			}
		}

		// Read and send all messages in the historical data feed
		void run()
		{
			while (read())
			{
				send_message();
			}
		}

	private:
		std::vector<msg_reader_ptr> readers_;
		std::multimap<ats::timestamp_t, size_t> indices_;
	};
}

#endif