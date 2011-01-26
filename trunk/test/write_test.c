#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memlink_client.h>
#include "logfile.h"
#include "test.h"

static MemLink *m;
static char key[32];

typedef struct linkNode
{
	int num;
	char key[20];
	char val[255];
	char mask[20];
	int flag;
	struct linkNode* next;
}LinkNode;

static LinkNode linkList;
static LinkNode* list = &linkList;

void creatList(LinkNode* l, char* key)
{
	strcpy(l->key, key);
	l->flag = MEMLINK_TAG_RESTORE;
	l->num = 0;
	strcpy(l->val, "");
	l->next = NULL;
	strcpy(l->mask, "");
}

void destroyList(LinkNode* l)
{
	LinkNode* p = l->next;
	int i = 0;
	int ret = -1;
	while( p != NULL)
	{	
		LinkNode* pp = p;		
		p = p->next;
		l->next = p;		
		free(pp);
	}
}

int linkNodeInsert(LinkNode* l, char* key, char* val, char* mask, int pos)
{
	LinkNode* node = (LinkNode*)malloc(sizeof(LinkNode));
	memset(node, 0, sizeof(node));
	strcpy(node->key, key);	
	strcpy(node->val, val);
	strcpy(node->mask, mask);
	node->flag = MEMLINK_TAG_RESTORE;
	node->next = NULL;
	int i = 0;
	LinkNode* p = l->next;
	while (i != pos && p != NULL)
	{
		l = p;
		p = p->next;
		i++;
	}
	l->next = node;
	node->next = p;
	return 0;
}

int linkNodeDel(LinkNode* l, char* key, char* val)
{
	LinkNode* p = l->next;
	int i = 0;
	int ret = -1;
	while( p != NULL)
	{	
		if( 0 == strcmp(p->val, val) )
		{	
			//LinkNode* pp = p;
			l->next = p->next;
			free(p);
			ret = 0;
			break;
		}
		l = p;
		p = p->next;
	}
	return 0;
}

int linkNodeUpdate(LinkNode* l, char* key, char* val, int pos)
{	
	LinkNode* head = l;
	LinkNode* p = head->next;	
	LinkNode* thatOne = NULL;
	int i = 0;
	int ret = -1;
	while( p != NULL)
	{	
		if( 0 == strcmp(p->val, val) )
		{	
			//LinkNode* pp = p;
			thatOne = p;
			head->next = p->next;
			ret = 0;
			break;
		}
		head = p;
		p = p->next;
	}
	if(0 != ret) //找到val，否则推出
		return 0;
	
	head = l;
	p = head->next;
	while( i != pos && p != NULL)
	{
		head = p;
		p = p->next;
		i++;
	}
	head->next = thatOne;
	thatOne->next = p;
	return 0;
}

int linkNodeTag(LinkNode* l, char* key, char* val, int flag)
{
	LinkNode* p = l->next;
	int ret = -1;
	while( p != NULL)
	{	
		if( 0 == strcmp(p->val, val) )
		{	
			//LinkNode* pp = p;
			p->flag = flag;
			ret = 0;
			break;
		}
		l = p;
		p = p->next;
	}
	return 0;
}

int linkNodeMask(LinkNode* l, char* key, char* val, char* mask)
{
	LinkNode* p = l->next;
	int ret = -1;
	while( p != NULL)
	{	
		if( 0 == strcmp(p->val, val) )
		{	
			//LinkNode* pp = p;
			strcpy(p->mask, mask);
			ret = 0;
			break;
		}
		l = p;
		p = p->next;
	}
	return 0;
}

int check_result(LinkNode* l)
{
	LinkNode* p = l->next;
	int ret = -1;
	int i = 0;
	
	DINFO("key: %s\n", p->key);
	
	MemLinkResult	result;
	int				range_start = 0;
	int				range_count = 2000;
	
	ret = memlink_cmd_range(m, key, MEMLINK_VALUE_VISIBLE, "", range_start, range_count, &result);
	if (ret != MEMLINK_OK) {
		DERROR("range error, key:%s, ret:%d\n", key, ret);
		return -4;
	}
	//printf("range count:%d\n", result.count);
	MemLinkItem	*item = result.root;
	while( p != NULL )
	{	
		/*
		DINFO("i: %d, value:%s, mask:%s ", i, p->val, p->mask);
		if(p->flag)
			printf(" del\n");
		else
			printf(" \n");
		*/
		if(p->flag == MEMLINK_TAG_DEL)
		{
			p = p->next;
			continue;
		}
		if(NULL == item)
		{
			DERROR("item must not be NULL!\n");
			return -1;
		}
		if (strcmp(item->value, p->val) != 0) {
			DERROR("range value error, value:%s, listvalue:%s, i:%d\n", item->value, p->val, i);
			return -1;
		}
		if (strcmp(item->mask, p->mask) != 0) {
			DERROR("range mask error, mask:%s, listmask:%s, i:%d\n", item->mask, p->mask, i);
			return -1;
		}
		item = item->next;
		l = p;
		p = p->next;
		i++;
	}
	memlink_result_free(&result);
	return 0;
}

int my_rand(int base)
{
	int i = -1;
	usleep(10);
	srand((unsigned)time(NULL)+rand());//在种子里再加一个随机数
	
	while (i < 0 )
	{
		i = rand()%base;
	}
	return i;
} 

int createKey()
{
	int  ret;
	char my_key[32];
	int num = my_rand(200);
	sprintf(my_key, "haha%d", num);
	ret = memlink_cmd_create_list(m, my_key, 6, "4:3:1");
	if (ret != MEMLINK_OK && ret != MEMLINK_ERR_EKEY) {
		DERROR("create %s error: %d\n", my_key, ret);
		return -2;
	}
	return 0;
}

int delVal()
{
	int  ret;
	char val[64];
	int num = my_rand(200);
	sprintf(val, "%06d", num);
	ret = memlink_cmd_del(m, key, val, strlen(val));
	
	linkNodeDel(list, key, val);
	if (ret != MEMLINK_OK && ret != MEMLINK_ERR_NOVAL) {
		DERROR("delVal error, key:%s, val:%s\n", key, val);
		return -5;
	}
	return 0;
}

int insertVal()
{
	int  ret;
	char val[64];
	int num = my_rand(200);
	
	char *mask  = "7:2:1";
	sprintf(val, "%06d", num);
	ret = memlink_cmd_insert(m, key, val, strlen(val), mask, 0);
	
	linkNodeInsert(list, key, val, mask, 0);
	if (ret != MEMLINK_OK) {
		DERROR("insertVal error, key:%s, val:%s\n", key, val);
		return -5;
	}
	return 0;
}

int moveVal()
{
	int  ret;
	char val[64];
	int num = my_rand(200);
	sprintf(val, "%06d", num);
    //DINFO("====== insert i:%d\n", i);
	ret = memlink_cmd_move(m, key, val, strlen(val), 0);
	
	linkNodeUpdate(list, key, val, 0);
	if (ret != MEMLINK_OK && ret != MEMLINK_ERR_NOVAL) {
		DERROR("moveVal error, key:%s, val:%s, ret:%d\n", key, val, ret);
		return -4;
	}

	return 0;
}

int tagVal()
{
	int  ret;
	char val[64];
	
	int num = my_rand(200);
	sprintf(val, "%06d", num);
	
	int tag;
	(my_rand(2))?(tag = MEMLINK_TAG_DEL):(tag = MEMLINK_TAG_RESTORE);
	ret = memlink_cmd_tag(m, key, val, strlen(val), tag);
	linkNodeTag(list, key, val, tag);
	if (ret != MEMLINK_OK && ret != MEMLINK_ERR_NOVAL) {
		DERROR("tagVal error, ret:%d\n", ret);
		return -4;
	}
	return 0;
}

int maskVal()
{
	int  ret;
	char val[64];
	
	int num = my_rand(200);
	
	sprintf(val, "%06d", num);
	
	char *newmask  = "7:2:1";
	ret = memlink_cmd_mask(m, key, val, strlen(val), newmask);
	linkNodeMask(list, key, val, newmask);
	if (ret != MEMLINK_OK && ret != MEMLINK_ERR_NOVAL) {
		DERROR("maskVal error, key:%s, val:%s, mask:%s, ret:%d\n", key, val, newmask, ret);
		return -4;
	}
	return 0;
}

int cleanKey()
{
	int ret;
	ret = memlink_cmd_clean(m, key);
	if (ret != MEMLINK_OK) {
		DERROR("clean error, key:%s, ret:%d\n", key, ret);
		return -5;
	}

	return 0;
}

typedef int (*pFunc)();
pFunc p[7] = {createKey, delVal, insertVal, moveVal, tagVal, maskVal, cleanKey};


int main()
{
#ifdef DEBUG
	logfile_create("stdout", 3);
#endif
	m = memlink_create("127.0.0.1", MEMLINK_READ_PORT, MEMLINK_WRITE_PORT, 30);
	if (NULL == m) {
		DERROR("memlink_create error!\n");
		return -1;
	}

	int  ret;

	sprintf(key, "haha");
	ret = memlink_cmd_create_list(m, key, 6, "4:3:1");
	creatList(list, key);
	if (ret != MEMLINK_OK) {
		DERROR("create %s error: %d\n", key, ret);
		return -2;
	}
	
	int  i;
	char val[64];
	char* maskstr1[] = {"8:3:1", "7:2:1", "6:2:1"};
	int	 insertnum = 200;
	for (i = 0; i < insertnum; i++) {
		sprintf(val, "%06d", i);
		int k = i%3;
		ret = memlink_cmd_insert(m, key, val, strlen(val), maskstr1[k], 0);
		////////////////////////////////
		linkNodeInsert(list, key, val, maskstr1[k], 0);

		if (ret != MEMLINK_OK) {
			DERROR("insert error, key:%s, val:%s, mask:%s, i:%d\n", key, val, maskstr1[k], i);
			return -3;
		}
	}

	int loopNum = 150;
	int opNum[7] = {0};
	for(i = 0; i < loopNum; i++)
	{
		int index = my_rand(7);
		opNum[index] += 1;
		//printf("index:%d\n", index);
		ret = p[index]();
		if (ret != MEMLINK_OK) {
			DERROR("Function %d err!\n", index);
			return -3;
		}		
		ret = check_result(list);
		if (ret != MEMLINK_OK) {
			DERROR("Function %d err!\n", index);
			printf("createKey:%d, delVal:%d, insertVal:%d, moveVal:%d, tagVal:%d, maskVal:%d, cleanKey:%d\n", 
				opNum[0], opNum[1],opNum[2],opNum[3],opNum[4],opNum[5],opNum[6]);
			return -3;
		}
	}
	printf("createKey:%d, delVal:%d, insertVal:%d, moveVal:%d, tagVal:%d, maskVal:%d, cleanKey:%d\n", 
		opNum[0], opNum[1],opNum[2],opNum[3],opNum[4],opNum[5],opNum[6]);
	//ret = check_result(list);

	destroyList(list);
	memlink_destroy(m);
	
	return ret;
}

