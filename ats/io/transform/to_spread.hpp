#ifndef TO_SPREAD_HPP
#define TO_SPREAD_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <ats/date_time/date_time.hpp>

namespace ats
{
	static void to_spread(const std::string& x_file, const std::string& y_file, const std::string& out_file,
			const std::string& header = "Symbol; Timestamp; BidPrc; AskPrc")
	{
		struct bbo
		{
			int seq_num;
			boost::posix_time::ptime time;
			int bid, ask;
			std::string symbol;
		};

		std::vector<bbo> x, y;
		bbo bbo_;

		boost::posix_time::time_input_facet* facet = new boost::posix_time::time_input_facet(1);
		std::stringstream ss;
		ss.imbue(std::locale(std::locale(), facet));
		facet->format("%Y%m%d %H%M%S%F");

		char symbol[15], exchange[15], time[50];

		std::FILE* file = std::fopen(x_file.c_str(), "r");
		std::fscanf(file, "%*[^\n]\n", nullptr);

		while (std::fscanf(file, "%[^;];%[^;];%d;%[^;];%d;%d\n",
				symbol, exchange, &bbo_.seq_num, time, &bbo_.bid, &bbo_.ask) == 6)
		{
			ss.str(time);
			ss >> bbo_.time;
			ss.clear();

			bbo_.symbol.assign(symbol);

			x.push_back(bbo_);
		}

		std::fclose(file);

		file = std::fopen(y_file.c_str(), "r");
		std::fscanf(file, "%*[^\n]\n", nullptr);

		while (std::fscanf(file, "%[^;];%[^;];%d;%[^;];%d;%d\n",
				symbol, exchange, &bbo_.seq_num, time, &bbo_.bid, &bbo_.ask) == 6)
		{
			ss.str(time);
			ss >> bbo_.time;
			ss.clear();

			bbo_.symbol.assign(symbol);

			bbo_.bid /= 5;
			bbo_.ask /= 5;
			y.push_back(bbo_);
		}

		std::fclose(file);

		// compute the spread
		std::ofstream out(out_file);
		out << header << '\n';

		size_t i = 0, j = 0;
		int seq_num = std::min(x[0].seq_num, y[0].seq_num);

		boost::posix_time::ptime tm = x[0].time;

		if (x[0].seq_num > y[0].seq_num)
		{
			for (j = 1; j < y.size() && y[j].seq_num < x[0].seq_num; ++j) ;
			--j;
		}
		else if (x[0].seq_num < y[0].seq_num)
		{
			for (i = 1; i < x.size() && x[i].seq_num < y[0].seq_num; ++i) ;
			--i;
			tm = y[0].time;
		}

		int bid, ask;
		std::string tm_str;
		while (i < x.size() - 1 && j < y.size() - 1)
		{
			bid = x[i].bid - y[j].ask;
			ask = x[i].ask - y[j].bid;

//			ats::date_time::date_time dt(tm);
			tm_str = ats::date_time::date_time(tm).to_string("%Y%m%d %H%M%S.%f");
			tm_str = tm_str.substr(0, tm_str.size() - 3);
			out << x[i].symbol << "-" << y[j].symbol << "; " << tm_str << "; " << bid << "; " << ask << '\n';

			if (x[i + 1].seq_num < y[j + 1].seq_num)
			{
				tm = x[++i].time;
			}
			else if (x[i + 1].seq_num > y[j + 1].seq_num)
			{
				tm = y[++j].time;
			}
			else
			{
				tm = x[++i].time;
				++j;
			}
		}

		out.close();
	}
}

#endif