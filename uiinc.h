#ifndef UIINC_H
#define UIINC_H

#include "sysinc.h"

/*! \enum ua_num_type_t
 *  \brief basic number type
 */
enum ua_num_type_t
{
	unt_i		= 0,
	unt_i8		= 1,
	unt_i16		= 2,
	unt_i32		= 3,
	unt_i64		= 4,
	unt_u8		= 5,
	unt_u16		= 6,
	unt_u32		= 7,
	unt_u64		= 8
};

/*! \enum ua_str_type_t
 *  \brief basic character type
 */
enum ua_str_type_t
{
	ust_char	= 1,
	ust_wchar	= 2
};

/*! \enum ua_data_type_t
 *  \brief built in data type in schema engine
 */
enum ua_data_type_t
{
	//basic data type
	udt_int     = 1,
	udt_str     = 2,
	udt_bool    = 3,
	udt_guid    = 4,
	udt_object  = 5,

	//the following is custom data type
	udt_camtree		= 10,		/*!< cameras in tree present */
	udt_camcomb		= 11,		/*!< cameras in combobox present */
	udt_tvws		= 12,
	udt_comptrg		= 13,
	udt_compact		= 14
};

/*! \enum ua_data_type_t
 *  \brief built in data action in schema engine
 */
enum ua_data_act_t
{
	uda_init    = 1,
	uda_add     = 2,
	uda_del     = 3,
	uda_mod     = 4,
	uda_clear   = 5
};

/*! \enum ua_data_type_t
 *  \brief built in data logical operation in schema engine
 */
enum ua_data_logic_t
{
	//for trigger
	udl_not	= 1,
	udl_and	= 2,
	udl_or	= 3,
	//for action
	udl_parallel = 1,
	udl_sequence = 2
};

/*!	\struct ua_binary_t
  \brief binary operations description
  */
struct ua_binary_t
{
	uint64_t llen:20;	/*!< length of left expression data */
	uint64_t rlen:20;	/*!< length of right expression data */
	uint64_t op:4;		/*!< binary logical operator. see ua_data_logic_t */
	uint64_t pad:20;
	void *ldata;		/*!< left expression data */
	void *rdata;		/*!< length of right express data */
};

/*!	\struct ua_unary_t
  \brief unary operations description
  */
struct ua_unary_t
{
	uint64_t dlen:32;	/*!< length of expression data */
	uint64_t op:4;      /*!< binary logical operator. see ua_data_logic_t */
	uint64_t pad:28;
	void *data;         /*!< expression data */
};

#define UA_HEAD_SIZE	(sizeof(ua_data_head_t))

/*! \struct ua_data_head_t
  \brief represent data header in schema engine
  */
struct ua_data_head_t
{
#define UA_HEAD_MAGIC	0xfbed
	uint64_t magic:16;		/*!< magic number in head. used by data validation */
	uint64_t dlen:20;		/*!< length of data */
	uint64_t dtype:7;		/*!< data type. see ua_data_type_t */
	uint64_t array:1;		/*!< whehter is array data */
	uint64_t attr:4;	    /*!< logical operator. see ua_data_logic_t. used by compound data type */
	uint64_t pad:16;		
	uint64_t nchildren:16;	/*!< length of children array */
	uint64_t fresh:1;		/*!< the flag set to 1 when add new data. it used by internal engine */
	uint64_t dsubtype:7;	/*!< the sub type of data type. such as int type include uint8_t, uint16_t etc.*/
	uint64_t pad2:40;
};

/*! \struct ua_data_t
  \brief represent a tree data structure used by schema engine
  */
struct ua_data_t
{
	ua_data_head_t head;	/*!< data head structure */
	void *data;				/*!< data payload */
	ua_data_t **children;	/*!< children array */
};

struct ua_data_invoke_t;

/*!	\func ua_data_callback_t
  \brief data callback by engine. engine will invoke the function to transfer data
  */
typedef bool (*ua_data_callback_t)(ua_data_invoke_t* Adata);

/*!	\struct ua_data_invoke_t
  \brief data action struture used by engine. engine will invoke the function when action data triggered
  */
struct ua_data_invoke_t
{
	uint64_t act:8;
	uint64_t path:20;
	uint64_t pad:36;
	ua_data_t *data;
	void *handle;
	ua_data_callback_t func;
};

/*!	\func ua_is_custom_dtype
 *	\brief whether the type is custom data type
 */
int GRCALL ua_is_custom_dtype(int type);

/*! \brief initialize ua_data_t structure. this will fill MAGIC number in data head
 *  \param Aud represent the data will be initialized
 */
void GRCALL ua_init(ua_data_t* Aud);

/*! \brief deinitilize ua_data_t structure
 *  \param Aud represent the data will be deinitialized
 */
void GRCALL ua_deinit(ua_data_t* Aud);

/*! \brief allocation children for ua_data_t structure
 *  \param Aud 'children' memebr will be stored allocation children. the caller should specify 'nchildren' in head
 */
void GRCALL ua_fork_children(ua_data_t *Aud);

/*! \brief resize children for ua_data_t structure
 *  \param Aud 'children' memebr will be stored allocation children. the caller should specify 'nchildren' in head
 *  \param Asize represent the number of children will be incremented
 */
void GRCALL ua_resize_children(ua_data_t *Aud, int Asize);

/*! \brief copy entire structue data recursively included children from param 'src' to param 'dst'
 *  \param dst 
 */
void GRCALL ua_copy(ua_data_t *dst, const ua_data_t *src);

/*! \brief update entire structue data recursively included children from param 'src' to param 'dst'
*/
void GRCALL ua_update(ua_data_t *dst, const ua_data_t *src);

/*! \brief copy data to ua_data_t's data member
 *  \param Aud represent will be receive data from Adata
 *  \param Adata represent data will be copied
 *  \param Alen represent the length of Adata
 */
void GRCALL ua_copy_data(ua_data_t *Aud,const void *Adata, int Alen);

/*! \brief add children to ua_data_t's children member
 *  \param Aud represent will store children
 *  \param Achild represent the chilren will be added
 */
void GRCALL ua_add_children(ua_data_t *Aud, const ua_data_t *Achild);

/*! \brief delete children from ua_data_t's children member
 *  \param Aud represent will be deleted from
 *  \param Achild represent the chilren will be deleted
 */
void GRCALL ua_del_children(ua_data_t *Aud, ua_data_t *Achild);

/*!	\func ua_get_int
 *	\brief get int array data from ua_data_t
 *	\param Aud store int type data will be got
 *	\param Aout output int value
 *	\return success return 0 else return -1
 */
int GRCALL ua_get_int(const ua_data_t *Aud, int *Aout);
int GRCALL ua_get_uint32(const ua_data_t *Aud, uint32_t *Aout);
int GRCALL ua_get_uint64(const ua_data_t *Aud, uint64_t *Aout);

/*!	\func ua_get_bool
 *	\brief get bool value data from ua_data_t
 *	\param Aud store int type data will be got
 *	\param Aout output int value
 *	\return success return 0 else return -1
 */
int GRCALL ua_get_bool(const ua_data_t *Aud, int *Aout);

/*!	\func ua_get_str
 *	\brief get string type data from ua_data_t
 *	\param Aud store string type data will be got
 *	\return success a pointer that to string else return NULL. if success. the pointer will be free by ua_free_str
 */
char* GRCALL ua_get_str(const ua_data_t *Aud);

/*!	\func ua_get_guid
 *	\brief get guid from ua_data_t
 *	\param Aud store guid type data will be got
 *	\param Aout store the output guid data
 *	\return success return 0 else return -1
 */
int GRCALL ua_get_guid(const ua_data_t *Aud, guid_t *Aout);

/*!	\func ua_get_int_array
 *	\brief get int array data from ua_data_t
 *	\param Aud store int array type data will be got
 *	\param Alen length of array
 *	\return success a pointer that point to array else return NULL. if success. the pointer will be free by ua_free_int_array
 */
int* GRCALL ua_get_int_array(const ua_data_t *Aud, int *Alen);
void* GRCALL ua_get_num_array(const ua_data_t *Aud, int AsubType, int *Alen);
/*!	\func ua_get_str_array
 *	\brief get guid string data from ua_data_t
 *	\param Aud store int array type data will be got
 *	\param Alen length of array
 *	\return success a pointer that point to array else return NULL. if success. the pointer will be free by ua_free_str_array
 */
char** GRCALL ua_get_str_array(const ua_data_t *Aud, int *Alen);

/*!	\func ua_get_guid_array
 *	\brief get guid array data from ua_data_t
 *	\param Aud store guid array type data will be got
 *	\param Alen length of array
 *	\return success a pointer that point to array else return NULL. if success. the pointer will be free by ua_free_guid_array
 */
guid_t* GRCALL ua_get_guid_array(const ua_data_t *Aud, int *Alen);

/*!	\func ua_set_int
 *	\brief store int type data to ua_data_t
 *	\param Anumber number will be store to ua_data_t
 *	\param Aud store int number
 *	\return success 0 else return -1
 */
int GRCALL ua_set_int(int Anumber, ua_data_t *Aud);
int GRCALL ua_set_uint32(int Anumber, ua_data_t *Aud);
int GRCALL ua_set_uint64(int Anumber, ua_data_t *Aud);

/*!	\func ua_set_bool
 *	\brief store bool type data to ua_data_t
 *	\param Anumber bool will be store to ua_data_t
 *	\param Aud store bool value
 *	\return success 0 else return -1
 */
int GRCALL ua_set_bool(int Abool, ua_data_t *Aud);

/*!	\func ua_set_str
 *	\brief store string type data to ua_data_t
 *	\param Astr string type data
 *	\param Aud store string type data
 *	\return success 0 else return -1
 */
int GRCALL ua_set_str(const char *Astr, ua_data_t *Aud);

/*!	\func ua_set_guid
 *	\brief store guid type data to ua_data_t
 *	\param Astr guid type data
 *	\param Aud store guid type data
 *	\return success 0 else return -1
 */
int GRCALL ua_set_guid(const char *str, ua_data_t *Aud);

/*!	\func ua_set_int_array
 *	\brief store int array type data to ua_data_t
 *	\param Anumbers int array data
 *	\param Alen length of array
 *	\param Aud store int array type data
 *	\return success 0 else return -1
 */
int GRCALL ua_set_int_array(const int *Anumbers, int Alen, ua_data_t *Aud);
int GRCALL ua_set_num_array(const void *Anumbers, int Alen, int AsubType, ua_data_t *Aud);

/*!	\func ua_set_str_array
 *	\brief store int array type data to ua_data_t
 *	\param Astr string array data
 *	\param Alen length of array
 *	\param Aud store Astr array type data
 *	\return success 0 else return -1
 */
int GRCALL ua_set_str_array(const char **Astr, int Alen, ua_data_t *Aud);

/*!	\func ua_set_guid_array
 *	\brief store guid array type data to ua_data_t
 *	\param Aids guid array data
 *	\param Alen length of array
 *	\param Aud store guid array type data
 *	\return success 0 else return -1
 */
int GRCALL ua_set_guid_array(const guid_t *Aids, int Alen, ua_data_t *Aud);

/*!	\func ua_free_str
 *	\brief free string data allocated by  ua_get_str
 *	\param Astr the string pointer will be destroyed
 */
void GRCALL ua_free_str(char *Astr);

/*!	\func ua_free_int_array
 *	\brief free int array data allocated by  ua_get_int_array
 *	\param Anumbers the int pointer will be destroyed
 */
void GRCALL ua_free_int_array(int *Anumbers);
void GRCALL ua_free_num_array(void *Anumbers);

/*!	\func ua_free_str_array
 *	\brief free string array data allocated by  ua_get_str_array
 *	\param Astr the string array pointer will be destroyed
 */
void GRCALL ua_free_str_array(char **Astr, int Alen);

/*!	\func ua_free_guid_array
 *	\brief free string array data allocated by  ua_get_guid_array
 *	\param Aguid the guid array pointer will be destroyed
 */
void GRCALL ua_free_guid_array(guid_t *Aguid);

/*! \brief expand ua_data_t structure to a block of continuous memory
 *  \param Adata represent the data will be expanded
 *  \param Amem represent will be a block of continuous memory
 *  \return error will reutrn -1 else return 0;
 */
int GRCALL ua_expand_data(const ua_data_t* Adata, void **Amem, int *Alen);

/*! \brief collapse a block of continuous memory to ua_data_t structure
 *  \param Amem represent a block of continuous memory  will be collpased
 *  \param Adata represent a structure data collpased to
 *  \return error will reutrn -1 else return 0;
 */
int GRCALL ua_collapse_data(void *Amem, int *Alen,  ua_data_t* Adata);

/*! \brief destroy memory allocated from  ua_expand_data
*/
void GRCALL ua_de_expand(void *Amem);

/*! \brief destroy data allocated from  ua_collapse_data
*/
void GRCALL ua_de_collapse(ua_data_t *Adata);

/*! \brief convert binary operation data to compound data type
 *  \param Abinary represent binary operation data
 *  \param Adata represent compound data after converted
 *  \sucess return 0 else return -1
 */
int ua_binary2compound(const ua_binary_t *Abinary, ua_data_t* Adata);

/*! \brief is contrary to ua_binary2compound
*/
int ua_compound2binary(const ua_data_t* Adata, ua_binary_t *Abinary);

/*! \brief convert unary operation data to compound data type
 *  \param Aunary represent binary operationi data
 *  \param Adata represent compound data after converted
 *  \sucess return 0 else return -1
 */
int ua_unary2compound(const ua_unary_t *Aunary, ua_data_t *Adata);

/*! \brief is contrary to ua_unary2compound
*/
int ua_compound2unary(const ua_data_t *Adata, ua_unary_t *Aunary);

/*! \function ua_tostring
 *	\brief convert ua_data_t to strig. the return string must free by ua_free_str
 */
char* ua_tostring(const ua_data_t *data);

struct ua_ctrl_invoke_t;

/*!	\func ua_ctrl_callback_t
 *	\brief external control callback. engine will invoke the function to manipulate external control
 */
typedef bool (*ua_ctrl_callback_t)(ua_ctrl_invoke_t *handle);

typedef void (*ua_ctrl_locale_t)(ua_ctrl_invoke_t*handle, const char *locale);

/*! \enum ua_ctrl_act_t
 *	\brief control action
 */
enum ua_ctrl_act_t
{
	uca_create	= 1,
	uca_set		= 2,
	uca_get		= 3,
    uca_exec	= 4
};

/*! \struct ua_ctrl_invoke_t
 *	\brief engine will the struct to manipulate external control
 */
struct ua_ctrl_invoke_t
{
    uint64_t type:8;             /*!< what kind of control(ua_ctrl_type_t) the caller should be created */
    uint64_t act:16;             /*!< control action. see ua_ctrl_act_t */
	uint64_t pad:40;
    void *caller;                /*!< external caller */
    void *data;                  /*!< external caller user data */
    void *parent;                /*!< the callber will paint control in this window handle */
    void *handle;                /*!< created window handle. caller will fill this member with new created window handle, the handle will be used to get/set data */
    ua_data_t* ud;               /*!< user data */
    ua_ctrl_callback_t func;     /*!< caller should register control callback function, engine will invoke the callback to act (see ua_ctrl_act_t) control */
    ua_ctrl_locale_t setlocale;  /*!< set control local */
};

/*!	\struct ua_convert_elem_t
 *	\breif correlation convert element 
 */
struct ua_convert_rule_t
{
	guid_t id;
	guid_t gid;
	uint32_t frompath;
	uint32_t topath;
	uint64_t convert_type:8;
	uint64_t istrg:1;
	uint64_t pad:55;
};

int ua_query_lang_index(const char *code);
/*! \func 
 *  \breif get current language index
 */
int ua_get_lang_index();
#endif // TYPES_H
