#!/usr/bin/env python
import os
import sys
import time
import subprocess
import threading
import psutil
import numpy
import argparse

class MemWatcher(threading.Thread):
	def __init__(self, pid, logfile='memlog.csv'):
		threading.Thread.__init__(self)
		self._pinfo = psutil.Process(pid)
		self._start = self._pinfo.create_time()
		self._log = open(logfile, 'w')

		self._last_rss = 0
		self._last_vms = 0

		self.setDaemon(True)
		self.start()

	def run(self):
		while True:
			elapsed = time.time() - self._start
			status = self._pinfo.status()
			meminfo = self._pinfo.memory_info()
			rss, vms = int(meminfo[0]), int(meminfo[1])

			if self._last_rss == rss and self._last_vms == vms:
				time.sleep(0.1)
				continue

			self._last_rss = rss
			self._last_vms = vms
			foutline = '{0},{1},{2},{3}\n'.format(elapsed, status, rss, vms)
			self._log.write(foutline)
			self._log.flush()
			time.sleep(0.25)


if __name__ == '__main__':
	import argparse
	parser = argparse.ArgumentParser(description='Watch memory usage.')
	parser.add_argument('E', type=float)
	parser.add_argument('aoi', type=float)
	parser.add_argument('-n', '--events', dest='n', type=int, default=3, help='number of events on the first run')
	parser.add_argument('-p', '--param', dest='ps', type=str, action='append', default=[], help='passed directly to ./fgamma')
	parser.add_argument('--exec', dest='executable', type=str, default='./fgamma', help='executable')
	args = parser.parse_args()

	start = time.time()
	elapsed = lambda: time.time() - start

	cmd = [args.executable, 'E={0},aoi={1},n={2}'.format(args.E, args.aoi, args.n)]+args.ps
	print 'Command:', cmd

	fgamma_stdout = open('fgamma.stdout.txt', 'w')
	p = subprocess.Popen(cmd, stdout=fgamma_stdout, stderr=subprocess.STDOUT)
	mw = MemWatcher(p.pid)
	p.wait()
	fgamma_stdout.close()
