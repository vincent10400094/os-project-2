#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#define PAGE_SIZE 4096
#define BUF_SIZE 512
#define MAXN 100
size_t get_filesize(const char* filename);//get the size of the input file


int main (int argc, char* argv[])
{
	char buf[BUF_SIZE];
	int i, dev_fd, file_fd;// the fd for the device and the fd for the input file
	size_t ret, file_size, offset = 0, tmp;
    int N, cnt = 1;
	char file_name[MAXN][50], method[20];
	char *kernel_address = NULL, *file_address = NULL;
	struct timeval start;
	struct timeval end;
	double trans_time; //calulate the time between the device is opened and it is closed
    
    N = atoi(argv[cnt++]);
    for(i = 0; i < N; i++)
        strcpy(file_name[i], argv[cnt++]);
    strcpy(method, argv[cnt++]);

	gettimeofday(&start ,NULL);
    for(i = 0; i < N; i++){
        offset = 0;
        if( (dev_fd = open("/dev/master_device", O_RDWR)) < 0)
        {
            perror("failed to open /dev/master_device\n");
            return 1;
        }

        if(ioctl(dev_fd, 0x12345677) == -1) //0x12345677 : create socket and accept the connection from the slave
        {
            perror("ioclt server create socket error\n");
            return 1;
        }

        if( (file_fd = open(file_name[i], O_RDWR)) < 0 )
        {
            perror("failed to open input file\n");
            return 1;
        }

        if( (file_size = get_filesize(file_name[i])) < 0)
        {
            perror("failed to get filesize\n");
            return 1;
        }

        switch(method[0])
        {
            case 'f': //fcntl : read()/write()
                do
                {
                    ret = read(file_fd, buf, sizeof(buf)); // read from the input file
                    //fprintf(stderr, "read %d bytes\n", ret);
                    write(dev_fd, buf, ret);//write to the the device
                }while(ret > 0);
                break;
            case 'm':{ //mmap
                int send_len;
                char *input_mem;
                char *output_mem;
                while(offset < file_size){
                    send_len = ((file_size - offset) > PAGE_SIZE)? PAGE_SIZE : (file_size - offset);
                    // fprintf(stderr, "data len: %d\n", send_len);
                    input_mem = mmap(NULL, send_len, PROT_READ, MAP_SHARED, file_fd, offset);
                    // fprintf(stderr, "to send is %s\n", input_mem);
                    output_mem = mmap(NULL, send_len, PROT_WRITE, MAP_SHARED, dev_fd, offset);
                    memcpy(output_mem, input_mem, send_len);
                    send_len = ioctl(dev_fd, 0x12345678, send_len);
                    offset += send_len;
                    munmap(input_mem, send_len);
                    munmap(output_mem, send_len);
                }
                break;
            }
        }
        if(ioctl(dev_fd, 0) < 0){
            perror("print page descriptor error\n");
            return 1;
        }
        if(ioctl(dev_fd, 0x12345679) == -1) // end sending data, close the connection
        {
            perror("ioclt server exits error\n");
            return 1;
        }
	    close(file_fd);
	    close(dev_fd);
    }

    gettimeofday(&end, NULL);
    trans_time = (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)*0.001;
    printf("Transmission time: %lf ms, File size: %d bytes\n", trans_time, file_size);

	return 0;
}

size_t get_filesize(const char* filename)
{
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}
