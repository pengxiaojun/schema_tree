#ifndef TOKEN_H
#define TOKEN_H

#include "sysinc.hpp"
#include "element.hpp"

struct json_object;
struct token_object_t
{
    char *name;
    int nattr;
    int nact;
	char *modname;
	char *modtype;
    elem_attr_t *attrs;
    elem_act_t *acts;
};
class token
{
public:
    token();
    ~token();
    bool parseFromFile(const char *file, char *err);
    bool parseFromStr(const char *str, char *err);
    token_object_t* findToken(const char *name);
	const char *schestr() { return schestr_; }
private:
    //parse token
    bool parseToken(token_object_t *token, json_object *jobj, char *err);
	bool parseElement(token_object_t *token, json_object *jobj, char *err);
	bool parseAttr(elem_attr_t *attr, json_object *jobj, char *err);
	bool parseAct(elem_act_t *act, json_object *jobj, char *err);
	bool parseAttrData(elem_attr_t *attr, json_object *jobj, char *err);
	bool parseDataItem(elem_data_t *data, json_object *jobj, char *err);
	void freeAttr(elem_attr_t *attr);
	void freeAct(elem_act_t *act);
    void freeToken(token_object_t *token);
private:
	char schestr_[4096];
    int ntoken_;
    token_object_t *tokens_;
};
#endif // TOKEN_H
