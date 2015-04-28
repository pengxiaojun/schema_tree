#include <limits.h>
#include "structgrab.h"
#include "uiinc.h"


sg_type_info_t type_info_table[] = 
{
	{'g', sizeof(guid_t), udt_guid, udt_guid},
	{'c', sizeof(uint16_t), udt_int, unt_u16},
	{'i', sizeof(uint32_t), udt_int, unt_u32},
	{'l', sizeof(uint64_t), udt_int, unt_u64},
	{'t', sizeof(wchar_t), udt_str, ust_wchar},
	{'b', sizeof(char), udt_str, ust_char}
};

const sg_type_info_t* GRCALL sg_get_typeinfo(const char type)
{
	for (size_t i = 0; i < sizeof(type_info_table) / sizeof(type_info_table[0]); ++i)
	{
		const sg_type_info_t *ti = &type_info_table[i];
		if (ti->token == type)
		{
			return ti;
		}
	}
	return NULL;
}

int GRCALL sg_parse_array_expr(const char *data, int len, const char *expr, sg_expr_t *se)
{
	char type = *expr;
	//get array length from 
	uint32_t array_len = 0;
	if (type == '$')
	{
		sg_expr_t _set;
		memset(&_set, 0, sizeof(_set));
		ua_data_t ud;
		int ret = sg_parse_variable_expr(data, len, expr+1, &_set, &ud);

		if (ret == -1)
		{
			return -1;
		}
		//array length is always a number
		if (ud.head.dtype != udt_int ||
			ud.data == NULL)
			return -1;

		array_len = *(uint32_t*)ud.data;
	}
	else
	{
		array_len = strtoul(expr, NULL, 10);
		if (array_len == ULONG_MAX)
		{
			return -1;
		}
	}
	se->len = (int)array_len;
	se->plural = 1;
	
	return 0;
}

//fomat: i=[startbit, bitcount]
int GRCALL sg_parse_bitfield_expr(const char *expr, sg_expr_t *se)
{
	//bit field
	//get start bit
	uint32_t num = strtoul(expr, NULL, 10);
	if (num == ULONG_MAX)
	{
		return -1;
	}
	se->bitstart = (int)num;
	const char *sep = strrchr(expr, ',');

	if (!sep){
		return -1;	//missing ','
	}

	//get bits
	num = strtoul(sep+1, NULL, 10);
	if (num == ULONG_MAX)
	{
		return -1;
	}
	se->bitcount = num;
	se->bitfield = 1;
	return 0;
}

//format g[4,1]gggg
int GRCALL sg_parse_variable_expr(const char *data, int len, const char *expr, sg_expr_t *se, ua_data_t *ud)
{
	//get the type of varaible. varaible is alaways is a number type
	//because the variable represent the data length
	char type = *expr;
	const sg_type_info_t *ti = sg_get_typeinfo(type);
	if (!ti)
	{
		return -1;
	}
	memcpy(&se->ti, ti, sizeof(*ti));
	//need read bytes after skip
	char next = *(expr+1);

	const char *endsquare = strrchr(expr, ']');
	//variable is bit filed type
	if (next == '[')
	{
		int ret = sg_parse_bitfield_expr(expr + 2, se);
		if (ret == -1)
		{
			return -1;
		}
	}
	if (endsquare)
	{
		strcpy(se->varexpr, endsquare+1);
	}
	else
	{
		strcpy(se->varexpr, expr+1);
	}
	char var_expr[2] = {0};
	var_expr[0] = type;
	
	//grab $variable value
	return sg_grab_data(data, len, se->varexpr, var_expr, ud);
}

int GRCALL sg_parse_expr(const char *data, int len, const char *expr, sg_expr_t *se)
{
	char type = *expr;
	char op = *(expr + 1);

	const sg_type_info_t *ti = sg_get_typeinfo(type);
	if (!ti)
	{
		return -1;
	}

	memcpy(&se->ti, ti, sizeof(*ti));
	if (!op)
	{
		return 0;
	}

	if (op == '=')
	{
		return sg_parse_array_expr(data, len, expr + 2, se);	
	}
	else if (op == '[')
	{
		return sg_parse_bitfield_expr(expr + 2, se);
	}
	else if (op == '$')
	{
		ua_data_t ud;
		int ret = sg_parse_variable_expr(data, len, expr + 2, se, &ud);
		if (ret == -1)
			return -1;
		
		if (ud.head.dtype == udt_int)
		{
			if (!ud.data) return -1;
			se->len = *(int*)ud.data;
		}
		else
		{
			//not support
			return -1;
		}
	}
	return -1;	//unsupport
}

int GRCALL sg_calc_skip_len(const char *skipexpr)
{
	char token = *skipexpr;
	int skiplen = 0;

	while ((token = *skipexpr))
	{
		const sg_type_info_t * sti = sg_get_typeinfo(token);
		if (!sti)
		{
			return -1;
		}
		int amount = 1;
		//check whether if array
		token = *(skipexpr+1);
		if (token && token == '=')
		{
			skipexpr += 2;	//skip 't='
			//get array length
			//skip array length
			char num;
			amount = 0;
			while (isdigit(num = *skipexpr))
			{
				if (amount == 0) amount = num - '0';
				else amount = (amount * 10) + (num - '0');
				skipexpr++;
			}
		}
		else
		{
			skipexpr++;
		}
		skiplen += (amount * sti->bytes);
	}
	return skiplen;
}

int GRCALL sg_grab_bitfiled_data(const char *buffer, sg_expr_t *se, void *out)
{
	int totalbits = se->ti.bytes * 8;

	if (se->ti.uatype != udt_int)
	{
		return -1;
	}
	if (se->bitstart > totalbits || se->bitcount > totalbits)
	{
		return -1;
	}
	uint64_t mask = 0ull;
	for (int i = 0; i<se->bitcount; ++i)
	{
		mask = (mask << 1) | 0x01;
	}

	//may be the bit count is 0
	//if (mask == 0ull) mask = UINT64_MAX;

	if (se->ti.uasubtype == unt_i)
	{
		int i = *(int*)buffer;
		i >>= se->bitstart;
		i &= mask;
		memcpy(out, &i, se->ti.bytes);
	}
	else if (se->ti.uasubtype == unt_u8)
	{
		uint8_t i = *(uint8_t*)buffer;
		i >>= se->bitstart;
		i &= mask;
		memcpy(out, &i, se->ti.bytes);
	}
	else if (se->ti.uasubtype == unt_u16)
	{
		uint16_t i = *(uint16_t*)buffer;
		i >>= se->bitstart;
		i &= mask;
		memcpy(out, &i, se->ti.bytes);
	}
	else if (se->ti.uasubtype == unt_u32)
	{
		uint32_t i = *(uint32_t*)buffer;
		i >>= se->bitstart;
		i &= mask;
		memcpy(out, &i, se->ti.bytes);
	}
	else if (se->ti.uasubtype == unt_u64)
	{
		uint64_t i = *(uint64_t*)buffer;
		i >>= se->bitstart;
		i &= mask;
		memcpy(out, &i, se->ti.bytes);
	}
	else{
		//TODO: support other type
		return -1;
	}
	return 0;
}

int GRCALL sg_grab_data(const char *data, int len, const char *skipexpr, const char *expr, ua_data_t *ud)
{
	int skiplen = sg_calc_skip_len(skipexpr);
	
	if (skiplen > len)
	{
		return -1;
	}
	const char *pstart = data + skiplen;

	//parse expr
	sg_expr_t se;
	memset(&se, 0, sizeof(se));
	int ret = sg_parse_expr(data, len, expr, &se);

	if (ret == -1)
	{
		return -1;
	}

	//get the expect data by expr
	ua_init(ud);
	ud->head.array = se.plural;
	ud->head.nchildren = se.len;
	ud->head.dlen = se.ti.bytes;
	ud->head.dtype = se.ti.uatype;
	ud->head.dsubtype = se.ti.uasubtype;

	if (ud->head.array)
	{
		void *array_data = calloc(ud->head.nchildren, ud->head.dlen);
		memcpy(array_data, pstart, (size_t)(ud->head.nchildren * ud->head.dlen));

		if (ud->head.dtype == udt_int)
		{
			//read int array data
			ua_set_num_array(array_data, ud->head.nchildren, ud->head.dsubtype, ud);
		}
		else if (ud->head.dtype == udt_str)	//char str[len]
		{
			ua_copy_data(ud, array_data, size_t(ud->head.nchildren * ud->head.dlen));
		}
		else if (ud->head.dtype == udt_guid)
		{
			ua_set_guid_array((guid_t*)array_data, ud->head.nchildren, ud);
		}
		else{
			//not support
		}
	}
	else
	{
		if (ud->head.dtype == udt_int)	//only number type support bitfield
		{
			void *uad = calloc(1, ud->head.dlen);
			if (se.bitfield)
			{
				sg_grab_bitfiled_data(pstart, &se, uad);
			}
			else
			{
				memcpy(uad, pstart, ud->head.dlen);
			}
			ua_copy_data(ud, uad, ud->head.dlen);
			free(uad);
		}
		else
		{
			void *uad = calloc(1, ud->head.dlen);
			memcpy(uad, pstart, ud->head.dlen);
			ua_copy_data(ud, uad, ud->head.dlen);
			free(uad);
		}
	}
	return 0;
}
