#ifndef INIT_HPP
#define INIT_HPP

#include <string>
#include <array>
#include <vector>
#include <map>
#include <set>
#include <map>
#include <fstream>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <ats/custom_message_readers/level2_message_reader.hpp>
#include <ats/order_book/exchange_order_book.hpp>

void transform(const std::string& dir, const std::string& filename)
{
	std::string head = filename.substr(0, 3);
	std::transform(head.begin(), head.end(), head.begin(), toupper);
	head += "_" + filename.substr(4, 13);

	std::string folder = "Results/" + head.substr(0, 6);
	boost::filesystem::path directory(folder);
	if (!boost::filesystem::exists(directory))
		boost::filesystem::create_directory(directory);

	std::ifstream file(dir + filename);
	std::ofstream out(folder + "/" + head + ".txt");

	ats::timestamp_t date(head.substr(9, 8), "%Y%m%d");

	std::string line;
	std::array<std::string, 7> fields;
	while (std::getline(file, line))
	{
		if (ats::tokenize(line, fields, ',') < fields.size())
		{
			out << '\n';
			continue;
		}

		long microseconds = std::stol(fields[0]);
		ats::timestamp_t time(date + boost::posix_time::microseconds(microseconds));

		char update_action;
		char entry_type;
		if (fields[1] == "C") update_action = 'C';
		else if (fields[1] == "D") update_action = 'D';
		else if (fields[1] == "A") update_action = 'N';
		else if (fields[1] == "T")
		{
			update_action = 'N';
			entry_type = 'T';
		}

		if (fields[2] == "B") entry_type = 'B';
		else if (fields[2] == "A") entry_type = 'A';

		out << time.to_string("%Y%m%d %H%M%S.%f") << ',' << update_action << ',' << entry_type;
		for (size_t i = 3; i < fields.size(); ++i)
			out << ',' << fields[i];
		out << '\n';
	}

	out.close();
	file.close();
}


void to_trades(const std::string& dir, const std::string& filename)
{
	std::string head = filename.substr(0, 8);

	std::string folder = "Results/" + filename.substr(0, 8);
	boost::filesystem::path directory(folder);
	if (!boost::filesystem::exists(directory))
		boost::filesystem::create_directory(directory);

	std::ofstream out(folder + "/" + filename);

	ats::exchange_order_book book({"GCZ4", 1}, "CME", 10);
	ats::l2_message_reader reader(dir + "/" + filename, "GCZ4", "CME");
	while (reader.read())
	{
		const ats::level2_message_packet& msg = reader.get_last_true_message();
		bool has_trades = false;
		for (const auto& m : msg.messages)
		{
			book.update(m);
			if (m.entry_type == ats::entry_type::Trade)
			{
				has_trades = true;
				out << m.time.to_string("%Y%m%d %H%M%S.%f") << ',' << m.price << ',' << m.quantity << ',';
				if (book.best_ask() != nullptr && m.price >= book.best_ask()->price)
					out << "A\n";
				else if (book.best_bid() != nullptr && m.price <= book.best_bid()->price)
					out << "B\n";
				else
					out << "U\n";
			}
		}

		if (has_trades)
			out << '\n';
	}

	out.close();
}

// Returns (Exchange, Symbol)
std::pair<std::string, std::string> get_full_symbol(const std::string& filename)
{
	std::string symbol, exchange;
	std::string result[2];
	size_t begin = 0, end = 0;
	for (size_t i = 0; i < 2; ++i)
	{
		while (end < filename.size() && filename[end] != '_') ++end;
		result[i] = filename.substr(begin, end - begin);
		if (i == 0)
			begin = ++end;
	}
	return std::make_pair(result[0], result[1]);
}

ats::timestamp_t get_date(const std::string& filename)
{
	size_t begin = 0;
	for (size_t i = 0; i < 2; ++i)
	{
		while (begin < filename.size() && filename[begin] != '_') ++begin;
		++begin;
	}
	std::string date = filename.substr(begin, 8);
	return ats::timestamp_t(date, "%Y%m%d");
}

void run(const std::vector<std::string>& symbols, const std::string& datasource,
		const std::string& date1, const std::string& date2,	portfolio& port)
{
	using pair = std::pair<std::string, std::string>;
	std::multimap<ats::timestamp_t, pair> files;

	ats::timestamp_t time1(date1, "%Y%m%d");
	ats::timestamp_t time2(date2, "%Y%m%d");
	for (const auto& sym : symbols)
	{
		// Extract symbol and exchange
		size_t i = 0, j = 0;
		while (j < sym.length() && sym[j] != '_') ++j;
		std::string exchange = sym.substr(i, j);
		i = ++j;
		while (j < sym.length() && sym[j] != '_') ++j;
		std::string symbol = sym.substr(i, j - i);
		++j;

		std::string folder = datasource + exchange + '_' + symbol + '/';
		boost::filesystem::path directory(folder);
		boost::filesystem::directory_iterator it(directory), end;
		std::string filename;
		for (; it != end; ++it)
		{
			filename = it->path().filename().c_str();
			ats::timestamp_t time = get_date(filename);
			if (time >= time1 && time <= time2)
				files.insert(std::make_pair(time, std::make_pair(exchange, symbol)));
		}
	}

	using reader_type = ats::l2_message_reader;
	using reader_ptr_type = std::shared_ptr<reader_type>;
	for (auto it = files.cbegin(); it != files.cend(); it = files.upper_bound(it->first))
	{
		std::stringstream ss;
		std::string symbol = it->second.second;
		std::string exchange = it->second.first;
		std::string folder = exchange + '_' + symbol;
		ss << datasource << folder << '/' << folder << '_' << it->first.to_string("%Y%m%d") << ".txt";
		std::cout << ss.str() << '\n';

		ats::historical_data_feed feed(&port);
		for (auto ii = it; ii != files.upper_bound(it->first); ++ii)
		{
			reader_ptr_type reader(new reader_type(ss.str(), symbol, exchange));
			feed.add_message_reader(reader);
		}

		// Replay
		feed.run();
	}
}

#endif
