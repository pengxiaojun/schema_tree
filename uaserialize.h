#ifndef UASERIALIZE_H
#define UASERIALIZE_H

#include "sysinc.h"

struct ua_data_t;
struct ua_data_head_t;

/*!	\func ua_serialize_u64
 *	\brief serialize uint64_t type
 *	\param buffer store result after serialized
 *	\param buffer num uint64_t number
 *	\return new buffer store serialized value
 */
unsigned char* ua_serialize_u64(unsigned char *buffer, uint64_t num);
unsigned char* ua_serialize_i64(unsigned char *buffer, int64_t num);

/*!	\func ua_serialize_u32
 *	\brief serialize uint32_t type
 *	\param buffer store result after serialized
 *	\param buffer num uint32_t number
 *	\return new buffer store serialized value
 */
unsigned char* ua_serialize_u32(unsigned char *buffer, uint32_t num);
unsigned char* ua_serialize_i32(unsigned char *buffer, int32_t num);
unsigned char* ua_serialize_int(unsigned char *buffer, int num);

/*!	\func ua_serialize_u16
 *	\brief serialize uint16_t type
 *	\param buffer store result after serialized
 *	\param buffer num uint16_t number
 *	\return new buffer store serialized value
 */
unsigned char* ua_serialize_u16(unsigned char *buffer, uint16_t num);
unsigned char* ua_serialize_i16(unsigned char *buffer, int16_t num);

/*!	\func ua_serialize_u8
 *	\brief serialize uint8_t type
 *	\param buffer store result after serialized
 *	\param buffer num uint8_t number
 *	\return new buffer store serialized value
 */
unsigned char* ua_serialize_u8(unsigned char *buffer, uint8_t num);
unsigned char* ua_serialize_i8(unsigned char *buffer, int8_t num);

/*!	\func ua_serialize_guid
 *	\brief serialize guid_t type
 *	\param buffer store result after serialized
 *	\param buffer num guid_t value
 *	\return new buffer store serialized value
 */
unsigned char* ua_serialize_guid(unsigned char *buffer, const guid_t* num);

/*!	\func ua_serialize_head
 *	\brief serialize guid_t type
 *	\param buffer store result after serialized
 *	\param head head value
 *	\return new buffer store serialized value
 */
unsigned char* ua_serialize_head(unsigned char *buffer, const ua_data_head_t *head);


/*!	\func ua_serialize_data
 *	\brief serialize data of ua_data_t type
 *	\param buffer store result after serialized
 *	\param data  data of ua_data_t value
 *	\return new buffer store serialized value
 */
unsigned char* ua_serialize_data(unsigned char *buffer, const ua_data_t *data);

/*!	\func sg_get_typeinfo
 *	\brief serialize ua_data_t type
 *	\param buffer store result after serialized
 *	\param ua head value
 *	\return new buffer store serialized value
 */
unsigned char* ua_serialize(unsigned char *buffer, const ua_data_t *ua);



//-------------------------------------------deserialize-----------------------------------------//

/*!	\func ua_deserialize_u64
 *	\brief serialize uint64_t type
 *	\param buffer store result after serialized
 *	\param buffer num uint64_t number
 *	\return new buffer store serialized value
 */
unsigned char* ua_deserialize_u64(unsigned char *buffer, uint64_t *num);
unsigned char* ua_deserialize_i64(unsigned char *buffer, int64_t *num);

/*!	\func ua_deserialize_u32
 *	\brief serialize uint32_t type
 *	\param buffer store result after serialized
 *	\param buffer num uint32_t number
 *	\return new buffer store serialized value
 */
unsigned char* ua_deserialize_u32(unsigned char *buffer, uint32_t *num);
unsigned char* ua_deserialize_i32(unsigned char *buffer, int32_t *num);
unsigned char* ua_deserialize_int(unsigned char *buffer, int *num);

/*!	\func ua_deserialize_u16
 *	\brief serialize uint16_t type
 *	\param buffer store result after serialized
 *	\param buffer num uint16_t number
 *	\return new buffer store serialized value
 */
unsigned char* ua_deserialize_u16(unsigned char *buffer, uint16_t *num);
unsigned char* ua_deserialize_i16(unsigned char *buffer, int16_t *num);

/*!	\func ua_deserialize_u8
 *	\brief serialize uint8_t type
 *	\param buffer store result after serialized
 *	\param buffer num uint8_t number
 *	\return new buffer store serialized value
 */
unsigned char* ua_deserialize_u8(unsigned char *buffer, uint8_t *num);
unsigned char* ua_deserialize_i8(unsigned char *buffer, int8_t *num);

/*!	\func ua_deserialize_guid
 *	\brief serialize guid_t type
 *	\param buffer store result after serialized
 *	\param buffer num guid_t value
 *	\return new buffer store serialized value
 */
unsigned char* ua_deserialize_guid(unsigned char *buffer, guid_t *id);

/*!	\func ua_deserialize_head
 *	\brief serialize ua_data_t header
 *	\param buffer store result after serialized
 *	\param head head value
 *	\return new buffer store serialized value
 */
unsigned char* ua_deserialize_head(unsigned char *buffer, ua_data_head_t *head);


/*!	\func ua_deserialize_data
 *	\brief serialize data of ua_data_t type
 *	\param buffer store result after serialized
 *	\param data  data of ua_data_t value
 *	\return new buffer store serialized value
 */
unsigned char* ua_deserialize_data(unsigned char *buffer, ua_data_t *ud);

/*!	\func ua_deserialize
 *	\brief serialize ua_data_t type
 *	\param buffer store result after serialized
 *	\param ua head value
 *	\return new buffer store serialized value
 */
unsigned char* ua_deserialize(unsigned char *buffer, const ua_data_t *ud);

#endif // TYPES_H
