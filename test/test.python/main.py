#!/usr/bin/env python
from multiprocessing import Process
import os
import time

def sleeper(name, seconds):
   print 'starting child process with id: ', os.getpid()
   print 'parent process:', os.getppid()
   print 'sleeping for %s ' % seconds
   time.sleep(seconds)
   print "Done sleeping"


if __name__ == '__main__':
   print "in parent process (id %s)" % os.getpid()
   p = Process(target=sleeper, args=('bob', 5))
   p.start()
   print "in parent process after child process start"
   print "parent process about to join child process"
   p.join()
   print "in parent process after child process join" 
   print "parent process exiting with id ", os.getpid()
   print "The parent's parent process:", os.getppid()

