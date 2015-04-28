#include "uiinc.h"
#include "uaserialize.h"

static int num_type_size[] =
{
	sizeof(int),sizeof(uint8_t),sizeof(uint16_t),sizeof(uint32_t),sizeof(uint64_t),
	sizeof(int8_t),sizeof(int16_t),sizeof(int32_t),sizeof(int64_t)
};

static const char *lang_code[] = {"en_US", "zh_CN"};

//static int str_type_size[] = {0, sizeof(char), sizeof(wchar_t)};

int GRCALL ua_is_custom_dtype(int type)
{
	return (type >= 10);	//udt_cams = 10
}

void GRCALL ua_init(ua_data_t* Aud)
{
	if (!Aud)
		return;
	memset(Aud, 0, sizeof(*Aud));
	Aud->head.magic = UA_HEAD_MAGIC;
}

void GRCALL ua_deinit(ua_data_t* Aud)
{
	if (!Aud)
		return;
	
	//deinit children
	int i;
	for (i = 0; i<Aud->head.nchildren; ++i)
	{
		ua_data_t *child = Aud->children[i];
        ua_deinit(child);
		free(child);
		Aud->children[i] = NULL;
	}
	if (Aud->data && Aud->head.dlen > 0)
	{
		free(Aud->data);
		Aud->data = NULL;
		Aud->head.dlen = 0;
	}
	if (Aud->head.nchildren > 0 && Aud->children)
	{
		free(Aud->children);
		Aud->children = NULL;
		Aud->head.nchildren = 0;
	}
}

void GRCALL ua_fork_children(ua_data_t *Aud)
{
	int i;
	if (!Aud || Aud->head.nchildren == 0)
		return;

	Aud->children = (ua_data_t**)calloc(Aud->head.nchildren, sizeof(ua_data_t*));
	for (i = 0; i<Aud->head.nchildren; ++i)
	{
		Aud->children[i] = (ua_data_t*)calloc(1, sizeof(ua_data_t));
		ua_init(Aud->children[i]);
	}
}

void GRCALL ua_resize_children(ua_data_t *Aud, int Asize)
{
	if (!Aud)
		return;
	int i;
	int before = Aud->head.nchildren;
	Aud->head.nchildren += Asize;

	if (before == 0)
	{
		ua_fork_children(Aud);
	}
	else
	{
		if (!Aud->children)
			ua_fork_children(Aud);
		else
		{
			Aud->children = (ua_data_t**)realloc(Aud->children, (size_t)(Aud->head.nchildren) * sizeof(ua_data_t*));
			for (i = 0; i<Asize; ++i)
			{
				Aud->children[before + i] = (ua_data_t*)calloc(1, sizeof(ua_data_t));
				ua_init(Aud->children[before + i]);
			}
		}
	}
}

void GRCALL ua_copy_data(ua_data_t *Aud, const void *Adata, int Alen)
{
	if (!Aud || !Adata || Alen == 0)
		return;
	Aud->head.dlen = Alen;
	Aud->data = calloc(1, Aud->head.dlen);
    memcpy(Aud->data, Adata, Alen);
}

void GRCALL ua_copy(ua_data_t *dst, const ua_data_t *src)
{
    memcpy(dst, src, sizeof(ua_data_t));
	ua_copy_data(dst, src->data, src->head.dlen);
	ua_fork_children(dst);
    for (uint32_t i = 0; i<src->head.nchildren; ++i)
    {
        ua_copy(dst->children[i], src->children[i]);
    }
}

void GRCALL ua_update(ua_data_t *dst, const ua_data_t *src)
{
	//verify data type
	if (src->head.dtype != src->head.dtype)
		return;

	if (src->head.dtype == udt_object)
	{
		if (dst->head.nchildren != src->head.nchildren)
			return;

		for (uint32_t i = 0; i<dst->head.nchildren; ++i)
		{
			ua_data_t *dstchild = dst->children[i];
			const ua_data_t *srcchild = src->children[i];

			if (dstchild->head.dtype != srcchild->head.dtype)
				continue;

			if (dstchild->head.dtype != udt_object)
			{
				if (dstchild->data && srcchild->data)
				{
					free(dstchild->data);
					ua_copy_data(dstchild, srcchild->data, srcchild->head.dlen);
				}
			}
		}
	}
	else
	{
		if (dst->data && src->data)
		{
			free(dst->data);
			ua_copy_data(dst, src->data, src->head.dlen);
		}
	}
}

void GRCALL ua_add_children(ua_data_t *Aud, const ua_data_t *Achild)
{
    //copy array
    if (Aud->head.array)
    {
		int nchild = Aud->head.nchildren;
		ua_resize_children(Aud, Achild->head.nchildren);
        for (uint32_t i = 0; i<Achild->head.nchildren; ++i)
        {
            ua_data_t *addchild = Achild->children[i];
            ua_data_t *dchild = Aud->children[nchild + i];
            ua_copy(dchild, addchild);
			dchild->head.fresh = 1;
        }
    }
    else
    {
        ua_copy(Aud, Achild);
		Aud->head.fresh = 1;
    }
}

void GRCALL ua_del_children(ua_data_t *Aud, ua_data_t *Achild)
{
	//reorg children
	if (!Aud || !Achild)
		return;

	ua_deinit(Achild);
	Aud->head.nchildren--;

	if (Aud->head.nchildren > 0)
	{
		ua_data_t **newchildren = (ua_data_t**)calloc(Aud->head.nchildren, sizeof(ua_data_t*));
		int m = 0;
		for (uint32_t k = 0; k <= Aud->head.nchildren; ++k)
		{
			ua_data_t *dchild = Aud->children[k];
			if (dchild != Achild)
			{
				newchildren[m] = (ua_data_t*)calloc(1, sizeof(ua_data_t));
				newchildren[m++] = Aud->children[k];
			}
		}
		free(Aud->children);
		Aud->children = newchildren;
	}
	else{
		free(Aud->children);
		Aud->children = NULL;
	}
}

int GRCALL ua_get_int(const ua_data_t *Aud, int *Aout)
{
	if (Aud->head.dtype != udt_int)
		return -1;
	*Aout = *((int*)Aud->data);
	return 0;
}

int GRCALL ua_get_uint32(const ua_data_t *Aud, uint32_t *Aout)
{
	if (Aud->head.dtype != udt_int)
		return -1;
	*Aout = *((uint32_t*)Aud->data);
	return 0;
}

int GRCALL ua_get_uint64(const ua_data_t *Aud, uint64_t *Aout)
{
	if (Aud->head.dtype != udt_int)
		return -1;
	*Aout = *((uint64_t*)Aud->data);
	return 0;
}

int GRCALL ua_get_bool(const ua_data_t *Aud, int *Aout)
{
	if (Aud->head.dtype != udt_bool)
		return -1;
	*Aout = *((int*)Aud->data);
	return 0;
}

char* GRCALL ua_get_str(const ua_data_t *Aud)
{
	if (Aud->head.dtype != udt_str)
		return NULL;
	char *str = strdup((char*)Aud->data);
	return str;
}

int GRCALL ua_get_guid(const ua_data_t *Aud, guid_t *Aout)
{
	if (Aud->head.dtype != udt_guid)
		return -1;

	*Aout = *((guid_t*)Aud->data);
	return 0;
}

int* GRCALL ua_get_int_array(const ua_data_t *Aud, int *Alen)
{
	if (!Aud)
		return NULL;
	if (!Aud->head.array)
		return NULL;
	int *numbers = (int*)calloc(Aud->head.nchildren, sizeof(int));

	for (int i = 0; i<Aud->head.nchildren; ++i)
	{
		const ua_data_t *child = Aud->children[i];
		if (child->head.dtype != udt_int)
		{
			free(numbers);
			return NULL;
		}
		numbers[i] = *((int*)child->data);
	}
	*Alen = Aud->head.nchildren;
	return numbers;
}

void* GRCALL ua_get_num_array(const ua_data_t *Aud, int AsubType, int *Alen)
{
	if (!Aud)
		return NULL;
	if (!Aud->head.array)
		return NULL;
	if (AsubType >= (int)(sizeof(num_type_size) / sizeof(num_type_size[0])))
		return NULL;
	void *numbers = calloc(Aud->head.nchildren, num_type_size[AsubType]);
	char *pos = (char*)numbers;

	for (int i = 0; i<Aud->head.nchildren; ++i)
	{
		const ua_data_t *child = Aud->children[i];
		if (child->head.dtype != udt_int)
		{
			free(numbers);
			return NULL;
		}
		memcpy(pos, child->data, num_type_size[AsubType]);
		pos += num_type_size[AsubType];
	}
	*Alen = Aud->head.nchildren;
	return numbers;
}

char** GRCALL ua_get_str_array(const ua_data_t *Aud, int *Alen)
{
	if (!Aud)
		return NULL;
	if (!Aud->head.array)
		return NULL;
	char **str = (char**)calloc(Aud->head.nchildren, sizeof(char*));

	for (int i = 0; i<Aud->head.nchildren; ++i)
	{
		const ua_data_t *child = Aud->children[i];
		if (child->head.dtype != udt_str)
		{
			free(str);
			return NULL;
		}
		str[i] = strdup((char*)child->data);
	}
	*Alen = Aud->head.nchildren;
	return str;
}

guid_t* GRCALL ua_get_guid_array(const ua_data_t *Aud, int *Alen)
{
	if (!Aud)
		return NULL;
	if (!Aud->head.array || Aud->head.nchildren == 0)
		return NULL;

	guid_t *ids = (guid_t*)calloc(Aud->head.nchildren, sizeof(guid_t));
	if (!ids){
		return NULL;
	}

	for (int i = 0; i<Aud->head.nchildren; ++i)
	{
		const ua_data_t *child = Aud->children[i];
		if (child->head.dtype != udt_guid)
		{
			free(ids);
			return NULL;
		}
		ids[i] = *((guid_t*)(child->data));
	}
	*Alen = Aud->head.nchildren;
	return ids;
}

void GRCALL ua_free_str(char *Astr)
{
	if (Astr){
		free(Astr);
	}
}

void GRCALL ua_free_int_array(int *Anumbers)
{
	if (Anumbers){
		free(Anumbers);
	}
}

void GRCALL ua_free_num_array(void *Anumbers)
{
	if (Anumbers){
		free(Anumbers);
	}
}

void GRCALL ua_free_str_array(char **Astr, int Alen)
{
	for (int i = 0; i<Alen; ++i)
	{
		free(Astr[i]);
	}
	free(Astr);
}

void GRCALL ua_free_guid_array(guid_t *Aguid)
{
	if (Aguid){
		free(Aguid);
	}
}

int GRCALL ua_set_int(int Anumber, ua_data_t *Aud)
{
	if (!Aud)
		return -1;
	ua_init(Aud);
	Aud->head.dtype = udt_int;
	ua_copy_data(Aud, &Anumber, sizeof(Anumber));
	return 0;
}

int GRCALL ua_set_uint32(uint32_t Anumber, ua_data_t *Aud)
{
	if (!Aud)
		return -1;
	ua_init(Aud);
	Aud->head.dtype = udt_int;
	ua_copy_data(Aud, &Anumber, sizeof(Anumber));
	return 0;
}

int GRCALL ua_set_uint64(uint64_t Anumber, ua_data_t *Aud)
{
	if (!Aud)
		return -1;
	ua_init(Aud);
	Aud->head.dtype = udt_int;
	ua_copy_data(Aud, &Anumber, sizeof(Anumber));
	return 0;
}

int GRCALL ua_set_bool(int Abool, ua_data_t *Aud)
{
	if (!Aud)
		return -1;
	ua_init(Aud);
	Aud->head.dtype = udt_bool;
	int bval = Abool ? 1 : 0;
	ua_copy_data(Aud, &bval, sizeof(bval));
	return 0;
}

int GRCALL ua_set_str(const char *Astr, ua_data_t *Aud)
{
	if (!Aud)
		return -1;
	ua_init(Aud);
	Aud->head.dtype = udt_str;
	if (Astr) ua_copy_data(Aud, Astr, strlen(Astr) + 1);	//add NULL terminator
	return 0;
}

int GRCALL ua_set_guid(const guid_t *Aid, ua_data_t *Aud)
{
	if (!Aud)
		return -1;

	ua_init(Aud);
	Aud->head.dtype = udt_guid;
	if (Aid) ua_copy_data(Aud, Aid, sizeof(*Aid));	//add NULL terminator
	return 0;
}

int GRCALL ua_set_int_array(const int *Anumbers, int Alen, ua_data_t *Aud)
{
	if (!Aud)
		return -1;

	ua_init(Aud);
	Aud->head.dtype = udt_object;	//the parent of array data type is alaways a object type. array element is the member of object
	Aud->head.array = 1;
	Aud->head.nchildren = Alen;

	ua_fork_children(Aud);
	for (int i = 0; i<Alen; ++i)
	{
		ua_data_t *child = Aud->children[i];
		child->head.dtype = udt_int;
		ua_copy_data(child, &Anumbers[i], sizeof(Anumbers[i]));
	}
	return 0;
}

int GRCALL ua_set_num_array(const void *Anumbers, int Alen, int AsubType, ua_data_t *Aud)
{
	if (!Aud)
		return -1;
	if (AsubType >= (int)(sizeof(num_type_size) / sizeof(num_type_size[0])))
		return -1;

	ua_init(Aud);
	Aud->head.dtype = udt_object;	//the parent of array data type is alaways a object type. 
	Aud->head.array = 1;
	Aud->head.nchildren = Alen;
	ua_fork_children(Aud);
	char *pos = (char*)Anumbers;

	for (int i = 0; i<Alen; ++i)
	{
		ua_data_t *child = Aud->children[i];
		child->head.dtype = udt_int;
		child->head.dsubtype = AsubType;
		ua_copy_data(child, pos, num_type_size[AsubType]);
		pos += num_type_size[AsubType];
	}
	return 0;
}

int GRCALL ua_set_str_array(const char **Astr, int Alen, ua_data_t *Aud)
{
	if (!Aud)
		return -1;

	ua_init(Aud);
	Aud->head.dtype = udt_object;	//the parent of array data type is alaways a object type. array element is the member of object
	Aud->head.array = 1;
	Aud->head.nchildren = Alen;
	ua_fork_children(Aud);

	for (int i = 0; i<Alen; ++i)
	{
		ua_data_t *child = Aud->children[i];
		child->head.dtype = udt_str;
		ua_copy_data(child, Astr[i], strlen(Astr[i])+1);
	}
	return 0;
}

int GRCALL ua_set_guid_array(const guid_t *Aids, int Alen, ua_data_t *Aud)
{
	if (!Aud)
		return -1;

	ua_init(Aud);
	Aud->head.dtype = udt_object;	//the parent of array data type is alaways a object type. array element is the member of object
	Aud->head.array = 1;
	Aud->head.nchildren = Alen;

	ua_fork_children(Aud);
	for (int i = 0; i<Alen; ++i)
	{
		ua_data_t *child = Aud->children[i];
		child->head.dtype = udt_guid;
		ua_copy_data(child, &Aids[i], sizeof(Aids[i]));
	}
	return 0;
}

int __get_data_len(const ua_data_t *Adata, int *Alen)
{
	if (!Adata || Adata->head.magic != UA_HEAD_MAGIC)
	{
		return -1;
	}
	uint32_t i;
	int datalen = (int)(UA_HEAD_SIZE + Adata->head.dlen);
	*Alen += datalen;
	for (i = 0; i<Adata->head.nchildren; ++i)
	{
		if (!Adata->children)
		{
			return -1;
		}

		if (-1 ==  __get_data_len(Adata->children[i], Alen))
		{
			return -1;
		}
    }
	return 0;
}

//convert to network byte order
int __expand_data_serialize(const ua_data_t *Adata, unsigned char **Apos)
{
	if (!Adata || Adata->head.magic != UA_HEAD_MAGIC)
	{
		return -1;
	}
	*Apos = ua_serialize_head(*Apos, &(Adata->head));
	
	if (Adata->head.dlen > 0)
	{
		*Apos = ua_serialize_data(*Apos, Adata);
	}
	uint32_t i;
	for (i = 0; i<Adata->head.nchildren; ++i)
	{
		if (-1 == __expand_data_serialize(Adata->children[i], Apos))
		{
			return -1;
		}
	}
	return 0;
}

int __expand_data(const ua_data_t *Adata, char **Apos, int *Alen)
{
	if (!Adata || Adata->head.magic != UA_HEAD_MAGIC)
	{
		return -1;
	}
	memcpy(*Apos, Adata, UA_HEAD_SIZE);
	*Apos += UA_HEAD_SIZE;
	*Alen += UA_HEAD_SIZE;
	if (Adata->head.dlen > 0)
	{
		memcpy(*Apos, Adata->data, Adata->head.dlen);
		*Apos += Adata->head.dlen;
		*Alen += (int)(Adata->head.dlen);
	}
	uint32_t i;
	for (i = 0; i<Adata->head.nchildren; ++i)
	{
		if (-1 == __expand_data(Adata->children[i], Apos, Alen))
		{
			return -1;
		}
	}
	return 0;
}

int __collapse_data_serialize(ua_data_t *Adata, unsigned char **Apos)
{
	*Apos = ua_deserialize_head(*Apos, &Adata->head);
	if (Adata->head.magic != UA_HEAD_MAGIC)
	{
		return -1;
	}

	ua_fork_children(Adata);
	if (Adata->head.dlen > 0)
	{
		ua_copy_data(Adata, *Apos, Adata->head.dlen);
		*Apos = ua_deserialize_data(*Apos, Adata);
	}
	uint32_t i;
	//copy data
	for (i = 0; i<Adata->head.nchildren; ++i)
	{
		if (-1 == __collapse_data_serialize(Adata->children[i], Apos))
		{
			return -1;
		}
	}
	return 0;
}

int __collapse_data(ua_data_t *Adata, char **Apos, int *Alen)
{
	const ua_data_t *data = (const ua_data_t*)(*Apos);
	if (data->head.magic != UA_HEAD_MAGIC)
	{
		return -1;
	}
	memcpy(Adata, data, UA_HEAD_SIZE);
	*Apos += UA_HEAD_SIZE;
	*Alen += UA_HEAD_SIZE;
	ua_fork_children(Adata);
	if (Adata->head.dlen > 0)
	{
		ua_copy_data(Adata, *Apos, Adata->head.dlen);
		*Apos += Adata->head.dlen;
		*Alen += (int)(Adata->head.dlen);
	}
	uint32_t i;
	//copy data
	for (i = 0; i<Adata->head.nchildren; ++i)
	{
		if (-1 == __collapse_data(Adata->children[i], Apos, Alen))
		{
			return -1;
		}
	}
	return 0;
}

int GRCALL ua_expand_data(const ua_data_t * Adata, void **Amem, int *Alen)
{
	int dlen= 0;
	if (-1 == __get_data_len(Adata, &dlen))
	{
		return -1;
	}
	//char *ps = (char*)calloc(1, dlen);
	unsigned char* ps = (unsigned char*)calloc(1, dlen);
	*Amem = ps;
	*Alen = dlen;
	//int len = 0;
	//return __expand_data(Adata, &ps, &len);
	return __expand_data_serialize(Adata, &ps);
}

int GRCALL ua_collapse_data(void *Amem, int *Alen,  ua_data_t* Adata)
{
	ua_init(Adata);
	if (*Alen == 0) return -1;
	unsigned char *ps = (unsigned char*)Amem;
	//return __collapse_data(Adata, &ps, Alen);
	return __collapse_data_serialize(Adata, &ps);
}

void  GRCALL ua_de_expand(void *Amem)
{
	if (Amem)
	{
		free(Amem);
		Amem = NULL;
	}
}

void  GRCALL ua_de_collapse(ua_data_t *Adata)
{
	ua_deinit(Adata);
	/*for (uint32_t i = 0; i<Adata->nchildren; ++i)
	  {
	  ua_data_t *child = Adata->children[i];
	  ua_de_collapse(child);
	  delete child;
	  Adata->children[i] = NULL;
	  }
	  if (Adata->data){
	  free(Adata->data);
	  Adata->data = NULL;
	  }
	  if (Adata->children && Adata->nchildren > 0){
	  delete(Adata->children);
	  Adata->children = NULL;
	  }*/
}

//a && b
int ua_binary2compound(const ua_binary_t *Abinary, ua_data_t* Adata)
{
	if (!Abinary || !Adata)
		return -1;

	ua_init(Adata);
	Adata->head.attr = Abinary->op;
	Adata->head.nchildren = 2;		//binary operator is alaways compoud of left and right expression
	ua_fork_children(Adata);

	ua_data_t *lchild = Adata->children[0];
	lchild->head.dtype = udt_guid;
	ua_copy_data(lchild, Abinary->ldata, Abinary->llen);

	ua_data_t *rchild = Adata->children[1];
	rchild->head.dtype = udt_guid;
	ua_copy_data(rchild, Abinary->rdata, Abinary->rlen);
	return 0;
}

int ua_compound2binary(const ua_data_t* Adata, ua_binary_t *Abinary)
{
	if (!Abinary || !Adata)
		return -1;

	if (Adata->head.nchildren < 2 || !Adata->children)
		return -1;

	memset(Abinary, 0, sizeof(*Abinary));
	Abinary->op = Adata->head.attr;
	ua_data_t *lchild = Adata->children[0];
	Abinary->llen = lchild->head.dlen;
	Abinary->ldata = calloc(1, Abinary->llen);
	memcpy(Abinary->ldata, lchild->data, Abinary->llen);

	ua_data_t *rchild = Adata->children[1];
	Abinary->rlen = rchild->head.dlen;
	Abinary->rdata = calloc(1, Abinary->rlen);
	memcpy(Abinary->rdata, rchild->data, Abinary->rlen);
	return 0;
}

//!c
int ua_unary2compound(const ua_unary_t *Aunary, ua_data_t *Adata)
{
	if (!Aunary || !Adata)
		return -1;

	ua_init(Adata);
	Adata->head.attr = Aunary->op;
	Adata->head.dtype = udt_guid;
	ua_copy_data(Adata, Aunary->data, Aunary->dlen);
	return 0;
}

int ua_compound2unary(const ua_data_t *Adata, ua_unary_t *Aunary)
{
	if (!Aunary || !Adata)
		return -1;

	memset(Aunary, 0, sizeof(*Aunary));
	Aunary->op = Adata->head.attr;
	Aunary->dlen = Adata->head.dlen;
	Aunary->data = calloc(1, Aunary->dlen);
	memcpy(Aunary->data, Adata->data, Aunary->dlen);
	return 0;
}

int ua_query_lang_index(const char *code)
{
	size_t i;
	for (i = 0; i<sizeof(lang_code)/sizeof(lang_code[0]); ++i)
	{
		if (strcmp(lang_code[i], code) == 0)
		{
			return i;
		}
	}
	return 0; //default set to en_US
}

int ua_get_lang_index()
{
	int index = 0; //default to en-US
	//get lanaguage path config
	char line[128] = {0};
	char *pline, *pos;
	char *lang_file_prefix = "langFile=";
	char *lang_prefix = "language=";
	char *file = NULL;
	FILE *fp = fopen("schema.ini","r");

	if (fp == NULL){
		return index;
	}
	while ((pline = fgets(line, sizeof(line), fp))!= NULL)
	{
		if (line[strlen(line)-1] == '\n')
			line[strlen(line)-1] = '\0';
		pos = strstr(line, lang_file_prefix); 
		if (pos){
            pos += strlen(lang_file_prefix);
            file = pos;
			break;
		}
	}
    if (file == NULL){
		fclose(fp);
		return index;
	}
	fclose(fp);
	fp = fopen(file, "r");
	if (fp == NULL)
	{
		return index;
	}
	while ((pline = fgets(line, sizeof(line), fp)) != NULL)
	{
		if (line[strlen(line)-1] == '\n')
			line[strlen(line)-1] = '\0';
		pos = strstr(line, lang_prefix); 
		if (pos != NULL){
			pos += strlen(lang_prefix);	
			index = ua_query_lang_index(pos);
			break;
		}
	}
	fclose(fp);
	return index;
}
