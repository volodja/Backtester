#ifndef DATETIME_HPP
#define DATETIME_HPP

#include <string>
#include <cstdlib>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

namespace ats {
namespace date_time {
	class date_time
	{
	public:
		typedef boost::gregorian::date::year_type year_type;
		typedef boost::gregorian::date::month_type month_type;
		typedef boost::gregorian::date::day_type day_type;
		typedef boost::gregorian::date::day_of_week_type day_of_week_type;
		typedef boost::gregorian::date::day_of_year_type day_of_year_type;
		typedef boost::posix_time::time_duration::hour_type	hour_type;
		typedef boost::posix_time::time_duration::min_type min_type;
		typedef boost::posix_time::time_duration::sec_type	sec_type;
		typedef boost::posix_time::time_duration::fractional_seconds_type fractional_seconds_type;

		date_time() : datetime_() { }
		date_time(const boost::posix_time::ptime& datetime) : datetime_(datetime) { }
		date_time(const boost::gregorian::date& date) : datetime_(date) { }
		date_time(const date_time& datetime) : datetime_(datetime.datetime_) { }
		date_time(year_type year, month_type month, day_type day)
			: datetime_(boost::gregorian::date(year, month, day)) { }
		date_time(const boost::gregorian::date& date, const boost::posix_time::time_duration& time)
			: datetime_(date, time) { }
		date_time(boost::posix_time::special_values sv) : datetime_(sv) { }
		date_time(const std::string& datetime, const std::string& fmt = "%Y%m%d %H%M%S%F")
		{
			this->parse(datetime, fmt);
		}

		// Returns current time in UTC
		static date_time now()
		{
			return date_time(boost::posix_time::microsec_clock::universal_time());
		}

		bool is_not_a_date_time() const { return datetime_.is_not_a_date_time(); }
		bool is_neg_infinity() const { return datetime_.is_neg_infinity(); }
		bool is_pos_infinity() const { return datetime_.is_pos_infinity(); }
		bool is_infinity() const { return datetime_.is_infinity(); }
		bool is_special() const { return datetime_.is_special(); }

		year_type year() const { return datetime_.date().year(); }
		month_type month() const { return datetime_.date().month(); }
		day_type day() const { return datetime_.date().day(); }
		hour_type hour() const { return datetime_.time_of_day().hours(); }
		min_type minute() const { return datetime_.time_of_day().minutes(); }
		sec_type second() const { return datetime_.time_of_day().seconds(); }
		long long int millisecond() const
		{
			boost::posix_time::time_duration durat(datetime_.time_of_day());
			durat = durat - boost::posix_time::hours(durat.hours()) -
					boost::posix_time::minutes(durat.minutes()) - boost::posix_time::seconds(durat.seconds());
			return durat.total_milliseconds();
		}

		boost::gregorian::date date() const { return datetime_.date(); }
		day_of_week_type day_of_week() const { return datetime_.date().day_of_week(); }
		day_of_year_type day_of_year() const { return datetime_.date().day_of_year(); }
		boost::posix_time::time_duration time_of_day() const { return datetime_.time_of_day(); }

		// Operators:
		// Compare two date_time objects
		bool operator < (const date_time& other) const { return datetime_ < other.datetime_; }
		bool operator < (const boost::posix_time::ptime& other) const { return datetime_ < other; }
		bool operator > (const date_time& other) const { return datetime_ > other.datetime_; }
		bool operator > (const boost::posix_time::ptime &other) const { return datetime_ > other; }
		bool operator == (const date_time& other) const { return datetime_ == other.datetime_; }
		bool operator == (const boost::posix_time::ptime &other) const { return datetime_ == other; }
		bool operator != (const date_time& other) const { return datetime_ != other.datetime_; }
		bool operator != (const boost::posix_time::ptime &other) const { return datetime_ != other; }
		bool operator <= (const date_time& other) const { return datetime_ <= other.datetime_; }
		bool operator >= (const date_time& other) const { return datetime_ >= other.datetime_; }
		
		// Adding/subtracting time to/from a date_time
		date_time operator + (const boost::posix_time::time_duration& duration) { return date_time(datetime_ + duration); }
		date_time operator - (const boost::posix_time::time_duration& duration) { return date_time(datetime_ - duration); }
		const date_time operator + (const boost::posix_time::time_duration& duration) const { return date_time(datetime_ + duration); }
		const date_time operator - (const boost::posix_time::time_duration& duration) const { return date_time(datetime_ - duration); }
		
		// Time difference between two date_time objects
		boost::posix_time::time_duration operator - (const date_time& other) { return datetime_ - other.datetime_; }
		boost::posix_time::time_duration operator - (const boost::posix_time::ptime& other) { return datetime_ - other; }
		const boost::posix_time::time_duration operator - (const date_time& other) const { return datetime_ - other.datetime_; }
		const boost::posix_time::time_duration operator - (const boost::posix_time::ptime& other) const { return datetime_ - other; }
		
		date_time& operator += (const boost::posix_time::time_duration& duration) { datetime_ += duration; return *this; }
		date_time& operator -= (const boost::posix_time::time_duration& duration) { datetime_ -= duration; return *this; }
		date_time& operator = (const date_time& other) { datetime_ = other.datetime_; return *this; }
		date_time& operator = (const boost::posix_time::ptime& other) { datetime_ = other; return *this; }
		date_time& operator = (const boost::gregorian::date& other) { datetime_ = boost::posix_time::ptime(other); return *this; }

		// Convert date_time to Boost's ptime
		operator boost::posix_time::ptime() const { return datetime_; }

	/*	void AddMonths(int months)
		{
			boost::gregorian::month_iterator mit(date.date(), months);
			++mit;
			date = ptime(*mit, date.time_of_day());
		}*/

		// Datetime string must have the format "%Y%m%d %H%M%S%F"
		void parse_simple(const char* datetime)
		{
			char* stop;
			long date = std::strtol(datetime, &stop, 10);
			long day = date % 100;
			date = (date - day) / 100;
			long month = date % 100;
			long year = (date - month) / 100;
			boost::posix_time::ptime result(boost::gregorian::date(year, month, day));

			long time = std::strtol(stop + 1, &stop, 10);
			long sec = time % 100;
			time = (time - sec) / 100;
			long min = time % 100;
			long hour = (time - min) / 100;

			if (*stop == '.')
			{
				long frac_sec = std::strtol(stop + 1, nullptr, 10);
				double frac = std::strtod(stop, nullptr);
				long resolution = (double)frac_sec / frac;
				frac_sec *= boost::posix_time::time_duration::ticks_per_second() / resolution;
				result += boost::posix_time::time_duration(hour, min, sec, frac_sec);
			}
			else
				result += boost::posix_time::time_duration(hour, min, sec);

			datetime_ = result;
		}

		// Datetime string must have the format "%Y%m%d %H%M%S%F"
		void parse_simple(const std::string& datetime)
		{
			parse_simple(datetime.c_str());
		}

		void parse(const char* datetime, const char* fmt = "%Y%m%d %H%M%S%F")
		{
			static boost::posix_time::time_input_facet* facet = nullptr;//new time_input_facet(format);
			static std::stringstream ss;//(date_str);
			if (facet == nullptr)
			{
				facet = new boost::posix_time::time_input_facet(1);
				ss.imbue(std::locale(std::locale(), facet));   // ss.imbue(locale(ss.getloc(), facet));
			}

			facet->format(fmt);//.c_str());
			ss.str(datetime);

			ss >> datetime_;
			ss.clear();
		}

		void parse(const std::string& datetime, const std::string& format = "%Y%m%d %H%M%S%F")
		{
			static boost::posix_time::time_input_facet* facet = nullptr;//new time_input_facet(format);
			static std::stringstream ss;//(date_str);
			if (facet == nullptr)
			{
				facet = new boost::posix_time::time_input_facet(1);
				ss.imbue(std::locale(std::locale(), facet));   // ss.imbue(locale(ss.getloc(), facet));
			}

			facet->format(format.c_str());
			ss.str(datetime);

			ss >> datetime_;
			ss.clear();
		}

		// Before applying this function, make sure that
		// ss.imbue with a proper time_input_facet has been used
		void parse(const char* datetime, std::stringstream& ss)
		{
			ss.str(datetime);
			ss >> datetime_;
			ss.clear();
		}

		void parse(const std::string& datetime, std::stringstream &ss)
		{
			ss.str(datetime);
			ss >> datetime_;
			ss.clear();
		}

		std::string to_string(const char* fmt = "%Y-%m-%d %H:%M:%S.%f") const
		{
			static boost::posix_time::time_facet* facet = nullptr;//new time_input_facet(format);
			static std::stringstream ss;
			if (facet == nullptr)
			{
				facet = new boost::posix_time::time_facet(fmt);
				ss.imbue(std::locale(std::locale(), facet));   // ss.imbue(locale(ss.getloc(), facet));
			}

			ss.str("");
			ss.clear();

			ss << datetime_;
			return ss.str();
		}
	//	void FromString(const char *date_stre, const char *format = "%Y%m%d %H%M%S");
	//	void FromString(const char *date_str, time_input_facet *facet);
	
		friend std::ostream& operator << (std::ostream& out, const date_time& datetime)
		{
			out << datetime.datetime_;
			return out;
		}
		
	private:
		boost::posix_time::ptime datetime_;
	};
}
}

#endif