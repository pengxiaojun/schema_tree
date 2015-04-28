#include "schema.hpp"
#include "element.hpp"
#include "uiinc.h"

#define CFG_DATA_DB     ".db"
#define CFG_TMP_DATA_DB ".db.tmp"
#define SCHEMA_DIR      "schemas"


#ifdef WIN32
#define PATH_SEP        "\\"
#else
#define PATH_SEP        "/"
#endif

static char *data_buildin_type[] =
{
	"",
	"int",
	"string",
	"bool",
	"guid",
	"object",
	"",
	"",
	"",
	"",
	"camtree",
	"camcomb",
	"tvws",
	"comptrg",
	"compact"
};

int gr_guid_from_str(const char* Astr, guid_t* Aid)
{
	int slen = strlen(Astr);
	char str[40] = { 0 };
	if (slen == 36)
		strncpy(str, Astr, 36);
	else if (slen == 38)
		strncpy(str, Astr + 1, 36);
	else
		return -1;
	int ret = sscanf(str, "%08x-%04hx-%04hx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx"
		, &Aid->uuid.data1
		, &Aid->uuid.data2
		, &Aid->uuid.data3
		, &Aid->uuid.data4[0], &Aid->uuid.data4[1]
		, &Aid->uuid.data5[0], &Aid->uuid.data5[1], &Aid->uuid.data5[2]
		, &Aid->uuid.data5[3], &Aid->uuid.data5[4], &Aid->uuid.data5[5]
		);
	Aid->uuid.data1 = htonl(Aid->uuid.data1);
	Aid->uuid.data2 = htons(Aid->uuid.data2);
	Aid->uuid.data3 = htons(Aid->uuid.data3);
	return ret == 11 ? 0 : -1;
}

char* gr_guid_to_str(const guid_t* Aid, char* Astr)
{
	sprintf(Astr, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"
		, ntohl(Aid->uuid.data1)
		, ntohs(Aid->uuid.data2)
		, ntohs(Aid->uuid.data3)
		, Aid->uuid.data4[0], Aid->uuid.data4[1]
		, Aid->uuid.data5[0], Aid->uuid.data5[1], Aid->uuid.data5[2]
		, Aid->uuid.data5[3], Aid->uuid.data5[4], Aid->uuid.data5[5]
		);
	return Astr;
}
schema::schema(const char *dbname):
	name_(NULL),
	cfgdb_(NULL),
	cfgtmpdb_(NULL),
	modname_(NULL),
	modtype_(NULL),
	builded_(false),
	cor_(NULL),
	cfg_(NULL)
{
	if (dbname && strlen(dbname) > 0){
		name_ = strdup(dbname);
		char cfg_db_name[128] = {0};
		char cfg_tmp_db_name[128] = {0};
		strcpy(cfg_db_name, name_);
		strcat(cfg_db_name, CFG_DATA_DB);
		strcpy(cfg_tmp_db_name, name_);
		strcat(cfg_tmp_db_name, CFG_TMP_DATA_DB);

		cfgdb_ = strdup(cfg_db_name);
		cfgtmpdb_ = strdup(cfg_tmp_db_name);
	}
	ua_init(&cordata_);
	ua_init(&cfgdata_);
}

schema::~schema()
{
	if (name_) free(name_);
	if (cfgdb_) free(cfgdb_);
	if (cfgtmpdb_) free(cfgtmpdb_);
	if (cor_)
	{
		if (cor_->attr) free(cor_->attr);
		freeElem(cor_);
	}
	if (cfg_)
	{
		if (cfg_->attr) free(cfg_->attr);
		freeElem(cfg_);
	}
}

/* support create only one level directory */
bool schema::initEnvrion()
{
#ifdef WIN32
	BOOL ret = CreateDirectoryA((LPSTR)SCHEMA_DIR, NULL);
	if (!ret){
		DWORD rc = GetLastError();

		if( rc == ERROR_ALREADY_EXISTS)
		{
			return true;             
		}
		return false;
	}
#else
	struct stat st;
	int ret = stat(SCHEMA_DIR, &st);
	if(ret != -1){
		ret = S_ISDIR(st.st_mode);
		if(ret)
			return true;
	}
	ret = mkdir(SCHEMA_DIR, S_IRWXU);
	if(ret == -1){
		return false;
	}
#endif
	//get language
	return true;
}

void schema::freeElem(element *e)
{
	for (size_t i = 0; i<e->children.size(); ++i)
	{
		freeElem(e->children[i]);
	}
	delete e;
}

bool schema::parseFromFile(const char *file, char *err)
{	
	if (file == NULL)
	{
		return false;
	}
	char error[1024] = {0};
	bool ret= tokener_.parseFromFile(file, error);
	if (!ret && err){
		strcpy(err, error);
    }
	return ret;
}

bool schema::parseFromStr(const char *str, char *err)
{
	if (str == NULL)
	{
		return false;
	}
	char error[1024] = {0};
	bool ret = tokener_.parseFromStr(str, err);
	if (!ret && err){
		strcpy(err, error);
	}
	return ret;
}

const char* schema::schestr()
{
	return tokener_.schestr();
}

bool schema::createCorRootElement(token_object_t *token)
{
	cor_ = new element();
	cor_->path = ELEM_ROOT_ID;
	cor_->type = udt_object;
	cor_->attr = (elem_attr_t*)calloc(1, sizeof(elem_attr_t));
	cor_->attr->name = token->name;
	return true;
}

bool schema::createCfgRootElement(token_object_t *token)
{
	cfg_= new element();
	cfg_->path = ELEM_ROOT_ID;
	cfg_->type = udt_object;
	cfg_->attr = (elem_attr_t*)calloc(1, sizeof(elem_attr_t));
	cfg_->attr->name = token->name;

	modname_ = token->modname;
	modtype_ = token->modtype;
	return true;
}

bool schema::build(char *err)
{
	//handle root
	token_object_t *t = tokener_.findToken(SCHE_ROOT_NAME);
	if (!t){
		if (err) strcpy(err, "Not exist element named 'root'");
		return false;
	}
	createCfgRootElement(t);
	createCorRootElement(t);
	if (!buildCfgElementTree(cfg_, t))
	{
		if (err) strcpy(err, "Parse element tree error. Schema contains invalid token type");
		return false;
	}
	if (!buildCorElementTree(cor_, t))
	{
		if (err) strcpy(err, "Parse cor tree error. Schema contains invalid token type");
		return false;
	}

	cfgdata_.head.nchildren = cfg_->children.size();

	//load schema database. some schema may be have no schema database
	if (!loadCfgData()){
		if (err) strcpy(err, "Load schema config database error");
		ua_fork_children(&cfgdata_);
	}
	if (cfg_->children.size() > 0 && (cfgdata_.head.nchildren == 0 || cfgdata_.children == NULL))
	{
		cfgdata_.head.nchildren = cfg_->children.size();
		ua_fork_children(&cfgdata_);
	}

	cordata_.head.nchildren = cor_->children.size();
	ua_fork_children(&cordata_);
	builded_ = true;
	return true;
}

bool schema::isTrigger()
{
	return strcmp(modtype_, SCHE_IN_NAME) == 0;
}

bool schema::isAction()
{
	return strcmp(modtype_, SCHE_OUT_NAME) == 0;
}

bool schema::isBuilded()
{
	return builded_;
}

bool schema::loadCfgData()
{
	if (!cfgdb_)
		return false;
	FILE *fp = fopen(cfgdb_, "rb");	
	if (!fp){
		return false;		//some schema do not need database
	}
	fseek(fp, 0, SEEK_END);
	int dlen = ftell(fp);
	rewind(fp);

	char *buf = (char*)calloc(1, dlen);
	int ret = fread(buf, 1, dlen, fp);
	if (ret != dlen){
		return false;
	}
	fclose(fp);
	ua_collapse_data(buf, &dlen, &cfgdata_);
	if (dlen == 0)
		return false;
	return true;
}

bool schema::dumpCfgData()
{
	if (!cfgtmpdb_ || !cfgdb_)
		return false;

	//dump to tmp db	
	FILE *fp = fopen(cfgtmpdb_, "wb");
	if (!fp)
	{
		return false;
	}
	void *pdata = NULL;
	int dlen = 0;
	ua_expand_data(&cfgdata_, &pdata, &dlen);	

	int ret = fwrite(pdata, 1, dlen, fp);
	if (ret != dlen){
		fclose(fp);
		ua_de_expand(pdata);
		return false;
	}
	fclose(fp);
	ua_de_expand(pdata);
	//rename to db
#ifdef WIN32
	MoveFileEx(cfgtmpdb_, cfgdb_, MOVEFILE_REPLACE_EXISTING);
#else
	struct stat st;
	ret = stat(cfgtmpdb_, &st);
	if(ret == -1){
		return false;
	}
	if(st.st_size > 0)
	{
		ret = rename(cfgtmpdb_, cfgdb_); 
		if(ret == -1)
		{
			return false;
		}
	}
#endif
	return true;
}

bool schema::validCorData(const ua_data_t *d)
{
	if (!cor_){
		return false;
	}
	if (!d){
		return false;
	}

	if (d->head.nchildren != cor_->children.size())
	{
		return false;
	}

	for (uint32_t i = 0; i<d->head.nchildren; ++i)
	{
		const element *child = cor_->children[i];
		int type = child->type;
		if (!type){
			const element *e = cfg_->lookup(child->path);
			if (!e) return false;
			type = e->type;
		}
		const ua_data_t *sub = d->children[i];
		if (type != sub->head.dtype)
		{
			return false;
		}
	}
	return true;
}

bool schema::buildCfgElementTree(element *e, token_object_t *token)
{
	//build children
	for (int i = 0; i<token->nact; ++i)
	{
		e->acts.push_back(&token->acts[i]);
	}
	for (int i = 0; i<token->nattr; ++i)
	{
		elem_attr_t *attr = &token->attrs[i];
		element *child = new element();
		child->attr = attr;
		child->path = e->childDepth() + (i+1);
		child->type = getDataBuiltin(attr->type);
		/*if (!child->type)
		  {
		  token_object_t *t = tokener_.findToken(attr->type);
		  if (!t) return false;    //invalid type
		  child->type = udt_object;   //have children
		  buildCfgElementTree(child, t);
		  }*/

		token_object_t *t = tokener_.findToken(attr->type);
		if (t)
		{
			if (!child->type) child->type = udt_object;
			buildCfgElementTree(child, t);
		}
		e->children.push_back(child);
	}
	return true;
}

bool schema::buildCorElementTree(element *e, token_object_t *token)
{
	for (int i = 0; i<token->nattr; ++i)
	{
		elem_attr_t *attr = &token->attrs[i];
		if (!attr->cor) continue;
		element *child = new element();
		child->attr = attr;
		child->path = e->childDepth() + (i+1);
		child->type = getDataBuiltin(attr->type);
		token_object_t *t = tokener_.findToken(attr->type);
		if (t)
		{
			if (!child->type) child->type = udt_object;
			buildCorElementTree(child, t);
		}
		e->children.push_back(child);
	}
	return true;
}

int schema::getDataBuiltin(const char *type)
{
	int len  = sizeof(data_buildin_type) / sizeof(data_buildin_type[0]);
	for (int i = 1; i<len; ++i)
	{
		if (strcmp(data_buildin_type[i], type) == 0)
		{
			return i;
		}
	}
	return 0;
}

bool schema::equalType(const ua_data_t *ud, int type)
{
	int dtype = ud->head.dtype;
	if (ud->head.dtype == type) return true;
	if (dtype == udt_guid && type == udt_camtree) return true;
	if (dtype == udt_guid && type == udt_camcomb) return true;
	if (dtype == udt_guid && type == udt_comptrg) return true;
	if (dtype == udt_guid && type == udt_compact) return true;
	if (dtype == udt_object)
	{
		for (uint32_t i = 0; i<ud->head.nchildren; ++i)
		{
			const ua_data_t *child = ud->children[i];
			if (!equalType(child, type))
				return false;
		}
		return true;
	}
	return false;
}

bool schema::validCfgData(const ua_data_t *Adata)
{
	if (cfg_->children.size() != Adata->head.nchildren)
	{
		return false;
	}

	for (uint32_t i = 0;  i<Adata->head.nchildren; ++i)
	{
		ua_data_t *child = Adata->children[i];
		element *elem = cfg_->children[i];
		if (!equalType(child, elem->type))
		{
			return false;
		}
	}
	return true;
}

bool schema::setCorData(const ua_data_t *data)
{
	if (!validCorData(data))
	{
		return false;
	}
	cor_->copyData(&cordata_, data);
	return true;
}

bool schema::setCorPathData(int path, const ua_data_t *data)
{
	/*int bit;
	  path = flipPath(path);
	  path /= 10;
	  ua_data_t *node = &cordata_;

	  while ( (bit=path%10) > 0)
	  {
	  node = node->children[bit];
	  path /= 10;
	  }
	  cor_->copyData(node, data);*/
	return true;
}

const ua_data_t* schema::getCorData()
{
	return &cordata_;
}

const ua_data_t* schema::getCfgData()
{
	return &cfgdata_;
}

bool schema::getCorPathData(int path, ua_data_t *data)
{
	return true;
}

//1234 => 4321
int schema::flipPath(int path)
{
	int ret = 0;
	if (path < 10){
		ret = path;
		return ret;
	}

	int bit;
	while ( (bit=path%10) > 0)
	{
		ret = ret * 10 + bit;
		path /= 10;
	}
	return ret;
}

bool schema::setCfgPathData(int path, const ua_data_t *data)
{
	path = flipPath(path);
	path /= 10;
	cfg_->handleData((ua_data_t*)data, &cfgdata_, path, uda_add);
	return true;
}

bool schema::getCfgPathData(int path, ua_data_t *data)
{
	return true;
}

bool schema::handleCfgData(const ua_data_invoke_t *invoke)
{
	int path = flipPath(invoke->path);
	path /= 10; //trip root
	cfg_->handleData(invoke->data, &cfgdata_, path, invoke->act);
	dumpCfgData();
	return true;
}

const ua_data_t* schema::lookCorPathData(int path, const ua_data_t *data, char **strfmt)
{
	return lookPathData(path, cor_, data, strfmt);
}

const ua_data_t* schema::lookCfgPathData(int path, const ua_data_t *data, char **strfmt)
{
	return lookPathData(path, cfg_, data, strfmt);
}

const ua_data_t* schema::lookPathData(int path, element *e, const ua_data_t *data, char **strfmt)
{
	path = flipPath(path);
	path /= 10; //trip root

	int bit;
	const ua_data_t *node = data;
	element *elem = e;
	char strelem[1024]= {0};		//string format: element name + element value

	while ( (bit=path%10) > 0)
	{
		if (!node || !node->children || node->head.nchildren < bit)
			return NULL;
		if (!elem || (int)(elem->children.size()) < bit)
			return NULL;
		node = node->children[bit-1];
		elem = elem->children[bit-1];
		path /= 10;

		/*if (strfmt)
		  {
		  strcat(strelem, elem->attr->name);
		  strcat(strelem, ":");
		  }*/
	}

	if (!node)
	{
		return node;
	}
	if (strfmt && elem)
	{
		//join value to 
		char *elemstr = elem2str(elem, node);
		if (elemstr){
			strcat(strelem, elemstr);
			free(elemstr);
		}
		*strfmt = strdup(strelem);
	}
	return node;
}

char* schema::elem2str(element *elem, const ua_data_t *data)
{
	char *str = NULL;

	if(!elem || !data)
		return str;

	if (data->head.dtype == udt_str)
	{
		str = strdup((char*)data->data);
	}
	else if (data->head.dtype == udt_int || 
			data->head.dtype == udt_bool)
	{
		char numstr[32] = {0};
		sprintf(numstr, "%d", *((int*)data->data));
		str = strdup(numstr);
	}
	else if (data->head.dtype == udt_bool)
	{
		char numstr[8] = {0};
		sprintf(numstr, "%s", *((bool*)data->data) ? "Yes" : "No");
		str = strdup(numstr);
	}
	else if (data->head.dtype == udt_guid)
	{
		guid_t *id = (guid_t*)data->data;
		char idstr[48] = {0};
		gr_guid_to_str(id, idstr);
		str = strdup(idstr);
	}
	else if (data->head.dtype == udt_object)
	{

	}
	else{
	}
	int slen = str ? strlen(str) : 0;
	int len = slen + strlen(elem->attr->name) + 2;	//include character ':'
	char *result = (char*)calloc(1, len);
	strcpy(result, elem->attr->name);
	strcat(result, ":");
	if (str)
	{
		strcat(result, str);
		free(str);
	}
	return result;
}

bool schema::canElementConvert(const element *left, const element* right)
{
	bool ret = canTypeConvert(left->type,  right->type);
	//TODO: handle element attribute
	return ret;
}

bool schema::canTypeConvert(int left, int right)
{
	if (left == right)
	{
		return true;
	}
	if (left == udt_camtree && right == udt_camcomb)
	{
		return true;
	}
	if (left == udt_camcomb && right == udt_camtree)
	{
		return true;
	}
	return false;
}
