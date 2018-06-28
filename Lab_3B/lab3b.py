#!/bin/python

#TODO: 
'''
Structure formats:

block_dict:
[state, offset, ref0, ref1, ref2, ref3]
possible states:
	allocated:			used/ reserved
	free:				free
	allocated and free:		free_alloc
	not allocated and not free:	unreserved	
offset - offset in "virtual" space
	NOTE: no actual need for offset, we keep for legacy purposes
ref0 - direct inodes that reference block 		(array)
ref1 - 1st level pointer inodes that reference block	(array)
ref2 - 2nd level pointer inodes that reference block	(array)
ref3 - 3rd level pointer inodes that reference block	(array)

inode_dict:
[state, link, linkC, parent, c_name, c_inode]
Possible states:
	allocated:			alloc
	free:				free
	allocated and free:		free_alloc
	not allocated and not free:	unalloc	
link - actual link count
linkC - metadata link count
parent - inode of parent
c_name - name of child		(array)
c_inode - inode of child	(array)

dir_dict:
[type, parent]
type - "." or ".."
parent - inode of parent
'''
import csv
import sys

filepath = sys.argv[1]
fp = open(filepath, 'rb')
reader = csv.reader(fp)

block_dict = {}
inode_dict = {}
dir_dict = {}
# must have only 1 group
for line in reader:
	if line[0] == 'SUPERBLOCK':
		total_blocks = int(line[1])
		inode_size = int(line[4])
		block_size = int(line[3])
		reserved_offset_inode = int(line[7])
	elif line[0] == 'GROUP':
		total_inodes = int(line[3]);
		reserved_offset_block = int(line[8]) + inode_size * total_inodes / block_size
	elif line[0] == 'BFREE':
		info = ['free', 0, [], [], [], []] 
		block_dict[int(line[1])] = info
	elif line[0] == 'INDIRECT':
		info = ['used', 0, [], [], [], []] 
		block_dict[int(line[5])] = info
	elif line[0] == 'IFREE':
		info = ['free', 0, 0, None, [], []] 
		if int(line[1]) == 2:
			info[3] = 2
		inode_dict[int(line[1])] = info

fp.seek(0)
reader = csv.reader(fp)
for line in reader:
	if line[0] == 'INODE':
		inode = int(line[1])
		for i in range(12, 27):
			block = int(line[i])
			if block == 0:
				continue
			if block in block_dict:
				if block_dict[block][0] == 'free':
					block_dict[block][0] = 'free_alloc'
				if i < 24:
					block_dict[block][2].append(inode)
				if i == 24:
					block_dict[block][3].append(inode)
				if i == 25:
					block_dict[block][4].append(inode)
				if i == 26:
					block_dict[block][5].append(inode)
			else:
				if block < reserved_offset_block:
					state = 'reserved'
				else:
					state = 'used'
				if i < 24:
					info = [state, 0, [inode], [], [], []]
				elif i == 24:
					info = [state, 12, [], [inode], [], []]
				elif i == 25:
					info = [state, 268, [], [], [inode], []]
				elif i == 26:
					info = [state, 65804, [], [], [], [inode]]
				block_dict[block] = info
		if inode in inode_dict:
			inode_dict[inode][0] = 'free_alloc'
			inode_dict[inode][2] = int(line[6])
		else:
			info = ['alloc', 0, int(line[6]), None, [], []]  
			if inode == 2:
				info[3] = 2
			inode_dict[inode] = info
fp.seek(0)
reader = csv.reader(fp)
for line in reader:
	if line[0] == 'DIRENT':
		ref_name = line[6]
		ref_inode = int(line[3])
		parent_inode = int(line[1])
		if ref_name == "'..'" or ref_name == "'.'":
			dir_dict[ref_inode] = [0,0]
			dir_dict[ref_inode][0] = ref_name
			dir_dict[ref_inode][1] = parent_inode
		else:
			if ref_inode in inode_dict:
				if ref_inode != 2:
					inode_dict[ref_inode][3] = parent_inode
			else:
				info = ['unalloc', 0, 0, parent_inode, [], []]  
				inode_dict[ref_inode] = info	

			if parent_inode in inode_dict:
				inode_dict[parent_inode][4].append(ref_name)
				inode_dict[parent_inode][5].append(ref_inode)
			else:
				info = ['unalloc', 0, 0, None, [ref_name], [ref_inode]]  
				if parent_inode == 2:
					info[3] = 2
				inode_dict[parent_inode] = info	

		inode_dict[ref_inode][1] = inode_dict[ref_inode][1] + 1
		#inode_dict[parent_inode][1] = inode_dict[parent_inode][1] + 1

for key in range(reserved_offset_block, total_blocks):
	if key not in block_dict:
		print 'UNREFERENCED BLOCK ' + str(key)
for key in block_dict:
	if key > total_blocks or key < 1:
		for i in range(0, len(block_dict[key][2])):
			print 'INVALID BLOCK ' + str(key) + ' IN INODE ' + str(block_dict[key][2][i]) + ' AT OFFSET 0'
		for i in range(0, len(block_dict[key][3])):
			print 'INVALID INDIRECT BLOCK ' + str(key) + ' IN INODE ' + str(block_dict[key][3][i]) + ' AT OFFSET 12'
		for i in range(0, len(block_dict[key][4])):
			print 'INVALID DOUBLE INDIRECT BLOCK ' + str(key) + ' IN INODE ' + str(block_dict[key][4][i]) + ' AT OFFSET 268'
		for i in range(0, len(block_dict[key][5])):
			print 'INVALID TRIPLE INDIRECT BLOCK ' + str(key) + ' IN INODE ' + str(block_dict[key][5][i]) + ' AT OFFSET 65804'
	if block_dict[key][0] == 'reserved':
		for i in range(0, len(block_dict[key][2])):
			print 'RESERVED BLOCK ' + str(key) + ' IN INODE ' + str(block_dict[key][2][i]) + ' AT OFFSET 0'
		for i in range(0, len(block_dict[key][3])):
			print 'RESERVED INDIRECT BLOCK ' + str(key) + ' IN INODE ' + str(block_dict[key][3][i]) + ' AT OFFSET 12'
		for i in range(0, len(block_dict[key][4])):
			print 'RESERVED DOUBLE INDIRECT BLOCK ' + str(key) + ' IN INODE ' + str(block_dict[key][4][i]) + ' AT OFFSET 268'
		for i in range(0, len(block_dict[key][5])):
			print 'RESERVED TRIPLE INDIRECT BLOCK ' + str(key) + ' IN INODE ' + str(block_dict[key][5][i]) + ' AT OFFSET 65804'
	if block_dict[key][0] == 'free_alloc':
		print 'ALLOCATED BLOCK ' + str(key) + ' ON FREELIST'	
	if len(block_dict[key][2]) + len(block_dict[key][3]) + len(block_dict[key][4]) + len(block_dict[key][5]) > 1:
		for i in range(0, len(block_dict[key][2])):
			print 'DUPLICATE BLOCK ' + str(key) + ' IN INODE ' + str(block_dict[key][2][i]) + ' AT OFFSET 0'
		for i in range(0, len(block_dict[key][3])):
			print 'DUPLICATE INDIRECT BLOCK ' + str(key) + ' IN INODE ' + str(block_dict[key][3][i]) + ' AT OFFSET 12'
		for i in range(0, len(block_dict[key][4])):
			print 'DUPLICATE DOUBLE INDIRECT BLOCK ' + str(key) + ' IN INODE ' + str(block_dict[key][4][i]) + ' AT OFFSET 268'
		for i in range(0, len(block_dict[key][5])):
			print 'DUPLICATE TRIPLE INDIRECT BLOCK ' + str(key) + ' IN INODE ' + str(block_dict[key][5][i]) + ' AT OFFSET 65804'


for key in range(reserved_offset_inode, total_inodes):
	if key not in inode_dict:
		print 'UNALLOCATED INODE ' + str(key) + ' NOT ON FREELIST'
for key in inode_dict:
	if inode_dict[key][0] == 'free_alloc':
		print 'ALLOCATED INODE ' + str(key) + ' ON FREELIST'
	if inode_dict[key][1] != inode_dict[key][2] and inode_dict[key][0] == 'alloc':
		print 'INODE ' + str(key) + ' HAS ' + str(inode_dict[key][1]) + ' LINKS BUT LINKCOUNT IS ' + str(inode_dict[key][2])
	for i in range(0, len(inode_dict[key][5])):
		if inode_dict[key][5][i] > total_inodes or inode_dict[key][5][i] < 1: 
			print 'DIRECTORY INODE ' + str(key) + ' NAME ' + inode_dict[key][4][i] + ' INVALID INODE ' + str(inode_dict[key][5][i])
		elif inode_dict[inode_dict[key][5][i]][0] == 'unalloc' or inode_dict[inode_dict[key][5][i]][0] == 'free':
			print 'DIRECTORY INODE ' + str(key) + ' NAME ' + inode_dict[key][4][i] + ' UNALLOCATED INODE ' + str(inode_dict[key][5][i])
for key in dir_dict:
	if dir_dict[key][0] == "'..'" and key != inode_dict[dir_dict[key][1]][3]:
		if dir_dict[dir_dict[key][1]][1] != 2 and key != 2:
			print 'DIRECTORY INODE ' + str(dir_dict[key][1]) + ' NAME \'..\' LINK TO INODE ' + str(key) + ' SHOULD BE ' + str(inode_dict[dir_dict[key][1]][3])
	elif dir_dict[key][0] == "'.'" and dir_dict[key][1] != key:
		print 'DIRECTORY INODE ' + str(key) + ' NAME \'.\' LINK TO INODE ' + str(dir_dict[key][1]) + ' SHOULD BE ' + str(key)
