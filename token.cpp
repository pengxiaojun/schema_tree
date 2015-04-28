#include "token.hpp"
#include "schema.hpp"
#include <json-c/json.h>

#define MAX_TOKENS  64

const char *event_act[]  = {"", "hide", "disable"};
const char *event_type[] = {"", "path", "cascade"};

token::token()
{
	memset(schestr_, 0, sizeof(schestr_));
	ntoken_ = 0;
	tokens_ = (token_object_t*)calloc(MAX_TOKENS, sizeof(token_object_t));
}

token::~token()
{
	for (int i = 0; i<ntoken_; ++i)
		freeToken(&tokens_[i]);
	free(tokens_);
}

bool token::parseFromFile(const char *file, char *err)
{
	FILE *fp = fopen(file, "r");
	if (!fp)
	{
		sprintf(err, "Not exist schema file %s", file);
		return false;
	}

	char *content = schestr_;
	char line[265];
	char *str;
	while ( (str=fgets(line, 256, fp)) != NULL)
	{
		//trim prefix space
		char *endptr = str + strlen(str);
		while (isspace(*str))
		{
			str++;
		}
		//trim suffix space
		while (isspace(*endptr))
		{
			*endptr = '\0';
		}
		strcpy(content, line);
		content += strlen(line);
	}
	fclose(fp);
	return parseFromStr(schestr_, err);
}

bool token::parseFromStr(const char *str, char *err)
{
	char lbrace = '{';
	char rbrace = '}';
	char c;
	stack<char> s;
	char json_obj[512] = {0};
	char *pjson = json_obj;
	bool obj_start = false;
	strcpy(schestr_, str);

	while ( (c = *str++))
	{
		if (isspace(c) && c != ' ') continue;
		if (c == lbrace)
		{
			obj_start = true;
			s.push(c);
		}
		else if (c == rbrace)
		{
			s.pop();
		}
		if (obj_start) *pjson++ = c;

		if (s.empty())
		{
			obj_start = false;
			json_object *jobj;
			json_tokener_error jerr;
			jobj = json_tokener_parse_verbose(json_obj, &jerr);
			if (jerr != json_tokener_success)
			{
				strcpy(err, json_tokener_error_desc(jerr));
				return false;
			}

			token_object_t *token = &tokens_[ntoken_++];
			if (!parseToken(token, jobj, err))
			{
				return false;
			}
			memset(json_obj, 0, sizeof(json_obj));
			pjson = json_obj;
		}
	}
	return true;
}

bool token::parseToken(token_object_t *token, json_object *jobj, char *err)
{
	json_object *jval;
	//parse name
	if (json_object_object_get_ex(jobj, "name", &jval) &&
		json_object_get_type(jval) == json_type_string)
	{
		token->name = strdup(json_object_get_string(jval));
	}
	else
	{
		strcpy(err, "token no exit key 'name'");
		return false;
	}
	return parseElement(token, jobj, err);
}

bool token::parseElement(token_object_t *token, json_object *jobj, char *err)
{
	json_object *jval;
	//parse attr
	if (json_object_object_get_ex(jobj, "attr", &jval) &&
		json_object_get_type(jval) == json_type_array)
	{
		int len = json_object_array_length(jval);
		token->nattr = len;
		token->attrs = (elem_attr_t*)calloc(len, sizeof(elem_attr_t));
		for (int i = 0; i<len; ++i)
		{
			elem_attr_t *attr = &token->attrs[i];
			json_object *jattr = json_object_array_get_idx(jval, i);
			attr->id = i;
			if (!parseAttr(attr, jattr, err))
			{
				return false;
			}
		}
	}
	else
	{
		if (err) strcpy(err, "missing key 'attr'");
		return false;
	}

	if (json_object_object_get_ex(jobj, "action", &jval) &&
		json_object_get_type(jval) == json_type_array)
	{
		int len = json_object_array_length(jval);
		token->nact = len;
		token->acts = (elem_act_t*)calloc(token->nact, sizeof(elem_act_t));

		for (int i = 0; i<len; ++i)
		{
			elem_act_t *act = &token->acts[i];
			json_object *jattr = json_object_array_get_idx(jval, i);
			if (!parseAct(act, jattr, err))
			{
				return false;
			}
		}
	}
	if (json_object_object_get_ex(jobj, "modname", &jval) &&
		json_object_get_type(jval) == json_type_string)
	{
		token->modname = strdup(json_object_get_string(jval));
	}
	if (json_object_object_get_ex(jobj, "modtype", &jval) &&
		json_object_get_type(jval) == json_type_string)
	{
		token->modtype = strdup(json_object_get_string(jval));
	}
	return true;
}

bool token::parseAttr(elem_attr_t *attr, json_object *jattr, char *err)
{
	json_object *jval;
	if (json_object_object_get_ex(jattr, "name", &jval) &&
		json_object_get_type(jval) == json_type_string)
	{
		attr->name = strdup(json_object_get_string(jval));
	}
	else
	{
		if (err) strcpy(err, "missing key 'attr.name'");
		return false;
	}

	if (json_object_object_get_ex(jattr, "type", &jval) &&
		json_object_get_type(jval) == json_type_string)
	{
		attr->type = strdup(json_object_get_string(jval));
	}
	else
	{
		if (err) strcpy(err, "missing key 'attr.type'");
		return false;
	}
	if (json_object_object_get_ex(jattr, "plural", &jval) &&
		json_object_get_type(jval) == json_type_boolean)
	{
		attr->plural = json_object_get_boolean(jval) ? true : false;
	}
	if (json_object_object_get_ex(jattr, "hide", &jval) &&
		json_object_get_type(jval) == json_type_boolean)
	{
		attr->hide = json_object_get_boolean(jval) ? true : false;
	}
	if (json_object_object_get_ex(jattr, "key", &jval) &&
		json_object_get_type(jval) == json_type_boolean)
	{
		attr->key = json_object_get_boolean(jval) ? true : false;
	}
	if (json_object_object_get_ex(jattr, "txt", &jval) &&
		json_object_get_type(jval) == json_type_boolean)
	{
		attr->txt = json_object_get_boolean(jval) ? true : false;
	}
	if (json_object_object_get_ex(jattr, "cor", &jval) &&
		json_object_get_type(jval) == json_type_boolean)
	{
		attr->cor = json_object_get_boolean(jval) ? true : false;
	}
	else
	{
		attr->cor = true;	//all element particulate cor
	}
	if (json_object_object_get_ex(jattr, "format", &jval) &&
		json_object_get_type(jval) == json_type_string)
	{
		attr->format = strdup(json_object_get_string(jval));
	}
	//parse data
	if (json_object_object_get_ex(jattr, "data", &jval) &&
		json_object_get_type(jval) == json_type_object)
	{
		if (!parseAttrData(attr, jval, err))
		{
			return false;
		}
	}
	return true;
}

bool token::parseAct(elem_act_t *act, json_object *jattr, char *err)
{
	json_object *jval;
	//parse attr.name
	if (json_object_object_get_ex(jattr, "name", &jval) &&
		json_object_get_type(jval) == json_type_string)
	{
		act->name = strdup(json_object_get_string(jval));
	}
	else
	{
		if (err) strcpy(err, "missing key 'act.name'");
		return false;
	}
	if (json_object_object_get_ex(jattr, "act", &jval) &&
		json_object_get_type(jval) == json_type_string)
	{
		act->act = strdup(json_object_get_string(jval));
	}
	else
	{
		if (err) strcpy(err, "missing key 'act.act'");
		return false;
	}
	return true;
}

bool token::parseAttrData(elem_attr_t *attr, json_object *jobj, char *err)
{
	attr->dataset = (elem_dataset_t*)calloc(1, sizeof(elem_dataset_t));
	json_object *jval;
	if (json_object_object_get_ex(jobj, "min", &jval) &&
		json_object_get_type(jval) == json_type_int)
	{
		attr->dataset->min = json_object_get_int64(jval);
	}
	if (json_object_object_get_ex(jobj, "max", &jval) &&
		json_object_get_type(jval) == json_type_int)
	{
		attr->dataset->max = json_object_get_int64(jval);
	}
	if (json_object_object_get_ex(jobj, "def", &jval) &&
		json_object_get_type(jval) == json_type_int)
	{
		attr->dataset->defnum = json_object_get_int64(jval);
	}
	if (json_object_object_get_ex(jobj, "def", &jval) &&
		json_object_get_type(jval) == json_type_string)
	{
		attr->dataset->defstr = strdup(json_object_get_string(jval));
	}

	if (json_object_object_get_ex(jobj, "set", &jval) &&
		json_object_get_type(jval) == json_type_array)
	{
		int len = json_object_array_length(jval);
		attr->dataset->count = len;
		attr->dataset->dataset = (elem_data_t*)calloc(len, sizeof(elem_data_t));

		for (int i = 0; i<len; ++i)
		{
			elem_data_t *data = &attr->dataset->dataset[i];
			json_object *jattr = json_object_array_get_idx(jval, i);
			if (!parseDataItem(data, jattr, err))
			{
				return false;
			}
		}
	}
	return true;
}

bool token::parseDataItem(elem_data_t *data, json_object *jobj, char *err)
{
	json_object *jval;
	if (json_object_object_get_ex(jobj, "value", &jval) &&
		json_object_get_type(jval) == json_type_int)
	{
		data->ivalue = json_object_get_int64(jval);
	}
	if (json_object_object_get_ex(jobj, "value", &jval) &&
		json_object_get_type(jval) == json_type_string)
	{
		data->svalue = strdup(json_object_get_string(jval));
	}
	if (json_object_object_get_ex(jobj, "event", &jval) &&
		json_object_get_type(jval) == json_type_object)
	{
        json_object *jevent;
        if (json_object_object_get_ex(jval, "act", &jevent) &&
            json_object_get_type(jevent) == json_type_string)
		{
            const char *act = json_object_get_string(jevent);
			for (size_t i = 0; i<sizeof(event_act)/sizeof(event_act[0]); ++i)
			{
				if (strcmp(event_act[i], act) == 0)
				{
					data->event.act = i;
					break;
				}
			}
		}

        if (json_object_object_get_ex(jval, "type", &jevent) &&
            json_object_get_type(jevent) == json_type_string)
		{
            const char *type = json_object_get_string(jevent);
			for (size_t i = 0; i<sizeof(event_type)/sizeof(event_type[0]); ++i)
			{
				if (strcmp(event_type[i], type) == 0)
				{
					data->event.type = i;
					break;
				}
			}
		}
        if (json_object_object_get_ex(jval, "target", &jevent) &&
            json_object_get_type(jevent) == json_type_array)
        {
            int len = json_object_array_length(jevent);
            data->event.npath = len;
            data->event.pathset = (int*)calloc(len, sizeof(int));

            for (int i = 0; i<len; ++i)
            {
                json_object *jattr = json_object_array_get_idx(jevent, i);
                if (json_object_get_type(jattr) == json_type_int)
                {
                    data->event.pathset[i] = json_object_get_int(jattr);
                }
                else
                {
                    if (err) strcpy(err, "Invalid data path item");
                    return false;
                }
            }
        }
    }//end event

	return true;
}

void token::freeToken(token_object_t *token)
{
	free(token->name);
	if (token->modname) free(token->modname);
	if (token->modtype) free(token->modtype);
	int i;
	for (i = 0; i<token->nattr; ++i)
	{
		freeAttr(&token->attrs[i]);
	}
	for (i = 0; i<token->nact; ++i)
	{
		freeAct(&token->acts[i]);
	}
	if (token->attrs) free(token->attrs);
	if (token->acts) free(token->acts);
}

void token::freeAttr(elem_attr_t *attr)
{
	if (!attr) return;
	if (attr->name) free(attr->name);
	if (attr->type) free(attr->type);
	if (attr->expr) free(attr->expr);
	if (attr->format) free(attr->format);

	if (attr->dataset)
	{
		elem_dataset_t *dataset = attr->dataset;
		if (dataset->defstr)
			free(dataset->defstr);
		int i;
		for (i = 0; i<dataset->count; ++i)
		{
			elem_data_t *data = &dataset->dataset[i];
			if (data->svalue) free(data->svalue);
			if (data->event.type == eet_path && data->event.npath > 0)
			{
				free(data->event.pathset);
			}
		}
		free(attr->dataset);
	}
}

void token::freeAct(elem_act_t *act)
{
	if (!act) return;
	if (act->act) free(act->act);
	if (act->name) free(act->name);
}

token_object_t* token::findToken(const char *name)
{
	for (int i = 0; i<ntoken_; ++i)
	{
		token_object_t *t = &tokens_[i];
		if (!strcmp(t->name, name))
		{
			return t;
		}
	}
	return NULL;
}

