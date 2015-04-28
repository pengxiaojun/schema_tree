#ifndef STRUCTGRAB_H
#define STRUCTGRAB_H

#include "sysinc.h"

struct ua_data_t;

/*! file structgrab
 *	\brief grab struct member data by expression
 */

/*! \struct sg_type_info_t
 *  \brief basic type information used in structure grab
 */
struct sg_type_info_t
{
	char token;
	int bytes;
	int uatype;
	int uasubtype;
};

/*! \struct sg_expr_t
 *  \brief expression of struct grab
 */
struct sg_expr_t
{
	uint64_t plural:1;		//whether is array
	uint64_t bitfield:1;	//bit field
	uint64_t variable:1;	//
	uint64_t bitstart:7;
	uint64_t bitcount:7;
	uint64_t len:20;		//length of data want to grab. if plural is true, it represent the length of array
	char varexpr[128];
	sg_type_info_t ti;
};

/*!	\func sg_get_typeinfo
 *	\brief get basic type information for grab
 *	\param type what kind of type information
 *	\return success return type info else return NULL
 */
const sg_type_info_t* GRCALL sg_get_typeinfo(const char type);

/*!	\func sg_parse_array_expr
 *	\brief parse grab array expression
 *	\param expr represent array expression
 *	\param se store result after parsed
 *	\return success return 0 else return -1
 */
int GRCALL sg_parse_array_expr(const char *data, int len, const char *expr, sg_expr_t *se);

/*!	\func sg_parse_array_expr
 *	\brief parse bit field expression
 *	\param expr represent bit field expression
 *	\param se store result after parsed
 *	\return success return 0 else return -1
 */
int GRCALL sg_parse_bitfield_expr(const char *expr, sg_expr_t *se);

/*!	\func sg_parse_variable_expr
 *	\brief parse variable field expression
 *	\param expr represent variable expression
 *	\param se store result after parsed
 *	\return success return 0 else return -1
 */
int GRCALL sg_parse_variable_expr(const char *data, int len, const char *expr, sg_expr_t *se, ua_data_t *ud);

/*!	\func sg_parse_expr
 *	\brief parse a grab expression
 *	\param expr represent grab expression
 *	\param se store result after parsed
 *	\return success return 0 else return -1
 */
int GRCALL sg_parse_expr(const char *data, int len, const char *expr, sg_expr_t *se);

/*!	\func sg_calc_skip_len
 *	\brief calculate skip length by expression
 *	\param skipexpr represent skip expression
 *	\return success return 0 else return -1
 */
int GRCALL sg_calc_skip_len(const char *skipexpr);

/*!	\func sg_get_bitfiled_data
 *	\brief calculate skip length by expression
 *	\param buffer represent skip expression
 *	\param se store grab expression info
 *	\param out store data after grab
 *	\return success return 0 else return -1
 */
int GRCALL sg_grab_bitfiled_data(const char *buffer, sg_expr_t *se, void *out);

/*!	\func sg_grab_data
 *	\brief grab data from strcut by expression
 *	\param buffer represent stream struct data
 *	\param Alen length of data
 *	\param skipexpr skip expression
 *	\param expr grab expression
 *	\param ud store data after grab
 *	\return success return 0 else return -1
 */
int GRCALL sg_grab_data(const char *data, int Alen, const char *skipexpr, const char *expr, ua_data_t *ud);

#endif // TYPES_H
