#ifndef TRADE_MESSAGE_HPP
#define TRADE_MESSAGE_HPP

#include <string>
#include <vector>
#include "message_defs.hpp"

namespace ats
{
	// Trade message
	struct trade_message : public ats::instrument_message
	{
		price_t price;
		long quantity;
		size_t seq_number = 0;
		ats::market_state state;
		ats::aggressor_side aggressor_side = ats::aggressor_side::Undefined; // FIX field 5797 (CME)
	};

	// A collection of trade messages received a single packet from an exchange
	typedef ats::instrument_message_packet<ats::trade_message> trade_message_packet;
}

#endif