#!/usr/bin/env python
import os
import sys
import time
import subprocess
import Queue
import numpy
import argparse

def sidefill(string, w=80, char='-'):
	if string is None:
		return char*w
	sw = int(w-len(string)-2)
	even = (sw%2==0)
	sw = sw/2 if even else (sw-1)/2
	return char*sw + ' ' + string + (' ' if even else '  ') + char*sw

class ChildExecutionError(Exception):
	def __init__(self, err, stdout):
		self.err = err
		self.stdout = stdout
	def __repr__(self):
		return 'ChildExecutionError: {0}'.format(self.err)

def measure(command, stdout=None):
	print 'Command:', command
	p = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
	print 'Process started, pid = {0}'.format(p.pid)
	if p.wait() != 0:
		fgammastdout = p.stdout.read()
		if stdout is not None:
			stdout.write(fgammastdout)
			stdout.flush()
		raise ChildExecutionError(
			'returncode ({0}) not 0'.format(p.returncode),
			fgammastdout
		)

	ts = []
	for line in p.stdout:
		if line.startswith('EVENT') or line.startswith('% done'):
			ts.append(float(line.split()[4]))
		if stdout is not None:
			stdout.write(line)

	return {
		'dts': [t2-t1 for t1,t2 in zip(ts, ts[1:])],
		'first': ts[0],
		'total': ts[-1]
	}

class liststats:
	def __init__(self, lst):
		self.lst = lst
		self.n = len(lst)
		self.mean = numpy.mean(lst)
		self.std = numpy.std(lst)

class mss_stats:
	def __init__(self, mss):
		dts, firsts = [], []
		for ms in mss:
			dts += ms['dts']
			firsts.append(ms['first'])
		self.ev = liststats(dts)
		self.first = liststats(firsts)

	def __repr__(self):
		return 'dts: {0:.2f} ({1:.2f})'.format(self.ev.mean, self.ev.std)

if __name__ == '__main__':
	import argparse
	parser = argparse.ArgumentParser(description='Run a run-time measurement.')
	parser.add_argument('E', type=float)
	parser.add_argument('aoi', type=float)
	parser.add_argument('-t', '--target', dest='target', type=float, default=120)
	parser.add_argument('-n', '--events', dest='n', type=int, default=3, help='number of events on the first run')
	parser.add_argument('-p', '--param', dest='ps', type=str, action='append', default=[], help='passed directly to ./fgamma')
	args = parser.parse_args()

	print 'Target time: {0} s'.format(args.target)

	start = time.time()
	elapsed = lambda: time.time() - start

	ns = Queue.Queue()
	ns.put(args.n)
	mss = []

	fgamma_stdout = open('fgamma.stdout.txt', 'w')

	while not ns.empty():
		n = ns.get()
		cmd = ['./fgamma', 'E={0},aoi={1},n={2}'.format(args.E, args.aoi, n)]+args.ps
		try:
			mss.append(measure(cmd, stdout=fgamma_stdout))
		except ChildExecutionError as e:
			print 'Error: fgamma failed or something.'
			print ' >>', e.err
			print sidefill('FGAMMA STDOUT')
			print e.stdout
			print sidefill(None)
			exit(1)
		stats = mss_stats(mss)
		print 'Stats:', stats

		timeleft = args.target - elapsed()
		maxn = max(int(round((args.target-stats.first.mean)/(10*stats.ev.mean))), 10)
		nextn = int(round((timeleft-stats.first.mean)/stats.ev.mean))
		if nextn >= 3:
			ns.put(min(nextn, maxn))
			fgamma_stdout.write('+'*80 + '\n')
	print 'Total elapsed:', elapsed()
	fgamma_stdout.close()

fout = open('results.txt', 'w')
fout.write('params {0} {1}\n'.format(args.E, args.aoi))
fout.write('boot {0} {1} {2}\n'.format(stats.first.n, stats.first.mean, stats.first.std))
fout.write('event {0} {1} {2}\n'.format(stats.ev.n, stats.ev.mean, stats.ev.std))
for ms in mss:
	fout.write('run {0} {1} {2}\n'.format(len(ms['dts']), ms['first'], ms['total']))
	fout.write(format(' '.join([str(it) for it in ms['dts']]))+'\n')
fout.close()
