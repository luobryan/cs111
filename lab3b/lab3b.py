#NAME: Bryan Luo
#EMAIL: luobryan@ucla.edu
#ID: 605303956

import sys
exit_code = 0

INDIR_OFFSET = 12
DOUBLE_INDIR_OFFSET = 12+256
TRIPLE_INDIR_OFFSET = 12+256+256*256

dict_of_inodes_to_parents_dirname_lc_rlc = {} #maps inodes to struct containing parents, direcotry name, link count, ref link count (in that order)
def map_inode_to_data(inode_num, data_type, data):
    if inode_num in dict_of_inodes_to_parents_dirname_lc_rlc:
        if data_type == "parent":
            dict_of_inodes_to_parents_dirname_lc_rlc[inode_num] = [data, dict_of_inodes_to_parents_dirname_lc_rlc[inode_num][1], dict_of_inodes_to_parents_dirname_lc_rlc[inode_num][2],dict_of_inodes_to_parents_dirname_lc_rlc[inode_num][3]]
        elif data_type == "dir_name":
            dict_of_inodes_to_parents_dirname_lc_rlc[inode_num] = [dict_of_inodes_to_parents_dirname_lc_rlc[inode_num][0],data,dict_of_inodes_to_parents_dirname_lc_rlc[inode_num][2],dict_of_inodes_to_parents_dirname_lc_rlc[inode_num][3]]
        elif data_type == "link_count":
            dict_of_inodes_to_parents_dirname_lc_rlc[inode_num] = [dict_of_inodes_to_parents_dirname_lc_rlc[inode_num][0],dict_of_inodes_to_parents_dirname_lc_rlc[inode_num][1],data,dict_of_inodes_to_parents_dirname_lc_rlc[inode_num][3]]
        elif data_type == "ref_link_count":
            dict_of_inodes_to_parents_dirname_lc_rlc[inode_num] = [dict_of_inodes_to_parents_dirname_lc_rlc[inode_num][0],dict_of_inodes_to_parents_dirname_lc_rlc[inode_num][1],dict_of_inodes_to_parents_dirname_lc_rlc[inode_num][2], data]
    else:
        if data_type == "parent":
            dict_of_inodes_to_parents_dirname_lc_rlc[inode_num] = [data, "", 0, 0]
        elif data_type == "dir_name":
            dict_of_inodes_to_parents_dirname_lc_rlc[inode_num] = [0, data, 0, 0]
        elif data_type == "link_count":
            dict_of_inodes_to_parents_dirname_lc_rlc[inode_num] = [0,"",data,0]
        elif data_type == "ref_link_count":
            dict_of_inodes_to_parents_dirname_lc_rlc[inode_num] = [0,"",0,data] 
def main():
    dict_of_blocks = {}
    dict_of_free_blocks = {}
    dict_of_free_inodes = {}
    dict_of_parents_to_inodes = {} 
    block_start = -1
    inode_size = -1
    block_size = -1
    total_num_blocks = -1
    total_num_inodes = -1
    first_non_res_inode = -1

    try:
        text_file = open(sys.argv[1], "r")
    except:
        sys.stderr.write('error in opening file \n')
        exit(1)

    text_file_line_arr = text_file.readlines()
   
    for line in text_file_line_arr:
        line_as_arr = line.split(",")
        if line_as_arr[0] == 'SUPERBLOCK':
            inode_size = int(line_as_arr[4])
            block_size = int(line_as_arr[3])
            total_num_blocks = int(line_as_arr[1])
            total_num_inodes = int(line_as_arr[2])
            first_non_res_inode = int(line_as_arr[7])
        if line_as_arr[0] == 'GROUP':
            block_start = int(int(line_as_arr[8])+inode_size*int(line_as_arr[3])/block_size)
            #first block of inodes + inode size * num inodes / block_size
        if line_as_arr[0] == 'DIRENT':
            map_inode_to_data(int(line_as_arr[3]),"dir_name",line_as_arr[6])
            if int(line_as_arr[3]) not in dict_of_inodes_to_parents_dirname_lc_rlc:
                map_inode_to_data(int(line_as_arr[3]),"ref_link_count",1)
            else:
                map_inode_to_data(int(line_as_arr[3]),"ref_link_count",dict_of_inodes_to_parents_dirname_lc_rlc[int(line_as_arr[3])][3] + 1)
            if int(line_as_arr[3]) < 1 or int(line_as_arr[3]) > total_num_inodes:
                exit_code = 2
                print('DIRECTORY INODE '+line_as_arr[1]+' NAME '+ line_as_arr[6][:-1]+' INVALID INODE '+line_as_arr[3])
            if line_as_arr[6][0:3] == "'.'" and int(line_as_arr[1]) != int(line_as_arr[3]):
                exit_code = 2
                print('DIRECTORY INODE '+line_as_arr[1]+" NAME '.' LINK TO INODE"+line_as_arr[3]+' SHOULD BE '+line_as_arr[1])
            elif line_as_arr[6][0:4] == "'..'":
                dict_of_parents_to_inodes[int(line_as_arr[1])] = int(line_as_arr[3])
            elif line_as_arr[6][0:3] != "'.'":
                map_inode_to_data(int(line_as_arr[3]),"parent",int(line_as_arr[1]))
        if line_as_arr[0] == 'INDIRECT':
            offset = 0
            indir_amount = ""
            if int(line_as_arr[2]) == 1:
                offset = INDIR_OFFSET
                indir_amount = ' INDIRECT'
            if int(line_as_arr[2]) == 2:
                offset = DOUBLE_INDIR_OFFSET
                indir_amount = ' DOUBLE INDIRECT'
            if int(line_as_arr[2]) == 3:
                offset = TRIPLE_INDIR_OFFSET
                indir_amount = ' TRIPLE INDIRECT'
            if int(line_as_arr[5]) < 0 or int(line_as_arr[5]) > total_num_blocks:
                exit_code = 2
                print('INVALID'+indir_amount+' BLOCK '+line_as_arr[5]+' IN INODE '+line_as_arr[1]+ ' AT OFFSET '+offset)
            elif int(line_as_arr[5]) < block_start:
                exit_code = 2
                print('RESERVED'+indir_amount+' BLOCK '+line_as_arr[5]+' IN INODE '+line_as_arr[1]+' AT OFFSET '+offset)
            elif int(line_as_arr[5]) in dict_of_blocks:
                dict_of_blocks[int(line_as_arr[5])].append([int(line_as_arr[2]),offset,int(line_as_arr[2])])
            else:
                dict_of_blocks[int(line_as_arr[5])] = [[int(line_as_arr[2]),offset,int(line_as_arr[2])]]
        if line_as_arr[0] == 'INODE':
            map_inode_to_data(int(line_as_arr[1]),"link_count",int(line_as_arr[6]))
            for i in range(12,27):    
                block_number = int(line_as_arr[i])
                if block_number != 0:
                    offset = 0
                    level = 0
                    indir_amount = ""
                    if i == 24:
                        offset = INDIR_OFFSET
                        level = 1
                        indir_amount = " INDIRECT"
                    elif i == 25:
                        offset = DOUBLE_INDIR_OFFSET
                        level = 2
                        indir_amount = " DOUBLE INDIRECT"
                    elif i == 26:
                        offset = TRIPLE_INDIR_OFFSET
                        level = 3
                        indir_amount = " TRIPLE INDIRECT"
                    if block_number > total_num_blocks or block_number < 0:
                        exit_code = 2
                        print('INVALID' + indir_amount+' BLOCK '+str(block_number)+' IN INODE ' + line_as_arr[1] + ' AT OFFSET ' + str(offset))
                    elif block_number < block_start:
                        exit_code = 2
                        print('RESERVED'+indir_amount+' BLOCK '+str(block_number)+' IN INODE '+line_as_arr[1]+' AT OFFSET '+str(offset))
                    elif block_number in dict_of_blocks:
                        dict_of_blocks[block_number].append([int(line_as_arr[1]),offset,level])
                    else:
                        dict_of_blocks[block_number] = [[int(line_as_arr[1]),offset,level]]
        if line_as_arr[0] == 'BFREE':
            dict_of_free_blocks[int(line_as_arr[1])] = 1
        if line_as_arr[0] == 'IFREE':
            dict_of_free_inodes[int(line_as_arr[1])] = 1
    for i in dict_of_blocks:
        if len(dict_of_blocks[i]) > 1:
            for trio in dict_of_blocks[i]:
                indir_amount = ''
                if trio[2] == 1:
                    indir_amount = ' INDIRECT'
                if trio[2] == 2:
                    indir_amount = ' DOUBLE INDIRECT'
                if trio[2] == 3:
                    indir_amount = ' TRIPLE INDIRECT'
                exit_code = 2
                print('DUPLICATE'+indir_amount+' BLOCK '+str(i)+' IN INODE '+str(trio[0])+' AT OFFSET '+str(trio[1]))
    for i in dict_of_inodes_to_parents_dirname_lc_rlc:
        if dict_of_inodes_to_parents_dirname_lc_rlc[i][2]!=0:
            if i in dict_of_free_inodes:   
                exit_code = 2
                print('ALLOCATED INODE '+str(i)+ ' ON FREELIST')
    for i in range(first_non_res_inode,total_num_inodes):
        if i not in dict_of_free_inodes and (i not in dict_of_inodes_to_parents_dirname_lc_rlc or dict_of_inodes_to_parents_dirname_lc_rlc[i][2]==0):  
            exit_code = 2
            print('UNALLOCATED INODE '+str(i)+' NOT ON FREELIST')
    for i in range(block_start,total_num_blocks-1):
        if i not in dict_of_blocks and i not in dict_of_free_blocks:
            exit_code = 2
            print('UNREFERENCED BLOCK '+str(i))
        elif i in dict_of_free_blocks and i in dict_of_blocks:
            exit_code = 2
            print('ALLOCATED BLOCK '+str(i)+' ON FREELIST')
    for i in dict_of_inodes_to_parents_dirname_lc_rlc:
        if dict_of_inodes_to_parents_dirname_lc_rlc[i][1] != 0:
                if i in dict_of_free_inodes and i in dict_of_inodes_to_parents_dirname_lc_rlc and dict_of_inodes_to_parents_dirname_lc_rlc[i][0] != 0:
                    exit_code = 2
                    print('DIRECTORY INODE '+str(dict_of_inodes_to_parents_dirname_lc_rlc[i][0])+' NAME '+dict_of_inodes_to_parents_dirname_lc_rlc[i][1][:-1]+" UNALLOCATED INODE "+str(i))
    for i in dict_of_parents_to_inodes:
        if i != 2 or dict_of_parents_to_inodes[i] != 2:
            if i == 2:
                exit_code = 2
                print("DIRECTORY INODE 2 NAME '..' LINK TO INODE "+str(dict_of_parents_to_inodes[i])+' SHOULD BE 2')
            elif i not in dict_of_inodes_to_parents_dirname_lc_rlc or dict_of_inodes_to_parents_dirname_lc_rlc[i][0]==0: 
                exit_code = 2
                print("DIRECTORY INODE "+str(dict_of_parents_to_inodes[i])+" NAME '..' LINK TO INODE "+str(i)+" SHOULD BE "+str(dict_of_parents_to_inodes[i]))
            elif dict_of_parents_to_inodes[i] != dict_of_inodes_to_parents_dirname_lc_rlc[i][0]:
                exit_code = 2
                print("DIRECTORY INODE "+str(i)+ " NAME '..' LINK TO INODE "+str(i)+" SHOULD BE "+str(dict_of_inodes_to_parents_dirname_lc_rlc[i][0]))
    for i in dict_of_inodes_to_parents_dirname_lc_rlc:
        if dict_of_inodes_to_parents_dirname_lc_rlc[i][2] != 0:  
            num_links = 0
            if i in dict_of_inodes_to_parents_dirname_lc_rlc:
                num_links = dict_of_inodes_to_parents_dirname_lc_rlc[i][3]
            if num_links != dict_of_inodes_to_parents_dirname_lc_rlc[i][2]:
                exit_code = 2
                print('INODE '+str(i)+' HAS '+str(num_links)+' LINKS BUT LINKCOUNT IS '+str(dict_of_inodes_to_parents_dirname_lc_rlc[i][2]))
    
if __name__ == "__main__":
    main()
    exit(exit_code)
