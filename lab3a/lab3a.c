/*
NAME: Bryan Luo
EMAIL: luobryan@ucla.edu
ID: 605303956
*/

#include <time.h> 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h> 
#include <stdio.h>
#include <math.h>
#include <signal.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ext2_fs.h"

char* imageName; 
int fd = 0; 

char* convert_time(unsigned int x){
    time_t rawtime = (time_t) x; 
    struct tm* time = gmtime(&rawtime);
    char* str_time = malloc(50); 
    strftime(str_time,1024,"%m/%d/%y %H:%M:%S",time); 
    return str_time; 
}
void print_single_indirect(int blockSize, int blockNum, int offset, int inode_num, int indirection_level){
    if(indirection_level == 0){
        return; 
    }
    int number_of_pointers = blockSize / sizeof(int); 

    for(int ptr = 0; ptr < number_of_pointers; ptr++){
        u_int32_t block;
        pread(fd,&block,sizeof(u_int32_t),blockSize*blockNum+sizeof(u_int32_t)*ptr); 
        if(block!=0){
            offset += ptr; 
            printf("INDIRECT,%d,%d,%d,%d,%d\n",inode_num+1,indirection_level,offset,blockNum,block); 
            print_single_indirect(blockSize,block,offset,inode_num,indirection_level-1); 
        }

    }

}
void print_block_inode_entries_and_inode_sum(int this_group_num, u_int32_t num_blocks_in_group, u_int32_t num_inodes_in_group, int blockSize, u_int32_t block_num_free_block_bitmap, u_int32_t block_num_free_inode_bitmap,u_int32_t inode_table){
    u_int8_t* block_bitmap = malloc(blockSize);
    u_int8_t* inode_bitmap = malloc(blockSize);
    pread(fd,block_bitmap,blockSize,blockSize*block_num_free_block_bitmap); //last argument is offset
    pread(fd,inode_bitmap,blockSize,blockSize*block_num_free_inode_bitmap);

    for(int block_num = 0; block_num < blockSize; block_num++){ 
        for(int offset = 0; offset<8 ;offset++){
            if(((block_bitmap[block_num]>>offset)&1) == 0){
            printf("BFREE,%d\n",(this_group_num*num_blocks_in_group)+8*block_num+offset+1);  
            }
        }
    }

        for(int block_num = 0; block_num < blockSize; block_num++){ 
        for(int offset = 0; offset<8 ;offset++){
            if(((inode_bitmap[block_num]>>offset)&1)==0){
            printf("IFREE,%d\n",(this_group_num*num_inodes_in_group)+8*block_num+offset+1);  
            }
        }
    }

    
    //inode summary
    for(unsigned int inode_num = 1; inode_num < num_inodes_in_group; inode_num++){ //skip 0 
            struct ext2_inode my_inode; 
            pread(fd,&my_inode,sizeof(my_inode),1024+(inode_table-1)*blockSize+sizeof(my_inode)*inode_num);
            if(my_inode.i_links_count!=0 && my_inode.i_mode!=0){
                char file_type; 
                if(S_ISDIR(my_inode.i_mode)){
                    file_type = 'd';
                }
                else if(S_ISREG(my_inode.i_mode)){
                    file_type = 'f';
                }
                else if(S_ISLNK(my_inode.i_mode)){
                    file_type = 's';
                }
                else{
                    file_type = '?';
                }
                printf("INODE,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d",inode_num+1,file_type,(my_inode.i_mode)&0xFFF, my_inode.i_uid,my_inode.i_gid,my_inode.i_links_count,convert_time(my_inode.i_ctime),convert_time(my_inode.i_mtime),convert_time(my_inode.i_atime),my_inode.i_size,my_inode.i_blocks);

                if(!(file_type=='s'&&my_inode.i_size<60)){
                    for(int k = 0; k < 15; k++){
                        printf(",%d",my_inode.i_block[k]);  
                    }
                }

                printf("\n"); 

                if(file_type=='d'){
                    for(int i = 0; i < EXT2_NDIR_BLOCKS; i++){
                        int blockNum = my_inode.i_block[i]; 
                        if(blockNum!=0){
                            struct ext2_dir_entry dirent; 
                            for(int j = 0; j < blockSize; j += dirent.rec_len){
                                pread(fd, &dirent,sizeof(dirent),(blockSize*blockNum)+j); 
                                if(dirent.inode!=0){
                                    printf("DIRENT,%d,%d,%d,%d,%d,'%s'\n", inode_num+1, j, dirent.inode, dirent.rec_len,dirent.name_len,dirent.name); 
                                }
                                
                            }
                        }
        
                    }
                }
                                //now handle indirects 
                int offset = EXT2_NDIR_BLOCKS; 
                print_single_indirect(blockSize,my_inode.i_block[EXT2_IND_BLOCK],offset,inode_num,1);
                offset = offset + blockSize/sizeof(int);
                print_single_indirect(blockSize,my_inode.i_block[EXT2_DIND_BLOCK],offset,inode_num,2);
                offset = offset + (blockSize/sizeof(int))*(blockSize/sizeof(int)); 
                print_single_indirect(blockSize,my_inode.i_block[EXT2_TIND_BLOCK],offset,inode_num,3); 
            }
    }
}
void print_group_summary(int numGroups, int num_blocks_in_group, int num_inodes_in_group, int blockSize){
    for(int i = 0; i < numGroups; i++){ 
        struct ext2_group_desc group; 
        pread(fd,&group,sizeof(group),1024+sizeof(struct ext2_super_block)+(i*sizeof(group))); 
        unsigned int num_free_blocks, num_free_inodes, block_num_free_block_bitmap,block_num_free_inode_bitmap,block_num_first_block_of_inodes = 0; 
        num_free_blocks = group.bg_free_blocks_count; 
        num_free_inodes = group.bg_free_inodes_count; 
        block_num_free_block_bitmap = group.bg_block_bitmap; 
        block_num_free_inode_bitmap = group.bg_inode_bitmap; 
        block_num_first_block_of_inodes = group.bg_inode_table; 
        printf("GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n",i, num_blocks_in_group,num_inodes_in_group,num_free_blocks,num_free_inodes,block_num_free_block_bitmap,block_num_free_inode_bitmap,block_num_first_block_of_inodes);
        print_block_inode_entries_and_inode_sum(i,num_blocks_in_group,num_inodes_in_group,blockSize,block_num_free_block_bitmap,block_num_free_inode_bitmap,block_num_first_block_of_inodes);
   }
}
void print_super_summary(){
  
    unsigned int inodes_count = 0, blocks_count = 0, block_size = 0, inode_size = 0, blocks_per_group = 0, inodes_per_group = 0, first_ino = 0;
    struct ext2_super_block super; 
    pread(fd,&super,sizeof(super),1024); 
    inodes_count = super.s_inodes_count; 
    blocks_count = super.s_blocks_count; 
    inode_size = super.s_inode_size; 
    blocks_per_group = super.s_blocks_per_group; 
    inodes_per_group = super.s_inodes_per_group; 
    block_size = 1024 << super.s_log_block_size; 
    first_ino = super.s_first_ino; 

    printf("SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n",blocks_count,inodes_count,block_size,inode_size,blocks_per_group,inodes_per_group,first_ino);
    


    print_group_summary((blocks_count/blocks_per_group)+1,blocks_count,inodes_per_group,block_size);

}

int main(int argc, char* argv[]){
   if(argc!=2){
	fprintf(stderr,"exactly one argiument required (to specify the img to mount)"); 
	exit(1); 
   }
   if((fd=open(argv[1], O_RDONLY))<0){
	fprintf(stderr,"error in opening the file"); 
	exit(1); 
   }
   imageName = malloc(1024); 
   strcpy(imageName,argv[1]); 
   fd = open(imageName,O_RDONLY); 
   print_super_summary(); 
   close(fd); 
   free(imageName);

   return 0; 
}



