#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct kvpair{
	char *key;
	char *value;
} *KVPair;

typedef struct dict{
	KVPair head;
	struct dict *next;
} *Dict;

KVPair init_kv(const char *k, const char *val){
	KVPair kv = (KVPair)malloc(sizeof(struct kvpair));
	if(kv != NULL){
		kv->key = (char*)malloc(sizeof(strlen(k)+1));
		kv->value = (char*)malloc(sizeof(strlen(val)+1));
		strcpy(kv->key,k);
		strcpy(kv->value,val);
	}
	return kv;
}

Dict init_dict(KVPair val){
	Dict dict = (Dict)malloc(sizeof(struct dict));
	if(dict != NULL){
		dict->next = NULL;
		dict->head = val;
	}
	return dict;
}

void printDict(volatile Dict *dictionary){
	Dict ptr = *dictionary;
	//fprintf(stderr, "key: %s, val: %s\n", dict->head->key, dict->head->value);
	while(ptr != NULL){
		fprintf(stderr, "next: %s\n", ptr->head->value);
		///fprintf(stderr, "key: %s, val: %s\n", dict->head->key, dict->head->value);
		ptr = ptr->next;
	}
	fprintf(stderr, "LISTO\n");
}

void dict_add(volatile Dict *dictionary, const char *key, char *value) {
	KVPair newKv = init_kv(key,value);
	Dict newDict = init_dict(newKv);
	if (*dictionary == NULL){
		*dictionary = newDict;
	}
	else{
		Dict ptr = *dictionary;
		while(ptr->next != NULL){
			ptr = ptr->next;
		}
		ptr->next = newDict;
	}
}

int dict_has(Dict *dictionary, const char *key) {
	Dict ptr = *dictionary;
    if (ptr == NULL)
        return 0;
    while(ptr != NULL) {
        if(strcmp(ptr->head->key, key) == 0)
            return 1;
        ptr = ptr->next;
    }
    return 0;
}

char* dict_get(Dict *dictionary, const char *key) {
	Dict ptr = *dictionary;
    if (ptr == NULL)
        return 0;
    while(ptr != NULL) {
        if(strcmp(ptr->head->key, key) == 0)
            return ptr->head->value;
        ptr = ptr->next;
    }
    return 0;
}

void kv_free(KVPair kv){
	free(kv->key);
	free(kv->value);
	free(kv);
}

void dict_free(Dict *dictionary) {
	Dict ptr = *dictionary;
    if(dictionary != NULL){
	    kv_free(ptr->head);
	    Dict next = ptr->next;
	    dict_free(&next);
	    free(ptr);
	}
}

igraph_strvector_t dict_values(Dict *dictionary){
	Dict ptr = *dictionary;
	igraph_strvector_t newVal;
	igraph_strvector_init(&newVal,0);
	while(ptr != NULL){
		igraph_strvector_add(&newVal,ptr->head->value);
		ptr = ptr->next;
	}
	return newVal;
}