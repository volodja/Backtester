#ifndef CSV_MESSAGE_READER_HPP
#define CSV_MESSAGE_READER_HPP

#include <ats/io/csv_reader.hpp>
#include "message_reader.hpp"

namespace ats
{
	template<typename MessageT, size_t n_columns>
	class csv_single_message_reader : public ats::single_message_reader<MessageT>
	{
	public:
		csv_single_message_reader(const std::string& filename)
			: ats::single_message_reader<MessageT>(), stream_(filename), reader_(&stream_) { }

	private:
		std::ifstream stream_;                       // file containing messages
		ats::csv_reader<n_columns> reader_;          // reads fields of CSV files line by line
		std::array<std::string, n_columns> fields_;  // fields of a line in our CSV file
	};
}

#endif