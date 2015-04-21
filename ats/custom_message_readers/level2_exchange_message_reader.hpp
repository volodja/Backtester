#ifndef LEVEL2_EXCHANGE_MESSAGE_READER_HPP
#define LEVEL2_EXCHANGE_MESSAGE_READER_HPP

#include <string>
#include <array>
#include <fstream>
#include <ats/data_feed/historical/exchange_message_reader_base.hpp>
#include <ats/message/level2_message.hpp>
#include <ats/io/csv_reader.hpp>

namespace ats
{
	class level2_exchange_message_reader : public ats::exchange_message_reader_base<ats::level2_message>
	{
	public:
		level2_exchange_message_reader(const std::string& filename)
		: ats::exchange_message_reader_base<ats::level2_message>(), stream_(filename), reader_(&stream_) { }

		virtual bool read() override
		{
			if (reader_.read(fields_))
			{
				message_.symbol = fields_[0];
				message_.exchange = fields_[1];
				message_.seq_number = std::strtol(fields_[2].c_str(), nullptr, 10);
				message_.time.parse(fields_[3], "%Y%m%d %H%M%S%F"); //= 0; // Time!!!!!!!!!!!!!!!
				message_.update_action = static_cast<ats::update_action>(std::strtol(fields_[4].c_str(), nullptr, 10));
				message_.entry_type = static_cast<ats::entry_type>(std::strtol(fields_[5].c_str(), nullptr, 10));
				message_.price = std::strtol(fields_[6].c_str(), nullptr, 10);
				message_.quantity = std::strtol(fields_[7].c_str(), nullptr, 10);
				message_.order_count = std::strtol(fields_[8].c_str(), nullptr, 10);
				message_.level = std::strtol(fields_[9].c_str(), nullptr, 10);
				return true;
			}
			else
				return false;
		}

	private:
		std::ifstream stream_;                // file containing messages
		ats::csv_reader<10> reader_;          // reads fields of CSV files line by line
		std::array<std::string, 10> fields_;  // fields of a line in our CSV file
	};
}

#endif