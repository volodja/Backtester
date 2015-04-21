#ifndef LEVEL2_MESSAGE_READER_HPP
#define LEVEL2_MESSAGE_READER_HPP

#include <ats/data_feed/historical/exchange_message_reader_base.hpp>
#include <ats/message/level2_message.hpp>
#include <ats/io/csv_reader.hpp>

#include <ats/io/tokenize.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include "boost/date_time/gregorian/gregorian.hpp"

namespace ats
{
	class l2_message_reader : public ats::exchange_message_reader_base<ats::level2_message_packet>
	{
	public:
		l2_message_reader(const std::string& filename, const std::string& symbol, const std::string& exchange)
			: ats::exchange_message_reader_base<ats::level2_message_packet>(),
			stream_(filename)
		{
				message_.symbol = symbol;
				message_.exchange = exchange;
			}

		virtual bool read() override
		{
			message_.messages.clear();
			ats::level2_message msg;

			bool is_message = false;
			while (std::getline(stream_, line_))
			{
				// Windows uses CRLF new lines as opposed to Unix's LF:
/*				if (line_.length() > 0 && *line_.crbegin() == '\r')
					line_.erase(line_.length() - 1);*/

				if (ats::tokenize(line_, fields_, ',') != fields_.size())
					break;

				if (!is_message)
				{
					message_.time.parse(fields_[0], "%Y%m%d %H%M%S%F");
					msg.time = message_.time;
					msg.symbol = message_.symbol;
					msg.exchange = message_.exchange;
					is_message = true;
				}

				if (fields_[1] == "N")
					msg.update_action = ats::update_action::New;
				else if (fields_[1] == "C")
					msg.update_action = ats::update_action::Change;
				else if (fields_[1] == "D")
					msg.update_action = ats::update_action::Delete;

				if (fields_[2] == "B")
					msg.entry_type = ats::entry_type::Bid;
				else if (fields_[2] == "A")
					msg.entry_type = ats::entry_type::Ask;
				else if (fields_[2] == "T")
					msg.entry_type = ats::entry_type::Trade;

				msg.level = std::stol(fields_[3]); //std::strtol(fields_[3].c_str(), nullptr, 10);
				msg.price = std::stol(fields_[4]);//std::strtol(fields_[4].c_str(), nullptr, 10);
				msg.quantity = std::stol(fields_[5]);//std::strtol(fields_[5].c_str(), nullptr, 10);
				msg.order_count = std::stol(fields_[6]);//std::strtol(fields_[6].c_str(), nullptr, 10);

				message_.messages.push_back(msg);

				//std::cout << msg.time << ',' << msg.price << ',' << msg.quantity << ',' << msg.order_count << '\n';
			}

			return is_message;
		}

	private:
		std::ifstream stream_;                // file containing messages
		std::array<std::string, 7U> fields_;  // fields of a line in our CSV file
		std::string line_;
	};
}

#endif