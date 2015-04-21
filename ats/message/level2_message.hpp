#ifndef LEVEL2_MESSAGE_HPP
#define LEVEL2_MESSAGE_HPP

#include <string>
#include <vector>
#include "message.hpp"
#include "message_defs.hpp"

namespace ats
{
	// Incremental refresh message
	struct level2_message : public ats::instrument_message
	{
		price_t price;
		long quantity;
		long order_count = 0;
		size_t level;
		size_t seq_number = 0;
		ats::update_action update_action;
		ats::entry_type entry_type;
		ats::market_state state;
		int aggressor_side = 0; // 1:Buy,-1:Sell,0:Undefined - FIX field 5797 (CME)
	};

	// A collection of incremental refresh messages received a single packet from an exchange
	typedef ats::instrument_message_packet<level2_message> level2_message_packet;
}

#endif