#!/usr/bin/env python

# Copyright (c) 2010, Loughborough University - Computer Science
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the Institute nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# This file is part of the Contiki operating system.

# \file
#         Automatic allocation of modules to code segments for bankable builds
#         with SDCC's huge memory model.
#
# \author
#         George Oikonomou - <oikonomou@users.sourceforge.net>
import sys
import re
import operator
import fileinput
import os

# Open a module object file (.rel) and read it's code size
def retrieve_module_size(file_name):
	size_pat = re.compile('^A\s+(?:HOME|BANK[0-9])\s+size\s+([1-9A-F][0-9A-F]*)')
	for code_line in open(file_name):
		matches = size_pat.search(code_line)
		if matches is not None:
			return int(matches.group(1), 16)
	return 0

# Searches for a code segment rule for file_name in the segment_rules file
# If there is a rule, we respect it. Otherwise, we can move the file around
def get_source_seg(source_file, object_file, segment_rules):
	for line in open(segment_rules):
		tokens = line.split(None)
		match = re.search(tokens[1], source_file)
		if match is not None:
			# Save it in basename.seg
			base, ext = os.path.splitext(object_file)
			of = open(base + '.seg', 'w')
			of.write(tokens[0] + '\n')
			of.close
			return tokens[0]
	return None

# If segment.rules specified a rule for a source file, the respective object
# file's banking requirement will be stored in object_file.seg
def get_object_seg(object_file):
	base, ext = os.path.splitext(object_file)
	seg = base + '.seg'
	bank = None
	if os.path.isfile(seg) is True:
		of = open(base + '.seg', 'r')
		bank = of.readline().strip()
		of.close()
	return bank

# Open project.mem and retreive the project's total code footprint
def get_total_size(project):
	mem_file = project + '.mem'
	pat = re.compile('FLASH\s+(0x[0-9a-f]+\s+){2}([0-9]+)')
	for line in open(mem_file):
		l = pat.search(line)
		if l is not None:
			return int(l.group(2))

# Open project.map and retrieve the list of modules linked in
# This will only consider contiki sources, not SDCC libraries
# NB: Sometimes object filenames get truncated:
# contiki-sensinode.lib                     [ obj_sensinode/watchdog-cc2430.re ]
# See how for this file the 'l' in 'rel' is missing. For that reason, we retrieve
# the filaname until the last '.' but without the extension and we append 'rel'
# As long as the filename doesn't get truncated, we're good
def populate(project, modules, segment_rules, bins):
	bankable_total = 0
	user_total = 0

	map_file = project + '.map'
	file_pat = re.compile('obj_cc2530dk[^ ]+\.')
	for line in open(map_file):
		file_name = file_pat.search(line)
		if file_name is not None:
			mod = file_name.group(0) + 'rel'
			code_size = retrieve_module_size(mod)
			seg = get_object_seg(mod)
			if seg is not None:
				# This module has been assigned to a bank by the user
				#print 'In', seg, file_name.group(0), 'size', code_size
				bins[seg][0] += code_size
				user_total += code_size
			else:
				# We are free to allocate this module
				modules.append([mod, code_size, "NONE"])
				bankable_total += code_size
	return bankable_total, user_total

# Allocate bankable modules to banks according to a simple
# 'first fit, decreasing' bin packing heuristic.
def bin_pack(modules, bins, offset, log):
	if offset==1:
		bins['HOME'][1] -= 4096

	# Sort by size, descending, in=place
	modules.sort(key=operator.itemgetter(1), reverse=True)

	for module in modules:
		# We want to iterate in a specific order and dict.keys() won't do that
		for bin_id in ['HOME', 'BANK1', 'BANK2', 'BANK3', 'BANK4', 'BANK5', 'BANK6', 'BANK7']:
			if bins[bin_id][0] + module[1] < bins[bin_id][1]:
				bins[bin_id][0] += module[1]
				module[2] = bin_id
				log.writelines('  '.join([module[2].ljust(8), \
					str(module[1]).rjust(5), module[0], '\n']))
				break
			else:
				if bin_id == 'BANK7':
					print "Failed to allocate", module[0], "with size", module[1], \
						"to a code bank. This is fatal"
					return 1
	return 0

# Hack the new bank directly in the .rel file
def relocate(module, bank):
	code_pat = re.compile('(A\s+)(?:HOME|BANK[0-9])(\s+size\s+[1-9A-F][0-9A-F]*.+\n)')

	for line in fileinput.input(module, inplace=1):
		m = code_pat.search(line)
		if m is not None:
			line = m.group(1) + bank + m.group(2)
		sys.stdout.write(line)
	return

if len(sys.argv) < 3:
	print 'Usage:'
	print 'bank-alloc.py project path_to_segment_rules [offset]'
	print 'bank-alloc.py source_file path_to_segment_rules object_file'
	sys.exit(1)

modules = list()
file_name = sys.argv[1]
segment_rules = sys.argv[2]

# Magic: Guess whether we want to determine the code bank for a code file
# or whether we want to bin-pack
basename, ext = os.path.splitext(file_name)
if ext == '.c':
	# Code Segment determination
	if len(sys.argv) < 4:
		print 'Usage:'
		print 'bank-alloc.py project path_to_segment_rules [offset]'
		print 'bank-alloc.py source_file path_to_segment_rules object_file'
		sys.exit(1)
	object_file = sys.argv[3]
	seg = get_source_seg(file_name, object_file, segment_rules)
	if seg is None:
		print "BANK1"
	else:
		print seg
	exit()

# Bin-Packing
offset = 0
if len(sys.argv) > 3 and sys.argv[3] is not None:
	offset = int(sys.argv[3])

sizes = {'total': 0, 'bankable': 0, 'user': 0, 'libs': 0}

# Name : [Allocated, capacity, start_addr]
bins = {
	'HOME': [0, 32768, '0x000000'],
	'BANK1': [0, 32768, '0x018000'],
	'BANK2': [0, 32768, '0x028000'],
	'BANK3': [0, 32768, '0x038000'],
	'BANK4': [0, 32768, '0x048000'],
	'BANK5': [0, 32768, '0x058000'],
	'BANK6': [0, 32768, '0x068000'],
	'BANK7': [0, 32768, '0x078000'],
}

sizes['total'] = get_total_size(basename)
sizes['bankable'], sizes['user'] = populate(basename, modules, segment_rules, bins)
sizes['libs'] = sizes['total'] - sizes['bankable'] - sizes['user']

print 'Total Size =', sizes['total'], 'bytes (' + \
	str(sizes['bankable']), 'bankable,', \
	str(sizes['user']), 'user-allocated,', \
	str(sizes['libs']), 'const+libs)'

bins['HOME'][0] += sizes['libs']

print 'Preallocations: HOME=' + str(bins['HOME'][0]),
for bin_id in ['BANK1', 'BANK2', 'BANK3', 'BANK4', 'BANK5', 'BANK6', 'BANK7']:
	if bins[bin_id][0] > 0:
		print ", " + bin_id + "=" + str(bins[bin_id][0]),
print

# Open a log file
of = open(basename + '.banks', 'w')
pack = bin_pack(modules, bins, offset, of)
of.close()

print "Bin-Packing results (target allocation):"
print "Segment - max - alloc"
for bin_id in ['HOME', 'BANK1', 'BANK2', 'BANK3', 'BANK4', 'BANK5', 'BANK6', 'BANK7']:
	if bins[bin_id][0] > 0:
		print bin_id.rjust(7), str(bins[bin_id][1]).rjust(6), str(bins[bin_id][0]).rjust(6)

if pack > 0:
	sys.exit(1)

# If we reach here we seem to have a sane allocation. Start changing .rel files
for module in modules:
	relocate(module[0], module[2])

flags = ""
# Export LD_POST_FLAGS
for bin_id in ['BANK1', 'BANK2', 'BANK3', 'BANK4', 'BANK5', 'BANK6', 'BANK7']:
	if bins[bin_id][0] > 0:
		flags += "-Wl-b" + bin_id + "=" + bins[bin_id][2] + " "
# Write LD_POST_FLAGS in project.flags
of = open(basename + '.flags', 'w')
of.write(flags + '\n')
of.close()
