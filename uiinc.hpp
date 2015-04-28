#pragma once

#include "uiinc.h"

namespace dyndef
{
	struct UaHelper
	{
		ua_data_t uad;

		UaHelper()
		{
			ua_init(&uad);
		}
		~UaHelper()
		{
			ua_de_collapse(&uad);
			ua_deinit(&uad);
		}

		void* Encode(int& Alen)
		{
			void* mem = NULL;
			ua_expand_data(&uad, &mem, &Alen);
				return mem;
		}
		bool Decode(void* Adat, int Alen)
		{
			return ua_collapse_data(Adat, &Alen, &uad) == GE_OK;
		}
	};
}
