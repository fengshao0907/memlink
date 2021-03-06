#!/usr/bin/python
# coding: utf-8
import os, sys
home = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(os.path.join(home, "client/python"))
import time
import subprocess
from memlinkclient import *
from synctest import *

def test():
    client2master = MemLinkClient('127.0.0.1', MASTER_READ_PORT, MASTER_WRITE_PORT, 30);
    client2slave  = MemLinkClient('127.0.0.1', SLAVE_READ_PORT, SLAVE_WRITE_PORT, 30);
    
    sync_test_clean()
    test_init()
    change_synclog_indexnum(5000000);
    data_produce1()


    print 
    print '============================= test g  =============================='
    #cmd = 'rm test.log'
    #print cmd
    #os.system(cmd)
    '''
    cmd = 'rm data/*'
    print cmd
    os.system(cmd)
    cmd = 'cp data_bak/* data/'
    print cmd
    os.system(cmd)
    
    x1 = restart_master()
    time.sleep(10) # wait master to load data
    
    cmd = 'rm data/dump.dat'
    print cmd
    os.system(cmd)
    cmd = 'mv data/dump.dat_bak data/dump.dat'
    os.system(cmd)'''

    x2 = start_a_new_slave()
    print 'sleep 10'
    time.sleep(10)

    #2 kill slave 1
    print 'kill slave'
    x2.kill()
    x2 = restart_slave()
    print 'sleep 20'
    time.sleep(20)

    #2 kill slave 2
    print 'kill slave'
    x2.kill()
    x2 = restart_slave()
    print 'sleep 30'
    time.sleep(30)

    #2 kill slave 3
    print 'kill slave'
    x2.kill()
    x2 = restart_slave()
    print 'sleep 30'    
    time.sleep(30)
    
    if 0 != stat_check(client2master, client2slave):
        print 'test g error!'
        return -1
    print 'stat ok'
    
    print 'test g ok'

    #x1.kill()
    #x2.kill()
    
    client2master.destroy()
    client2slave.destroy()
    sync_test_clean()

    return 0

if __name__ == '__main__':
    sys.exit(test())

