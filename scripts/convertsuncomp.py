#!/usr/bin/env python
"""
Converts a .dat file of the solar composition to a YAML file.
"""
import re
import yaml
from datetime import datetime

# Configuration variables
cfg_ifname = 'sun_composition.dat'
cfg_ofname = 'solarmodel.yml'

# Convenience functions
def component(e, n):
	return {'element': e, 'number_density': float(n)}

def isotopes(e, isos):
	isotopes = []
	for i in isos:
		isotopes.append({'A':i[0], 'number_density': float(i[1])})
	return {'element': e, 'isotopes': isotopes}

if __name__=='__main__':
	# Read the .dat file
	print 'Opening the input file:', cfg_ifname
	fin=open(cfg_ifname)

	# The first three lines are assumed to be the column headers
	column_matches = []
	rxp=re.compile('([A-Za-z0-9_]+)\(([A-Za-z0-9/\-]+)\)|([A-Za-z0-9_]+)')
	for _ in range(3):
		column_matches += rxp.findall(fin.readline())
	columns = [c3 if len(c1)==0 else c1 for c1,c2,c3 in column_matches]
	#print 'Columns:', columns

	# The rest of the lines are either empty or space-delimited rows of values
	layers = []
	for line in fin.readlines():
		line = line.strip()
		if len(line)==0 or line[0] == '#':
			continue
		valuestrings=line.split()
		values =[int(valuestrings[0])] # first value is the index (i.e. int)
		values+=[float(s) for s in valuestrings[1:]] # all others are floats
		layers.append(dict(zip(columns, values)))

	# Start constructing the YAML model
	print 'Constructing the dictionary'
	model = {
		'name': 'The Sun',
		'date': 'Feb 24',
		'convert_datetime': str(datetime.now())
	}
	model_layers = {'layers': []}

	last_radius = 0.0
	for layer in layers:
		r = layer['radius'] - last_radius

		ly = {
			'thickness': float(r),
			'temperature': float(layer['Temp']),
			'pressure': float(0.1 * layer['pres']), # from dyne/cm2 -> Pa
			'components': []
		}

		ly['components'].append(component('H', layer['n_H']))

		ly['components'].append(component('Ne', layer['n_Ne']))
		ly['components'].append(component('Na', layer['n_Na']))
		ly['components'].append(component('Mg', layer['nMg']))
		ly['components'].append(component('Al', layer['n_Al']))
		ly['components'].append(component('Si', layer['n_Si']))
		ly['components'].append(component('P', layer['n_P']))

		ly['components'].append(component('S', layer['n_S']))
		ly['components'].append(component('Cl', layer['n_Cl']))
		ly['components'].append(component('Ar', layer['n_Ar']))
		ly['components'].append(component('K', layer['n_K']))
		ly['components'].append(component('Ca', layer['n_Ca']))
		ly['components'].append(component('Sc', layer['n_Sc']))

		ly['components'].append(component('Ti', layer['n_Ti']))
		ly['components'].append(component('V', layer['n_V']))
		ly['components'].append(component('Co', layer['n_Co']))
		ly['components'].append(component('Ni', layer['n_Ni']))
		ly['components'].append(component('Cr', layer['n_Cr']))
		ly['components'].append(component('Mn', layer['n_Mn']))
		ly['components'].append(component('Fe', layer['n_Fe']))

		ly['components'].append(isotopes('He', [(3, layer['n_He3']), (4, layer['n_He4'])]))
		ly['components'].append(isotopes('C', [(12, layer['n_C12']), (13, layer['n_C13'])]))
		ly['components'].append(isotopes('N', [(14, layer['n_N14']), (15, layer['n_N15'])]))
		ly['components'].append(isotopes('O', [(16, layer['n_o16']), (17, layer['n_o17']), (18, layer['n_o18'])]))

		model_layers['layers'].append(ly)
		last_radius = r

	print 'Dumping the model into YAML'
	oyaml  = "# Solar atmosphere\n"
	oyaml += yaml.dump(model, default_flow_style=False)
	oyaml += "\n"
	oyaml += yaml.dump(model_layers)

	print 'Writing the model to the output file:', cfg_ofname
	fout = open(cfg_ofname, 'w')
	fout.write(oyaml)
	fout.close()

	print 'All done!'
