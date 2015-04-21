#ifndef STREAM_READER_HPP
#define STREAM_READER_HPP

#include <istream>
#include <vector>
#include <queue>

namespace ats
{
	template<typename MsgT>
	class stream_reader
	{
	public:
		virtual ~stream_reader() { }

		virtual bool read(std::queue<MsgT>& messages) = 0;
	private:
		std::istream* stream_ = nullptr;
	};
}

#endif