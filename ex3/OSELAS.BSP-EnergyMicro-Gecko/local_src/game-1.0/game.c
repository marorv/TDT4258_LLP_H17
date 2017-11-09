#include <stdio.h>
#include <stdlib.h>

#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> //For close
#include <fcntl.h> 	//For open
#include <sys/mman.h> //For memory map
#include <sys/ioctl.h> //For ioctl
#include <stdint.h> //For ioctl
#include <inttypes.h> // for printing hex numbers

#define FILEPATH "/dev/fb0"
#define WIDTH 320
#define HEIGHT 240
#define NUMPIXELS  (76799) //320*240-1
#define FILESIZE (153598) //16 bits per pixel

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[])
{		

	printf("Hello World, I'm game!\n");

	uint16_t *map;
	int fd;
	int gpio_fd;
	struct stat sb;
	off_t offset, pa_offset;
	struct fb_copyarea area;
	uint32_t ch, write_buf[100];
	uint32_t read_buf;


	/*The following program prints part of the file specified in its first
       command-line argument to standard output.  The range of bytes to be
       printed is specified via offset and length values in the second and
       third command-line arguments.  The program creates a memory mapping
       of the required pages of the file and then uses write(2) to output
       the desired bytes.
	*/
	gpio_fd = open("/dev/GPIO_buttons", O_RDWR);
	if (gpio_fd == -1)
	   handle_error("open");
	printf("%s\n", "Opened GPIO_buttons");

	fd = open("/dev/fb0", O_RDWR);
	if (fd == -1)
	   handle_error("open");

	if (fstat(fd, &sb) == -1)           /* To obtain file size */
	   handle_error("fstat");

	offset = 0; 
	pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
	   /* offset for mmap() must be page aligned */

	/*
	if (offset >= sb.st_size) {
	   fprintf(stderr, "offset is past end of file\n");
	   exit(EXIT_FAILURE);
	}
	*/

	/* No length arg ==> display to end of file */
	//length = sb.st_size - offset;

	map = (uint16_t*)mmap(NULL, FILESIZE, PROT_READ, MAP_PRIVATE, fd, 0);
	if (map == MAP_FAILED)
	   handle_error("mmap");

	/* DO NOT WRITE
	s = write(STDOUT_FILENO, map + offset - pa_offset, length);
	if (s != length) {
	   if (s == -1)
	       handle_error("write");

	   fprintf(stderr, "partial write");
	   exit(EXIT_FAILURE);
	}
	*/
	char prev_input = 0;
	int k = 1;
	int j = 0;
	int i;
	while(j++ < 1000)
	{
		read(gpio_fd, &read_buf, sizeof(read_buf));

		uint16_t buttons_pressed = read_buf << 8;

		printf ("The data in the device is %x\n", buttons_pressed);
		if (buttons_pressed == 0xfe00)
		{
			printf("%s\n", "Making a line");
			for (i = 0; i < WIDTH*k - 1; i++)
			{
				map[i] = 0xF800;
			}
			k++;

			//Update display
			printf("%s\n", "Updating display");
			area.dx = 0;
			area.dy = 0;
			area.width = WIDTH;
			area.height = HEIGHT;
			ioctl(fd, 0x4680, &area);
		}	
	}




	munmap(map, FILESIZE);
	close(fd);
	close(gpio_fd);

	exit(EXIT_SUCCESS);

}
