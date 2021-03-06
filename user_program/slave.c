#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#define INIT_VALUE 99999
#define MAXN 100
int main (int argc, char* argv[])
{
	char buf[BUF_SIZE];
	int i, dev_fd, file_fd;// the fd for the device and the fd for the input file
	size_t ret, file_size = 0, data_size = -1;
    int N, cnt = 1;
	char file_name[MAXN][50];
	char method[20];
	char ip[20];
	struct timeval start;
	struct timeval end;
	double trans_time; //calulate the time between the device is opened and it is closed
	char *kernel_address, *file_address;

    N = atoi(argv[cnt++]);
    for(i = 0; i < N; i++)
        strcpy(file_name[i], argv[cnt++]);
    strcpy(method, argv[cnt++]);
    strcpy(ip, argv[cnt++]);

	gettimeofday(&start ,NULL);
    for(i = 0; i < N; i++){
        file_size = 0;

        if( (dev_fd = open("/dev/slave_device", O_RDWR)) < 0)//should be O_RDWR for PROT_WRITE when mmap()
        {
            perror("failed to open /dev/slave_device\n");
            return 1;
        }

        if(ioctl(dev_fd, 0x12345677, ip) == -1)	//0x12345677 : connect to master in the device
        {
            perror("ioctl create slave socket error\n");
            return 1;
        }

        if( (file_fd = open (file_name[i], O_RDWR | O_CREAT | O_TRUNC)) < 0)
        {
            perror("failed to open input file\n");
            return 1;
        }

        write(1, "ioctl success\n", 14);

        switch(method[0])
        {
            case 'f':{ //fcntl : read()/write()
                do
                {
                    ret = read(dev_fd, buf, sizeof(buf)); // read from the the device
                    write(file_fd, buf, ret); //write to the input file
                    file_size += ret;
                }while(ret > 0);
                break;
            }
            case 'm':{ //mmap
                int recv_len;
                char *input_mem;
                char *output_mem;
                while((recv_len = ioctl(dev_fd, 0x12345678, file_size)) != 0){
                    if(recv_len < 0){
                        perror("receive message error\n");
                        return 1;
                    }
                    //fprintf(stderr, "recv_len = %d\n", recv_len);
                    ftruncate(file_fd, file_size + recv_len);
                    input_mem = mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, dev_fd, file_size);
                    output_mem = mmap(NULL, PAGE_SIZE, PROT_WRITE, MAP_SHARED, file_fd, file_size);
                    memcpy(output_mem, input_mem, recv_len);
                    munmap(input_mem, PAGE_SIZE);
                    munmap(output_mem, PAGE_SIZE);
                    file_size += recv_len;
                    //fprintf(stderr, "now file size = %d\n", file_size);
                }
                break;
           }
        }
        if(ioctl(dev_fd, 0) < 0){
            perror("print page descriptor error\n");
            return;
        }
        if(ioctl(dev_fd, 0x12345679) == -1)// end receiving data, close the connection
        {
            perror("ioctl client exits error\n");
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


