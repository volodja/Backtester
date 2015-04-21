#ifndef EXCHANGE_MESSAGE_READER_BASE_HPP
#define EXCHANGE_MESSAGE_READER_BASE_HPP

#include <ats/portfolio/portfolio_base.hpp>
#include "message_reader.hpp"

namespace ats
{
	template<typename MessageT>
	class exchange_message_reader_base : public ats::message_reader
	{
	public:
		virtual ~exchange_message_reader_base() { }

		virtual void send_message(ats::portfolio_base* universe) const override
		{
			const ats::instrument_message& msg = static_cast<const ats::instrument_message&>(message_);
			ats::execution_engine* engine = universe->get_execution_engine(msg.exchange);
			if (engine != nullptr)
				engine->invoke(message_);//(static_cast<const MessageT&>(message_));
		}

		virtual const ats::message& get_last_message() const override
		{
			return static_cast<const ats::message&>(message_);
		}

		const MessageT& get_last_true_message() const
		{
			return message_;
		}
	
	protected:
		MessageT message_;
	};
}

#endif