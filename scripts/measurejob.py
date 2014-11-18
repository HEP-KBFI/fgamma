#!/usr/bin/env python
import os
import sys
import time
import argparse
import subprocess
import Queue
import json
import numpy

def sidefill(string, w=80, char='-'):
	if string is None:
		return char*w
	sw = int(w-len(string)-2)
	even = (sw%2==0)
	sw = sw/2 if even else (sw-1)/2
	return char*sw + ' ' + string + (' ' if even else '  ') + char*sw

def measure(command, stdout=None):
	"""Measures a single execution of fgamma.

	It uses subprocess to run fgamma, parses its stdout and returns a
	dictionary of measured event times.
	"""
	print 'Measure: {0}'.format(command)
	call_stdout = subprocess.check_output(command, stderr=subprocess.STDOUT)

	ts = []
	for line in call_stdout.decode().split('\n'):
		if line.startswith('% event '):
			splitline = line.split(None,7)
			ts.append(float(splitline[5]))
		elif line.startswith('% done'):
			ts.append(float(line.split()[4]))
		if stdout is not None:
			stdout.write(line+'\n')

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
	parser.add_argument('-c', '--cutoff', dest='cutoff', type=float, help='cutoff value')
	parser.add_argument('-m', '--model', dest='model', type=str, help='model file')
	parser.add_argument('-p', '--param', dest='ps', type=str, action='append', default=[], help='passed directly to ./fgamma')
	parser.add_argument('-n', '--events', dest='n', type=int, default=3, help='number of events on the first run')
	parser.add_argument('-t', '--target', dest='target', type=float, default=120)
	parser.add_argument('--exec', dest='executable', type=str, default='./fgamma', help='executable')
	args = parser.parse_args()

	print 'Target time: {0} s'.format(args.target)

	start = time.time()
	elapsed = lambda: time.time() - start

	nextn = args.n
	mss = []

	fgamma_stdout = open('fgamma.stdout.txt', 'w')

	while nextn is not None:
		cmd = [args.executable, 'E={0},aoi={1},n={2}'.format(args.E, args.aoi, nextn)]
		if args.cutoff is not None:
			cmd += ['--cutoff={0}'.format(args.cutoff)]
		if args.model is not None:
			cmd += ['--model={0}'.format(args.model)]
		cmd += args.ps

		try:
			mss.append(measure(cmd, stdout=fgamma_stdout))
		except subprocess.CalledProcessError as e:
			print 'Error: fgamma failed (returncode={0})'.format(e.returncode)
			print sidefill('FGAMMA STDOUT')
			print e.output
			print sidefill(None)
			exit(1)
		stats = mss_stats(mss)
		print 'Stats:', stats

		timeleft = args.target - elapsed()
		maxn = max(int(round((args.target-stats.first.mean)/(10*stats.ev.mean))), 10)
		nextn = int(round((timeleft-stats.first.mean)/stats.ev.mean))
		if nextn < 3:
			break
		nextn = min(nextn, maxn)
		fgamma_stdout.write('+'*80 + '\n')
	print 'Total elapsed:', elapsed()
	fgamma_stdout.close()

with open('results.json', 'w') as fout:
	json.dump({
		'params': {
			'E': args.E,
			'aoi': args.aoi,
			'cutoff': args.cutoff
		},
		'boot': {
			'n': stats.first.n,
			'mean': stats.first.mean,
			'stdev': stats.first.std
		},
		'event': {
			'n': stats.ev.n,
			'mean': stats.ev.mean,
			'stdev': stats.ev.std
		}
	}, fout)
with open('data.json', 'w') as fout:
	json.dump(mss, fout)
