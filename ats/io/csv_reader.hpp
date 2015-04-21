#ifndef CSV_READER_HPP
#define CSV_READER_HPP

#include <string>
#include <array>
#include <fstream>

namespace ats
{
	template<size_t num_columns>
	class csv_reader
	{
	public:
		csv_reader(std::ifstream* stream = nullptr, char delim = ';', bool header = true)
			: stream_(stream), delim_(delim), header_flag_(header) { }

		void set_stream(std::ifstream* stream) { stream_ = stream; }
		void set_delim(char delim) { delim_ = delim; }

		const std::array<std::string, num_columns>& header() const { return header_; }

		bool read(std::array<std::string, num_columns>& fields)
		{
			if (header_flag_)
			{
				std::getline(*stream_, line);
				parse(line, header_);
				header_flag_ = false;
			}

			if (std::getline(*stream_, line))
			{
				parse(line, fields);
				return true;
			}
			return false;
		}

	private:
		void parse(const std::string& str, std::array<std::string, num_columns>& arr)
		{
			size_t i = arr.size() - 1;
			std::string::const_iterator end = str.cend();
			for (auto it = str.cend(); it != str.cbegin(); --it)
			{
				if (*it == delim_)
				{
					arr[i--].assign(it + 1, end);
					end = it;
				}
			}
			arr[0].assign(str.cbegin(), end);
		}

	private:
		std::array<std::string, num_columns> header_;
		std::string line;
		std::ifstream* stream_;
		char delim_ = ';';
		bool header_flag_;
	};
}

#endif