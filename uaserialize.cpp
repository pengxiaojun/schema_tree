#include "uaserialize.h"
#include "uiinc.h"

unsigned char* ua_serialize_int(unsigned char *buffer, int num)
{
	buffer[0] = num >> 24;
	buffer[1] = num >> 16;
	buffer[2] = num >> 8;
	buffer[3] = num;
	return buffer+sizeof(num);
}

unsigned char* ua_serialize_u64(unsigned char *buffer, uint64_t num)
{
	/*size_t i;
	int movebit = sizeof(num) * 8;
	for (i = 0; i<sizeof(num); ++i)
	{
		movebit -= 8;
		buffer[i] = (unsigned char)(num >> movebit);
	}*/

	buffer[0] = (unsigned char)(num >> 56);
	buffer[1] = (unsigned char)(num >> 48);
	buffer[2] = (unsigned char)(num >> 40);
	buffer[3] = (unsigned char)(num >> 32);
	buffer[4] = (unsigned char)(num >> 24);
	buffer[5] = (unsigned char)(num >> 16);
	buffer[6] = (unsigned char)(num >> 8);
	buffer[7] = (unsigned char)num;
	return buffer+sizeof(num);
}

unsigned char* ua_serialize_i64(unsigned char *buffer, int64_t num)
{
	buffer[0] = (unsigned char)(num >> 56);
	buffer[1] = (unsigned char)(num >> 48);
	buffer[2] = (unsigned char)(num >> 40);
	buffer[3] = (unsigned char)(num >> 32);
	buffer[4] = (unsigned char)(num >> 24);
	buffer[5] = (unsigned char)(num >> 16);
	buffer[6] = (unsigned char)(num >> 8);
	buffer[7] = (unsigned char)num;
	return buffer+sizeof(num);
}

unsigned char* ua_serialize_u32(unsigned char *buffer, uint32_t num)
{
	buffer[0] = num >> 24;
	buffer[1] = num >> 16;
	buffer[2] = num >> 8;
	buffer[3] = num;
	return buffer+sizeof(num);
}

unsigned char* ua_serialize_i32(unsigned char *buffer, int32_t num)
{
	buffer[0] = num >> 24;
	buffer[1] = num >> 16;
	buffer[2] = num >> 8;
	buffer[3] = num;
	return buffer+sizeof(num);
}

unsigned char* ua_serialize_u16(unsigned char *buffer, uint16_t num)
{
	buffer[0] = (unsigned char)(num >> 8);
	buffer[1] = (unsigned char)num;
	return buffer+sizeof(num);
}

unsigned char* ua_serialize_i16(unsigned char *buffer, int16_t num)
{
	buffer[0] = (unsigned char)(num >> 8);
	buffer[1] = (unsigned char)num;
	return buffer+sizeof(num);
}

unsigned char* ua_serialize_u8(unsigned char *buffer, uint8_t num)
{
	buffer[0] = num;
	return buffer+sizeof(num);
}

unsigned char* ua_serialize_i8(unsigned char *buffer, int8_t num)
{
	buffer[0] = num;
	return buffer+sizeof(num);
}

unsigned char* ua_serialize_guid(unsigned char *buffer, const guid_t* id)
{
	buffer[0] = id->bytes[15];
	buffer[1] = id->bytes[14];
	buffer[2] = id->bytes[13];
	buffer[3] = id->bytes[12];
	buffer[4] = id->bytes[11];
	buffer[5] = id->bytes[10];
	buffer[6] = id->bytes[9];
	buffer[7] = id->bytes[8];

	buffer[8] = id->bytes[7];
	buffer[9] = id->bytes[6];
	buffer[10] = id->bytes[5];
	buffer[11] = id->bytes[4];
	buffer[12] = id->bytes[3];
	buffer[13] = id->bytes[2];
	buffer[14] = id->bytes[1];
	buffer[15] = id->bytes[0];

	return buffer+sizeof(*id);
}

unsigned char* ua_serialize_head(unsigned char *buffer, const ua_data_head_t *head)
{
	uint64_t u1, u2;
	u1 = *(uint64_t*)head;
	u2 = *(uint64_t*)((char*)head + sizeof(u1));
	buffer = ua_serialize_u64(buffer, u1);
	buffer = ua_serialize_u64(buffer, u2);
	return buffer;
}

unsigned char* ua_serialize_data(unsigned char *buffer, const ua_data_t *ud)
{
	if (ud->head.dlen == 0 || !ud->data)
	{
		return buffer;
	}
	if (ud->head.dtype == udt_int)
	{
		if (ud->head.dsubtype == unt_i || ud->head.dtype == udt_bool)
		{
			buffer = ua_serialize_int(buffer, *(int*)ud->data);
		}
		else if (ud->head.dsubtype == unt_u8)
		{
			buffer = ua_serialize_u16(buffer, *(uint8_t*)ud->data);
		}
		else if (ud->head.dsubtype == unt_i8)
		{
			  buffer = ua_serialize_i8(buffer, *(int8_t*)ud->data);
		}
		else if (ud->head.dsubtype == unt_u16)
		{
			buffer = ua_serialize_u16(buffer, *(uint16_t*)ud->data);
		}
		else if (ud->head.dsubtype == unt_i16)
		{
			buffer = ua_serialize_i16(buffer, *(int16_t*)ud->data);
		}
		else if (ud->head.dsubtype == unt_u32)
		{
			buffer = ua_serialize_u32(buffer, *(uint32_t*)ud->data);
		}
		else if (ud->head.dsubtype == unt_i32)
		{
			buffer = ua_serialize_i32(buffer, *(int32_t*)ud->data);
		}
		else if (ud->head.dsubtype == unt_u64)
		{
			buffer = ua_serialize_u64(buffer, *(uint64_t*)ud->data);
		}
		else if (ud->head.dsubtype == unt_i64)
		{
			buffer = ua_serialize_i64(buffer, *(int64_t*)ud->data);
		}
		else
		{
			//unknown type
			memcpy(buffer, ud->data, ud->head.dlen);
			buffer += ud->head.dlen;
		}
	}
	else if (ud->head.dtype == udt_str)
	{
		memcpy(buffer, ud->data, ud->head.dlen);
		buffer += ud->head.dlen;
	}
	else if (ud->head.dtype == udt_guid)
	{
		buffer = ua_serialize_guid(buffer, (guid_t*)ud->data);
	}
	else{
		//unknow type
        memcpy(buffer, ud->data, ud->head.dlen);
        buffer += ud->head.dlen;
	}

	return buffer;
}

unsigned char* ua_serialize(unsigned char *buffer, const ua_data_t *ud)
{
	return buffer;
}


//---------------------------------------deserialize-------------------------------------------//
unsigned char* ua_deserialize_int(unsigned char *buffer, int *value)
{
	int num = 0;
	
	num = buffer[0];
	num = num << 8 | buffer[1];
	num = num << 8 | buffer[2];
	num = num << 8 | buffer[3];

	*value = num;
	return buffer+sizeof(num);
}

unsigned char* ua_deserialize_u64(unsigned char *buffer, uint64_t *value)
{
	uint64_t num = 0;
	size_t i;
	num = buffer[0];
	for (i = 1; i<sizeof(num); ++i)
	{
		num = num << 8 | buffer[i];		
	}
	*value = num;
	return buffer+sizeof(num);
}

unsigned char* ua_deserialize_i64(unsigned char *buffer, int64_t *value)
{
	int64_t num = 0;
	size_t i;
	num = buffer[0];
	for (i = 1; i<sizeof(num); ++i)
	{
		num = num << 8 | buffer[i];		
	}
	*value = num;
	return buffer+sizeof(num);
}

unsigned char* ua_deserialize_u32(unsigned char *buffer, uint32_t *value)
{
	uint32_t num = 0;
	
	num = buffer[0];
	num = num << 8 | buffer[1];
	num = num << 8 | buffer[2];
	num = num << 8 | buffer[3];

	*value = num;
	return buffer+sizeof(num);
}

unsigned char* ua_deserialize_i32(unsigned char *buffer, int32_t *value)
{
	int32_t num = 0;
	
	num = buffer[0];
	num = num << 8 | buffer[1];
	num = num << 8 | buffer[2];
	num = num << 8 | buffer[3];

	*value = num;
	return buffer+sizeof(num);
}

unsigned char* ua_deserialize_u16(unsigned char *buffer, uint16_t *value)
{
	uint16_t num = 0;
	
	num = buffer[0];
	num = num << 8 | buffer[1];
	*value = num;
	return buffer+sizeof(num);
}

unsigned char* ua_deserialize_i16(unsigned char *buffer, int16_t *value)
{
	int16_t num = 0;
	
	num = buffer[0];
	num = num << 8 | buffer[1];
	*value = num;
	return buffer+sizeof(num);
}

unsigned char* ua_deserialize_u8(unsigned char *buffer, uint8_t *value)
{
	*value = buffer[0];
	return buffer+sizeof(*value);
}

unsigned char* ua_deserialize_i8(unsigned char *buffer, int8_t *value)
{
	*value = buffer[0];
	return buffer+sizeof(*value);
}

unsigned char* ua_deserialize_guid(unsigned char *buffer, guid_t* id)
{
	size_t i;
	for (i = 0; i<sizeof(*id); ++i)
	{
		id->bytes[i] = buffer[sizeof(*id) - i - 1];
	}
	return buffer+sizeof(*id);
}

unsigned char* ua_deserialize_head(unsigned char *buffer, ua_data_head_t *head)
{
	uint64_t u1 = 0ull, u2 = 0ull;
	buffer = ua_deserialize_u64(buffer, &u1);
	buffer = ua_deserialize_u64(buffer, &u2);

	memcpy((char*)head, &u1, sizeof(u1));
	memcpy((char*)head + sizeof(u1), &u2, sizeof(u2));
	return buffer;
}

unsigned char* ua_deserialize_data(unsigned char *buffer, ua_data_t *ud)
{
	if (ud->head.dtype == udt_int)
	{
		if (ud->head.dsubtype == unt_i || ud->head.dtype == udt_bool)
		{
			buffer = ua_deserialize_int(buffer, (int*)ud->data);
		}
		else if (ud->head.dsubtype == unt_u8)
		{
			buffer = ua_deserialize_u8(buffer, (uint8_t*)ud->data);
		}
		else if (ud->head.dsubtype == unt_i8)
		{
			  buffer = ua_deserialize_i8(buffer, (int8_t*)ud->data);
		}
		else if (ud->head.dsubtype == unt_u16)
		{
			buffer = ua_deserialize_u16(buffer, (uint16_t*)ud->data);
		}
		else if (ud->head.dsubtype == unt_i16)
		{
			buffer = ua_deserialize_i16(buffer, (int16_t*)ud->data);
		}
		else if (ud->head.dsubtype == unt_u32)
		{
			buffer = ua_deserialize_u32(buffer, (uint32_t*)ud->data);
		}
		else if (ud->head.dsubtype == unt_i32)
		{
			buffer = ua_deserialize_i32(buffer, (int32_t*)ud->data);
		}
		else if (ud->head.dsubtype == unt_u64)
		{
			buffer = ua_deserialize_u64(buffer, (uint64_t*)ud->data);
		}
		else if (ud->head.dsubtype == unt_i64)
		{
			buffer = ua_deserialize_i64(buffer, (int64_t*)ud->data);
		}
		else
		{
			//unknown type
			//memcpy(buffer, ud->data, ud->head.dlen);
			buffer += ud->head.dlen;
		}
	}
	else if (ud->head.dtype == udt_str)
	{
		//memcpy(buffer, ud->data, ud->head.dlen);	//alreayd do this in the caller
		buffer += ud->head.dlen;
	}
	else if (ud->head.dtype == udt_guid)
	{
		buffer = ua_deserialize_guid(buffer, (guid_t*)ud->data);
	}
	else{
		//unknow type
        buffer += ud->head.dlen;
	}

	return buffer;
}

unsigned char* ua_deserialize(unsigned char *buffer, const ua_data_t *ud)
{
	return buffer;
}
