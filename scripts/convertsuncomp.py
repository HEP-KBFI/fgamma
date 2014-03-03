#!/usr/bin/env python
import re
import yaml

ifile = 'sun_composition.dat'
ofile = 'solarmodel.yml'

# Read the .dat file
print 'Open input file:', ifile
fin=open(ifile)
rxp=re.compile('([A-Za-z0-9_]+)\(([A-Za-z0-9/\-]+)\)|([A-Za-z0-9_]+)')
cat_matches = rxp.findall(fin.readline())+rxp.findall(fin.readline())+rxp.findall(fin.readline())
cats = [c3 if len(c1)==0 else c1 for  c1,c2,c3 in cat_matches]

lys=[]
for ln in fin.readlines():
	ln = ln.strip()
	if len(ln)==0 or ln[0] == '#':
		continue
	ps=ln.split()
	ps=[int(ps[0])]+[float(s) for s in ps[1:]]
	lys.append(dict(zip(cats, ps)))

# Define a few convenience functions
def prev_next_rowiterator(iterator):
	prv = None
	cur = iterator.next()
	nxt = iterator.next()
	try:
		while True:
			yield (prv,cur,nxt)
			prv = cur
			cur = nxt
			nxt = iterator.next()
	except StopIteration:
		yield (prv,cur,None)

def component(e, n):
	return {'element': e, 'number_density': float(n)}

def isotopes(e, isos):
	isotopes = []
	for i in isos:
		isotopes.append({'A':i[0], 'number_density': float(i[1])})
	return {'element': e, 'isotopes': isotopes}

# Start constructing the YAML model
mdlyaml = {
	'name': 'The Sun (Feb 24)',
}
mdlyaml_layers = {'layers': []}

r_prv = 0.0
for prv,cur,nxt in prev_next_rowiterator(iter(lys)):
	if nxt is not None:
		r = 0.5*(nxt['radius'] + cur['radius'])
	else:
		r = 2*cur['radius'] - r_prv
	ly = {
		'thickness': float(r-r_prv),
		'temperature': float(cur['Temp']),
		'pressure': float(0.1 * cur['pres']), # from dyne/cm2 -> Pa
		'components': []
	}

	ly['components'].append(component('H', cur['n_H']))

	ly['components'].append(component('Ne', cur['n_Ne']))
	ly['components'].append(component('Na', cur['n_Na']))
	ly['components'].append(component('Mg', cur['nMg']))
	ly['components'].append(component('Al', cur['n_Al']))
	ly['components'].append(component('Si', cur['n_Si']))
	ly['components'].append(component('P', cur['n_P']))

	ly['components'].append(component('S', cur['n_S']))
	ly['components'].append(component('Cl', cur['n_Cl']))
	ly['components'].append(component('Ar', cur['n_Ar']))
	ly['components'].append(component('K', cur['n_K']))
	ly['components'].append(component('Ca', cur['n_Ca']))
	ly['components'].append(component('Sc', cur['n_Sc']))

	ly['components'].append(component('Ti', cur['n_Ti']))
	ly['components'].append(component('V', cur['n_V']))
	ly['components'].append(component('Co', cur['n_Co']))
	ly['components'].append(component('Ni', cur['n_Ni']))
	ly['components'].append(component('Cr', cur['n_Cr']))
	ly['components'].append(component('Mn', cur['n_Mn']))
	ly['components'].append(component('Fe', cur['n_Fe']))

	ly['components'].append(isotopes('He', [(3, cur['n_He3']), (4, cur['n_He4'])]))
	ly['components'].append(isotopes('C', [(12, cur['n_C12']), (13, cur['n_C13'])]))
	ly['components'].append(isotopes('N', [(14, cur['n_N14']), (15, cur['n_N15'])]))
	ly['components'].append(isotopes('O', [(16, cur['n_o16']), (17, cur['n_o17']), (18, cur['n_o18'])]))

	mdlyaml_layers['layers'].append(ly)
	r_prv = r

print 'Write the model to the output file:', ofile
oyaml  = "# Solar atmosphere\n"
oyaml += yaml.dump(mdlyaml, default_flow_style=False)
oyaml += "\n"
oyaml += yaml.dump(mdlyaml_layers)
fout = open('solarmodel.yml', 'w')
fout.write(oyaml)
fout.close()

print 'All done!'
