#ifndef MESSAGE_DEFS_HPP
#define MESSAGE_DEFS_HPP

#include <string>
#include <vector>
#include "message.hpp"

namespace ats
{
	enum class trading_session_id
	{
		PreOpening = 0, Opening, Continuous
	};

	enum class update_action { New = 0, Change, Delete, Overlay };

	enum class entry_type { Bid = 0, Ask, Trade };

	enum class aggressor_side { Undefined = 0, Buy, Sell };

	enum class market_state
	{
		PreOpening, Opening, ContinuousTrading
	};
}

#endif