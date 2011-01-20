#!/usr/bin/python
# coding: utf-8
import os, sys
home = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(os.path.join(home, "client/python"))
import time
import subprocess
from memlinkclient import *

MASTER_READ_PORT  = 11011
MASTER_WRITE_PORT = 11012

SLAVE_READ_PORT  = 11021
SLAVE_WRITE_PORT = 11022

client2master = MemLinkClient('127.0.0.1', MASTER_READ_PORT, MASTER_WRITE_PORT, 30);
client2slave  = MemLinkClient('127.0.0.1', SLAVE_READ_PORT, SLAVE_WRITE_PORT, 30);

def result_check():
    ret1, stat1 = client2master.stat_sys()
    ret2, stat2 = client2slave.stat_sys()
    if stat1 and stat2:
        if stat1.keys != stat2.keys or stat1.values != stat2.values or stat1.data_used != stat2.data_used:
            print 'stat error!'
            print 'ret1: %d, ret2: %d' % (ret1, ret2)
            print 'master stat', stat1
            print 'slave stat', stat2
            return -1
    else:
        print 'stat error!'
        print 'ret1: %d, ret2: %d' % (ret1, ret2)
        return -1

    return 0

def test():
    cmd = "cp ../memlink ../memlink_master -rf"
    print cmd
    os.system(cmd)
    
    cmd = "cp ../memlink ../memlink_slave -rf"
    print cmd
    os.system(cmd)
    
    home = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    os.chdir(home)

    memlink_master_file  = os.path.join(home, 'memlink_master')
    memlink_master_start = memlink_master_file + ' etc/memlink.conf'

    memlink_slave_file  = os.path.join(home, 'memlink_slave')
    memlink_slave_start = memlink_slave_file + ' etc_slave/memlink.conf'

    print 
    print '============================= test c  =============================='
    #start a new master
    cmd = 'bash clean.sh'
    print cmd
    os.system(cmd)
    cmd = "killall memlink_master"
    print cmd
    os.system(cmd)
    print memlink_master_start
    x1 = subprocess.Popen(memlink_master_start, stdout=subprocess.PIPE, stderr=subprocess.PIPE, 
                             shell=True, env=os.environ, universal_newlines=True)     
    #start a new slave
    cmd = 'bash clean_slave.sh'
    print cmd
    os.system(cmd)
    cmd = "killall memlink_slave"
    print cmd
    os.system(cmd)
    print memlink_slave_start
    x2 = subprocess.Popen(memlink_slave_start, stdout=subprocess.PIPE, stderr=subprocess.PIPE, 
                             shell=True, env=os.environ, universal_newlines=True) 

    key = 'haha'
    maskstr = "8:1:1"
    ret = client2master.create(key , 12, "4:3:1")
    if ret != MEMLINK_OK:
        print 'create error:', ret, key
        return -1
    print 'create 1 key'

    #1 insert 999
    num = 999
    for i in xrange(0, num):
        val = '%012d' % i
        ret = client2master.insert(key, val, maskstr, i)
        if ret != MEMLINK_OK:
            print 'insert error!', key, val, maskstr, ret
            return -2;
    print 'insert %d val' % num

    time.sleep(1)

    #2 kill slave
    print 'kill slave'
    x2.kill()

    #3 insert 1000
    num2 = 1999
    for i in xrange(num, num2):
        val = '%012d' % i
        ret = client2master.insert(key, val, maskstr, i)
        if ret != MEMLINK_OK:
            print 'insert error!', key, val, maskstr, ret
            return -2;
    print 'insert %d val' % (num2 - num)

    print memlink_slave_start
    x2 = subprocess.Popen(memlink_slave_start, stdout=subprocess.PIPE, stderr=subprocess.PIPE, 
                             shell=True, env=os.environ, universal_newlines=True) 
    time.sleep(2)
    
    if 0 != result_check():
        print 'test c error!'
        return -1

    print 'test c ok'

    x1.kill()
    x2.kill()

    client2master.close()
    client2slave.close()
    
    print 
    print '============================= test d  =============================='
    #start a new master
    cmd = 'bash clean.sh'
    print cmd
    os.system(cmd)
    cmd = "killall memlink_master"
    print cmd
    os.system(cmd)
    print memlink_master_start
    x1 = subprocess.Popen(memlink_master_start, stdout=subprocess.PIPE, stderr=subprocess.PIPE, 
                             shell=True, env=os.environ, universal_newlines=True)     
    time.sleep(1)

    key = 'haha'
    maskstr = "8:1:1"
    ret = client2master.create(key, 12, "4:3:1")
    if ret != MEMLINK_OK:
        print 'create error:', ret, key
        return -1
    print 'create 1 key'

    #1 insert 999
    num = 999
    for i in xrange(0, num):
        val = '%012d' % i
        ret = client2master.insert(key, val, maskstr, i)
        if ret != MEMLINK_OK:
            print 'insert error!', key, val, maskstr, ret
            return -2;
    print 'insert %d val' % num

    print 'dump...'
    client2master.dump()
    
    #start a new slave
    cmd = 'bash clean_slave.sh'
    print cmd
    os.system(cmd)
    cmd = "killall memlink_slave"
    print cmd
    os.system(cmd)
    print memlink_slave_start
    x2 = subprocess.Popen(memlink_slave_start, stdout=subprocess.PIPE, stderr=subprocess.PIPE, 
                             shell=True, env=os.environ, universal_newlines=True) 

    time.sleep(3)
    
    if 0 != result_check():
        print 'test d error!'
        return -1

    print 'test d ok'
    
    client2master.destroy()
    client2slave.destroy()

    return 0

if __name__ == '__main__':
    sys.exit(test())

