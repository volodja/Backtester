#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>
#include <utility>
#include <ats/types.hpp>

namespace ats
{
	struct message
	{
		ats::timestamp_t time;

		virtual ~message() { }

		message() = default;
		message(const ats::timestamp_t& time) : time(time) { }
		message(ats::timestamp_t&& time) : time(std::move(time)) { }
	};

	struct instrument_message : public ats::message
	{
		virtual ~instrument_message() { }

		std::string symbol;
		std::string exchange;
	};

	template<typename MessageT>
	struct instrument_message_packet : public ats::instrument_message
	{
		typedef typename std::vector<MessageT>::iterator iterator;
		typedef typename std::vector<MessageT>::const_iterator const_iterator;
		iterator begin() { return messages.begin(); }
		const_iterator cbegin() const { return messages.cbegin(); }
		iterator end() { return messages.end(); }
		const_iterator cend() const { return messages.cend(); }

		MessageT& operator[](size_t index) { return messages[index]; }
		const MessageT& operator[](size_t index) const { return messages[index]; }
		
		// A collection of messages in the packet
		std::vector<MessageT> messages;
	};
}

#endif