#ifndef SCHEMA_HPP
#define SCHEMA_HPP

#include "element.hpp"
#include "token.hpp"

#define ELEM_ROOT_ID    1

#define SCHE_ROOT_NAME	"root"
#define SCHE_IN_NAME	"in"
#define SCHE_OUT_NAME	"out"

class schema
{
public:
	schema(const char *dbname=NULL);
	~schema();
	//parse schema from file. if error occurs. store error message to err
	bool parseFromFile(const char *file, char *err=NULL);
	//parse schema from string. if error occurs. store error message to err
	bool parseFromStr(const char *str, char *err=NULL);
	//return schema string
	const char* schestr();
	//build schema after schema parsed by parseXXX method
	bool build(char *err);
	//trigger schema
	bool isTrigger();
	//action schema
	bool isAction();

	bool isBuilded();
	//config and cor data
	//load module config data from database. database name is specified by the name in schema constructor
	bool loadCfgData();
	//save module config data to database
	bool dumpCfgData();
	//return module config data
	const ua_data_t* getCfgData();

	//valid correlation data by schema
	bool validCorData(const ua_data_t* d);
	//valid module config data by schema
	bool validCfgData(const ua_data_t *d);

	//set correlation data
	bool setCorData(const ua_data_t *d);
	//return correlation data
	const ua_data_t* getCorData();
	//set correlation node data by path
	bool setCorPathData(int path, const ua_data_t *data);
	//get correlation node data by path
	bool getCorPathData(int path, ua_data_t *data);

	//set module node config data by path
	bool setCfgPathData(int path, const ua_data_t *data);
	//get module node config data by path
	bool getCfgPathData(int path, ua_data_t *data);
	//handle module config data by action(etc add/modify/delete/clear)
	bool handleCfgData(const ua_data_invoke_t *invoke);

	//lookup correlation data by path. if strfmt is not NULL, it will filled with the string format of node data
	const ua_data_t* lookCorPathData(int path, const ua_data_t *data, char **strfmt = NULL);
	//lookup module config data by path. if strfmt is not NULL, it will filled with the string format of node data
	const ua_data_t* lookCfgPathData(int path, const ua_data_t *data, char **strfmt = NULL);

	//return module config root element
	element *cfgElement() { return cfg_; }
	//return correlation root element
	element *corElement() { return cor_; }

	//return module name
	const char* modName() { return modname_; }

	static bool canElementConvert(const element *left, const element* right);
	static bool canTypeConvert(int left, int right);
private:
	bool initEnvrion();
	void freeElem(element *e);
	int flipPath(int path);
	bool createCfgRootElement(token_object_t *token);
	bool createCorRootElement(token_object_t *cor);
	bool buildCfgElementTree(element *e, token_object_t *token);
	bool buildCorElementTree(element *e, token_object_t *token);
	int getDataBuiltin(const char *type);
	char *elem2str(element *elem, const ua_data_t *data);
	const ua_data_t* lookPathData(int path, element *e, const ua_data_t *data, char **strfmt = NULL);
	bool equalType(const ua_data_t *ud, int type);
private:
	char *name_;
	char *cfgdb_;
	char *cfgtmpdb_;
	char *modname_;
	char *modtype_;
	bool builded_;
	token tokener_;
	ua_data_t cordata_;  //cor data
	ua_data_t cfgdata_;  //module data
	element *cor_;
	element *cfg_;
};

#endif // SCHEMA_H
