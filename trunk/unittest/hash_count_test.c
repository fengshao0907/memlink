#include "hashtest.h"

int main()
{
#ifdef DEBUG
	logfile_create("test.log", 3);
#endif
	HashTable* ht;
	char key[64];
	int valuesize = 8;
	char val[64];
	int maskformat[3] = {4, 3, 1};	
	char charmaskformat[3] = {4, 3, 1};
	char* maskstr1[] = {"8:3:1", "7:2:1", "6:2:1"};
	int maskarray[6][3] = { { 7, UINT_MAX, 1}, {6, 2, 1}, { 4, 1, UINT_MAX}, 
		                    {8, 3, 1}, {8, 8, 8}, { UINT_MAX, UINT_MAX, UINT_MAX} }; 
	int num  = 200;
	int masknum = 3;
	int ret;
	int i = 0;

	char *conffile;
	conffile = "memlink.conf";
	DINFO("config file: %s\n", conffile);
	myconfig_create(conffile);

	my_runtime_create_common("memlink");

	ht = g_runtime->ht;


	///////////begin test;
	//test1 : hashtable_add_info_mask //����key
	for(i = 0; i < num; i++)
	{
		sprintf(key, "heihei%03d", i);
		hashtable_key_create_mask(ht, key, valuesize, maskformat, masknum);
	}
	for(i = 0; i < num; i++)
	{
		sprintf(key, "heihei%03d", i);
		HashNode* pNode = hashtable_find(ht, key);
		if(NULL == pNode)
		{
			printf("hashtable_add_info_mask error. can not find %s\n", key);
			return -1;
		}
	}

	///////test : hashtable_add_mask ����num��value
	HashNode *node = NULL;
	DataBlock *dbk = NULL;
	char	 *item = NULL; 	
	int pos = 0;
	for(i = 0; i < num; i++)
	{
		sprintf(val, "value%03d", i);
		pos = i;
		ret = hashtable_add_mask(ht, key, val, maskarray[i%4], masknum, pos);
		if (ret < 0) {
			printf("add value err: %d, %s\n", ret, val);
			return ret;
		}
		
		ret = hashtable_find_value(ht, key, val, &node, &dbk, &item);
		if (ret < 0) {
			printf("not found value: %d, %s\n", ret, key);
			return ret;
		}
	}
	printf("insert %d values ok!\n", num);

	//////////test 5 :count
	int visible_count;
	int mask_count;
	for(i = 0; i < 4; i++)
	{
		ret = hashtable_count(ht, key, maskarray[i], masknum, &visible_count, &mask_count);
		if (ret < 0) {
			printf("count key err: %d, %s\n", ret, key);
			return ret;
		}
		if((visible_count) != 50)
		{
			printf("count key err: %d, %s, visible_count:%d\n", ret, key, visible_count);
			return ret;
		}
	}
	
	///����maskfomat�涨: {8, 8, 8}
	ret = hashtable_count(ht, key, maskarray[4], masknum, &visible_count, &mask_count);
	if (ret < 0) {
		printf("count key err: %d, %s\n", ret, key);
		return ret;
	}
	if(visible_count == 50)
	{
		printf("count key err: %d, %s, visible_count:%d\n", ret, key, visible_count);
		return ret;
	}

	//maskΪ��	
	ret = hashtable_count(ht, key, maskarray[5], masknum, &visible_count, &mask_count);
	if (ret < 0) {
		printf("count key err: %d, %s\n", ret, key);
		return -1;
	}
	if(visible_count != num)
	{
		printf("count key err: %d, %s, visible_count:%d\n", ret, key, visible_count);
		return -1;
	}
	printf("hashtable count test end!\n");
	return 0;
}

