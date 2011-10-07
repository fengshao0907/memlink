#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memlink_client.h>
#include "logfile.h"
#include "test.h"

int main()
{
	MemLink	*m;
#ifdef DEBUG
	logfile_create("stdout", 3);
#endif
	m = memlink_create("127.0.0.1", MEMLINK_READ_PORT, MEMLINK_WRITE_PORT, 30);
	if (NULL == m) {
		DERROR("memlink_create error!\n");
		return -1;
	}

	int  ret;
	char key[64];
    char name[32];	
	int nodenum = 100;
	int i;
	
    for( i = 0 ; i < nodenum; i++) 
	{
		//sprintf(key, "haha%d", i);
        sprintf(name, "test%d", i);
        DINFO("create table:%s\n", name);
		ret = memlink_cmd_create_table_list(m, name, 6, "4:3:1");
		
		if (ret != MEMLINK_OK) {
			DERROR("create table %s error: %d\n", name, ret);
			return -2;
		}
	}
	
    DINFO("create table:%s\n", name);
	ret = memlink_cmd_create_table_list(m, name, 6, "4:3:1");
	if (ret == MEMLINK_OK) {
		DERROR("memlink_cmd_create %s error: %d\n", key, ret);
		return -3;
	}

	strcpy(name, "haha1111");
    DINFO("create table:%s\n", name);
	ret = memlink_cmd_create_table_list(m, name, -1, "4:3:1");
	if (ret == MEMLINK_OK) {
		DERROR("memlink_cmd_create %s error: %d\n", key, ret);
		return -3;
	}

	strcpy(name, "haha2222");
    DINFO("create table:%s\n", name);
	ret = memlink_cmd_create_table_list(m, name, 12, "4:3:21474");
	if (ret == MEMLINK_OK) {
		DERROR("memlink_cmd_create %s error: %d, mask=%s\n", key, ret, "4:3:21474");
		return -3;
	}

	strcpy(name, "haha3333");
    DINFO("create table:%s\n", name);
	ret = memlink_cmd_create_table_list(m, name, 12, "");
	if (ret != MEMLINK_OK) {
		DERROR("memlink_cmd_create %s error: %d, mask=%s\n", key, ret, "");
		return -3;
	}

    // test create node
    sprintf(key, "%s.haha", name);
    
    DINFO("create table node:%s\n", key);
    ret = memlink_cmd_create_node(m, key);
    if (ret != MEMLINK_OK) {
        DERROR("create node error:%d\n", ret);
        return -1;
    }
    DINFO("create table node:%s\n", key);
    ret = memlink_cmd_create_node(m, NULL);
    if (ret == MEMLINK_OK) {
        DERROR("create node error:%d\n", ret);
        return -1;
    }
    DINFO("create table node:%s\n", key);
    ret = memlink_cmd_create_node(m, ".haha");
    if (ret == MEMLINK_OK) {
        DERROR("create node error:%d\n", ret);
        return -1;
    }
    DINFO("create table node:%s\n", key);
    ret = memlink_cmd_create_node(m, "");
    if (ret == MEMLINK_OK) {
        DERROR("create node error:%d\n", ret);
        return -1;
    }
    DINFO("create table node:%s\n", key);
    ret = memlink_cmd_create_node(m, "haha");
    if (ret == MEMLINK_OK) {
        DERROR("create node error:%d\n", ret);
        return -1;
    }

    DINFO("create table node:%s\n", key);
    ret = memlink_cmd_create_node(m, key);
    if (ret == MEMLINK_OK) {
        DERROR("create node error:%d\n", ret);
        return -1;
    }

	memlink_destroy(m);

	return 0;
}

