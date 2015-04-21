#ifndef FIX_PARSER_HPP
#define FIX_PARSER_HPP

#include <unordered_set>
#include <string>
#include <ats/message/level2_message.hpp>
#include <ats/date_time/date_time.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <quickfix/DataDictionary.h>
#include <quickfix/fix50sp2/MarketDataIncrementalRefresh.h>

namespace ats
{
	static bool parse_fix_msg(const char* fix_msg, ats::level2_message_packet& result,
			const FIX::DataDictionary& dictionary, const std::unordered_set<std::string>& symbols)
	{
		result.messages.clear();

		FIX50SP2::MarketDataIncrementalRefresh msg(FIX::Message(fix_msg, dictionary, false));

		std::string value; // FIX field values will be recorder here

		size_t groups_count = 0;
		long seq_number;

		// get data from the header
		try
		{
			groups_count = std::strtol(msg.getField(268).c_str(), nullptr, 10);
			FIX::Header header = msg.getHeader();
			value = header.getField(52);
			long millis = std::strtol(&value[value.size() - 3], nullptr, 10);
			result.time.parse(value, "%Y%m%d%H%M%S");
			result.time += boost::posix_time::milliseconds(millis);
			seq_number = std::strtol(&header.getField(34)[0], nullptr, 10);
			result.exchange = header.getField(49);
		}
		catch (...)
		{
			return false;
		}

		// get data from groups of the message
		FIX::SecurityDesc securityDescField; // FIX field 107
		FIX::Symbol symbolField;             // FIX field 55
		FIX::NumberOfOrders numberOfOrdersField;

		FIX50SP2::MarketDataIncrementalRefresh::NoMDEntries group;
		for (size_t i = 1; i <= groups_count; ++i)
		{
			msg.getGroup(i, group);

			try
			{
				value = group.getField(securityDescField).getString();
				if (symbols.size() != 0 && symbols.find(value) == symbols.cend())
					continue;

				// discard groups with quote condition = exchange best (field 276 = 'C')
				// as they aren't book updates
				try
				{
					value = group.getField(276);
					if (value == "C") continue;
				}
				catch (...) { }

				ats::level2_message m;
				m.time = result.time;
				m.seq_number = seq_number;
				m.price = std::stoi(group.getField(270));
				m.quantity = std::stoi(group.getField(271));
/*				try
				{
					e.symbol = group.getField(symbolField).getString();;
				}
				catch(...) { }*/
				m.symbol = value;
				m.exchange = result.exchange;

				// MDUpdateAction
				value = group.getField(279);
				if (value == "0")
					m.update_action = ats::update_action::New;
				else if (value == "1")
					m.update_action = ats::update_action::Change;
				else if (value == "2")
					m.update_action = ats::update_action::Delete;
				else
					continue;

				// MDEntryType: discard anythind beyond Bid, Ask, or Trade
				value = group.getField(269);
				if (value == "0")
					m.entry_type = ats::entry_type::Bid;
				else if (value == "1")
					m.entry_type = ats::entry_type::Ask;
				else
				{
					// If a trade
					if (value == "2")
					{
						m.entry_type = ats::entry_type::Trade;
						try
						{
//							int aggressor = std::stoi(group.getField(5797));
							m.aggressor_side = std::stoi(group.getField(5797));//aggressor;
							if (m.aggressor_side == 2)
								m.aggressor_side = -1;
									//static_cast<ats::aggressor_side>(aggressor);
						}
						catch (...) { }
						result.messages.push_back(m);
					}
					continue;
				}

				m.level = std::stoi(group.getField(1023));
				m.order_count = std::stoi(group.getField(numberOfOrdersField).getString());//stoi(group.getField(346));

				result.messages.push_back(m);
			}
			catch (...)
			{
				continue;
			}
		}

		return true;
	}
}

#endif