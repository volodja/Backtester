#ifndef MESSAGE_READER_HPP
#define MESSAGE_READER_HPP

#include <ats/event_handler/multievent_handler.hpp>
#include <ats/message/message.hpp>

namespace ats
{
	class message_reader
	{
	public:
		virtual ~message_reader() { }

		// Read the next message (return true if successful and false otherwise)
		virtual bool read() = 0;

		// Send message to a multi-event handler
		virtual void send_message(ats::portfolio_base* universe) const = 0;

		template<typename CallableT>
		void send(CallableT& f)
		{
			f(static_cast<decltype(get_last_message())>(get_last_message()));
		}

		// The last read message casted to the base class type
		virtual const ats::message& get_last_message() const = 0;
	};

	template<typename MessageT>
	class single_message_reader : public ats::message_reader
	{
	public:
		virtual ~single_message_reader() { }

/*		virtual void send_message(ats::multievent_handler* handler) const override
		{
			handler->invoke(message_);
		}*/

		virtual void send_message(ats::portfolio_base* universe) const override
		{
			universe->invoke(message_);
		}

		virtual const ats::message& get_last_message() const override
		{
			return static_cast<const ats::message&>(message_);
		}

	protected:
		MessageT message_;
	};
}

#endif