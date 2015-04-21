#ifndef FIX_TO_CSV_HPP
#define FIX_TO_CSV_HPP

#include <fstream>
#include <string>
#include <unordered_set>
#include <ats/io/parser/fix_parser.hpp>
#include <ats/message/level2_message.hpp>

namespace ats
{
	static void fix_to_csv(const std::string& fix_file, const std::string& csv_file,
			const std::string& fix_specs_xml, const std::unordered_set<std::string>& symbols,
			bool print_seq_num = false, const std::string& eop_str = "EOP")
	{
		std::ifstream fix(fix_file);
		std::ofstream csv(csv_file);

		FIX::DataDictionary dictionary(fix_specs_xml);

		auto entry_type_to_char = [](ats::entry_type type)
		{
			switch (type)
			{
			case ats::entry_type::Bid:
				return 'B';
			case ats::entry_type::Ask:
				return 'A';
			case ats::entry_type::Trade:
				return 'T';
			default:
				throw std::invalid_argument("Unknown entry type");
			}
		};

		auto update_action_to_char = [](ats::update_action action)
		{
			switch (action)
			{
			case ats::update_action::New:
				return 'N';
			case ats::update_action::Change:
				return 'C';
			case ats::update_action::Delete:
				return 'D';
			default:
				throw std::invalid_argument("Unknown update action");
			}
		};

		std::string fix_msg;
		ats::level2_message_packet msg;
		while (std::getline(fix, fix_msg))
		{
			if (ats::parse_fix_msg(fix_msg.c_str(), msg, dictionary, symbols) && !msg.messages.empty())
			{
				for (const auto& m : msg.messages)
				{
					csv << m.exchange << ',' << m.symbol << ',';
					if (print_seq_num)
						csv << m.seq_number << ',';
					csv << m.time.to_string("%Y%m%d %H%M%S.%f") << ',' << update_action_to_char(m.update_action) << ','
						<< entry_type_to_char(m.entry_type) << ',' << m.price << ',' << m.quantity << ','
						<< m.order_count << ',' << m.level << '\n';
				}
				csv << eop_str;
			}
		}

		csv.close();
		fix.close();
	}
}

#endif