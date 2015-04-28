#ifndef ELEMENT_H
#define ELEMENT_H

#include "sysinc.hpp"
#include "uiinc.h"

enum elem_event_act_t
{
	eea_hide    = 1,
	eea_disable = 2
};

enum elem_event_type_t
{
	eet_path    = 1,
	eet_cascade = 2
};

struct elem_data_t
{
	uint64_t ivalue;      /*!< effect when data type is number */
	char *svalue;         /*!< effect when data type is string */

	struct{
		int act;
		int type;         /*!< reference elem_event_type_t */
		int npath;        /*!< effect when type is eet_path */
		int *pathset;     /*!< effect when type is eet_path */
	}event;
};

struct elem_dataset_t
{
	uint64_t min;    /*!< minimun value. used when data type is int*/
	uint64_t max;    /*!< maximun value.used when data type is int*/
	uint64_t defnum; /*!< default number value. used when data type is int */
	char *defstr;    /*!< default string value. used when data type is string */
	int count;       /*!< count of data set */
	elem_data_t *dataset;
};

struct elem_attr_t
{
	int id;			/*!< attribute id */
	char *name;		/*!< attribute name */
	char *type;		/*!< attribute type in string format */
	char *expr;		/*!< used for fetch data member in structure data */
	bool key;		/*!< whether is key filed when type is a object. key is used identity data */
	bool txt;		/*!< whether is a text filed when type is a object. txt field is used in ui */
	bool plural;	/*!< whether the data is plural */
	bool hide;		/*!< whether to hide this in ui */
	bool cor;		/*!< whether to particulate cor */
	char *format;   /*!< string format. used in data type is string */

	elem_dataset_t *dataset; /*!< element data set */
};

struct elem_act_t
{
	char *name;
	char *act;
};

class element
{
public:
	element();
	~element();
	//lookup element by path
	element* lookup(int path);
	//set tree depth
	int childDepth() { return path * 10; }
	//handle data for element
	void handleData(ua_data_t *dop, ua_data_t *d, int path, int act);
	//compare if the data is equal
	bool hitData(const ua_data_t *src, const ua_data_t *dst);
	//copy data from src to dst
	void copyData(ua_data_t *dst, const ua_data_t *src);
	//get text data from data. used in ui(etc comobobox text). specified by 'txt' attribute in schema
	void* getTxtData(const ua_data_t *d);
	//get key data from data. used in ui(etc comobobox data). specified by 'key' attribute in schema
	int getKeyData(const ua_data_t *d, void **pkey);
	//bool isroot() { return path == 1; }
	void getLangText(const char *text, char *out, const char *lang=NULL);
	//get display attribute name. used in upper UI
	void getDisplayName(char *name, const char *lang=NULL);
private:
	//data operator
	void addData(ua_data_t *add, ua_data_t *d);
	void delData(ua_data_t *del, ua_data_t *d);
	void modData(ua_data_t *mod, ua_data_t *d);
	void clearData(ua_data_t *clear, ua_data_t *d);
public:
	int path;
	int type;
	elem_attr_t *attr;
	vector<element*> children;
	vector<elem_act_t*> acts;
};
#endif // ELEMENT_H
