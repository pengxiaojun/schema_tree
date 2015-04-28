#include "element.hpp"
#include "schema.hpp"

element::element():
	path(0),
	type(0),
	attr(NULL)
{
	//memset(&data, 0, sizeof(ua_data_t));
}

element::~element()
{

}

element *element::lookup(int path)
{
	element *child = this;
	char strpath[10] = {0};
	sprintf(strpath, "%d", path);

	for (size_t i = 1; i<strlen(strpath); i++)
	{
		int index = strpath[i] - '0';
		child = child->children[index - 1];
	}
	return child;
}

bool element::hitData(const ua_data_t *src, const ua_data_t *dst)
{
	if (src->head.nchildren != dst->head.nchildren)
		return false;

	if (src->head.dtype == udt_object)
	{
		for (uint32_t i = 0; i<src->head.nchildren; ++i)
		{
			const ua_data_t *subsrc = src->children[i];
			const ua_data_t *subdst = dst->children[i];
			element *sube = children[i];
			if (sube->attr->key && memcmp(subsrc->data, subdst->data, subdst->head.dlen) == 0)
			{
				return true;
			}
		}
	}
	else
	{
		if(memcmp(src->data, dst->data, src->head.dlen) == 0)
		{
			return true;
		}
	}
	return false;
}

void *element::getTxtData(const ua_data_t *d)
{
	if (d->head.dtype == udt_object)
	{
		if (d->head.nchildren != children.size())
			return NULL;
		for (uint32_t i = 0; i<d->head.nchildren; ++i)
		{
			element *sube = children[i];
			if (sube->attr->txt) return d->children[i]->data;
		}
	}
	else
	{
		return d->data;
	}
	return NULL;
}

int element::getKeyData(const ua_data_t *d, void **pkey)
{
	if (d->head.dtype == udt_object)
	{
		if (d->head.nchildren != children.size())
			return 0;
		for (uint32_t i = 0; i<d->head.nchildren; ++i)
		{
			element *sube = children[i];
			if (sube->attr->key)
			{
				*pkey = d->children[i]->data;
				return d->children[i]->head.dlen;
			}
		}
	}
	else
	{
		*pkey = d->data;
		return d->head.dlen;
	}
	return 0;
}

void element::handleData(ua_data_t *dop, ua_data_t *d, int path, int act)
{
	if (path == 0)
	{
		if (act == uda_init)
		{
			if (this->path != ELEM_ROOT_ID)	// do no clear root children
			{
				clearData(dop, d);
			}
			addData(dop, d);
		}
		else if (act == uda_add)
			return addData(dop, d);
		else if (act == uda_del)
			return delData(dop, d);
		else if (act == uda_mod)
			return modData(dop, d);
		else if (act == uda_clear)
			return clearData(dop, d);
		return;
	}

	int index = path % 10;
	path /= 10;
	ua_data_t *found = NULL;
	ua_data_t *subdop = dop;

	if (attr->plural)
	{
		//find object by id
		subdop = dop->children[0];
		for (uint32_t i = 0; i < d->head.nchildren; ++i)
		{
			ua_data_t *item = d->children[i];
			if (hitData(item, subdop))
			{
				found = item;
				break;
			}
		}
	}
	else{
		found = d;
	}

	if (!found){
		return;
	}
	element *child = children[index - 1];
	child->handleData(subdop->children[index - 1], found->children[index - 1], path, act); 
}

//fresh flag indicate the data will be add to UI
/*path 1.1.3.2  path 1.1.3*/
void element::addData(ua_data_t *add, ua_data_t *d)
{
	if (path == ELEM_ROOT_ID)
	{
		for (uint32_t i = 0; i<add->head.nchildren; ++i)  //add->head
		{
			ua_copy(d->children[i], add->children[i]);
			d->children[i]->head.fresh = 1;
		}
	}
	else
	{
		d->head.array = attr->plural;
		d->head.dtype = add->head.dtype;
		ua_add_children(d, add);
	}
}

void element::modData(ua_data_t *mod, ua_data_t *d)
{
	//do modify
	if (attr->plural)
	{
		for (uint32_t i = 0; i<mod->head.nchildren; ++i)
		{
			ua_data_t *modchild = mod->children[i];
			for (uint32_t j = 0; j<d->head.nchildren; ++j)
			{
				ua_data_t *dchild = d->children[j];
				if (hitData(dchild, modchild))
				{
					ua_update(dchild, modchild);
					break;
				}
			}
		}
	}
	else
	{
		ua_update(d, mod);
	}
}

void element::delData(ua_data_t *del, ua_data_t *d)
{
	if (attr->plural)
	{
		for (uint32_t i = 0; i<del->head.nchildren; ++i)
		{
			ua_data_t *delchild = del->children[i];
			for (uint32_t j = 0; j<d->head.nchildren; ++j)
			{
				ua_data_t *dchild = d->children[j];
				if (hitData(dchild, delchild))
				{
					ua_del_children(d, dchild);
					break;
				}
			}
		}
	}
	else
	{
		ua_deinit(d);
	}
}

void element::clearData(ua_data_t *clear, ua_data_t *d)
{
	ua_deinit(d);
}

void element::copyData(ua_data_t *dst, const ua_data_t *src)
{
	ua_deinit(dst);
	ua_copy(dst, src);
}

void element::getLangText(const char *text, char *out, const char *lang)
{
	int lang_index = ua_get_lang_index();
	if (lang){
		lang_index = ua_query_lang_index(lang);
	}
	int i = 0;
	char *lang_delimitor = "$";
    char *pos, *start, *next;
	char *lang_text = strdup(text);
    next = lang_text;

	do
	{
        start = next;
        pos = strstr(start, lang_delimitor);
		if (pos == NULL)
			break;
		*pos = '\0';
        next = pos + 1;
		i++;
    }while (i<=lang_index);
	if (out)
		strcpy(out, start);
	free(lang_text);
}

void element::getDisplayName(char *name, const char *lang)
{
	getLangText(attr->name, name, lang);	
}

