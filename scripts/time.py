#!/usr/bin/env python3
import argparse
import subprocess
import numpy as np

def measure_once(call):
	sp=subprocess.Popen(call, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
	sp.wait()
	usertime = float(sp.stderr.readlines()[-1])
	return usertime

if __name__=='__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument('-n', type=int, default=3)
	parser.add_argument('command', nargs=argparse.REMAINDER)
	args = parser.parse_args()
	#print(args)

	call = ['/usr/bin/time', '-f%U'] + args.command
	#print('Call:', call)

	print('Start measuring..')
	utimes = []
	for _ in range(args.n):
		try:
			utime = measure_once(call)
		except KeyboardInterrupt:
			print('Measurement interrupted.')
			break
		print(' - measured:', utime)
		utimes.append(utime)

	if len(utimes) == 0:
		print('No measurements, no math!')
	elif len(utimes) == 1:
		print('Measured once:', utimes[0])
	else:
		print('Measured {0} times.'.format(len(utimes)))
		mean = np.mean(utimes)
		stdev = np.std(utimes)
		print('Mean utime: {:7.2f}'.format(mean))
		print('Std. dev:   {:7.2f} ({:7.2%})'.format(stdev, stdev/mean if mean!=0.0 else float('nan')))
		print('Min, max:   {:7.2f}, {:7.2f}'.format(min(utimes), max(utimes)))


