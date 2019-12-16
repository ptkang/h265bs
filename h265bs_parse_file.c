#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUFSIZE		8192

int main(int argc, char *argv[])
{
	int bsfd = -1;
	int nalfd = -1;

	char bsbuf[BUFSIZE];
	int readcnt = 0;
	int nalcnt = 0;
	int leftcnt = 0;

	char *endptr = NULL, *startptr = NULL;
	int naltype = 0;

	if (argc < 2) {
		printf("Usage:%s h265bsfile", argv[0]);
		return -1;
	}

	bsfd = open(argv[1], O_RDONLY);
	if (bsfd < 0) {
		printf("open %s failed\n", argv[1]);
		goto err_open_bsfile;
	}

    startptr = endptr = bsbuf;
	while (1) {
		readcnt = read(bsfd, bsbuf + leftcnt, BUFSIZE - leftcnt);
		if (readcnt <= 0) {
			printf("readcnt = %d\n", readcnt);
			break;
		}

        printf("leftcnt=%d, readcnt=%d\n", leftcnt, readcnt);
		leftcnt += readcnt;

		while (leftcnt >= 5) {
			if ((endptr[0] == 0x00) && (endptr[1] == 0x00) && ((endptr[2] == 0x01)
						|| ((endptr[2] == 0x00) && (endptr[3] == 0x01)))) {
				if (nalcnt > 0) {
					write(nalfd, startptr, endptr - startptr);
					close(nalfd);
                    startptr = endptr;
					nalfd = -1;
				}
				nalcnt++;
				if ((endptr[0] == 0x00) && (endptr[1] == 0x00) && (endptr[2] == 0x01)) {
					naltype = (endptr[3] >> 1) & 0x3f;
					endptr += 4;
					leftcnt -= 4;
				} else {
					naltype = (endptr[4] >> 1) & 0x3f;
					endptr += 5;
					leftcnt -= 5;
				}

				/* reopen the naltype file*/
				char nalname[64];
				sprintf(nalname, "nal%04d_type%d.h265", nalcnt, naltype);
				nalfd = open(nalname, O_RDWR | O_CREAT | O_TRUNC, 0644);
				if (nalfd < 0) {
					printf("open %s failed\n", nalname);
					goto err_open_nalname;
				}

			} else {
				endptr++;
				leftcnt--;
			}
		}

        if (nalfd > 0 && startptr && endptr && endptr - startptr) {
            write(nalfd, startptr, endptr - startptr);
            startptr = endptr;
        }

        if (endptr != NULL && leftcnt > 0) {
            printf("nalcnt=%d, leftcnt=%d, startptr=%p,endptr=%p,bsbufe=%p\n", nalcnt, leftcnt, startptr, endptr, bsbuf + BUFSIZE);
            memmove(bsbuf, endptr, leftcnt);
            startptr = endptr = bsbuf;
        }
	}

    /* last nal */
    if (nalfd > 0) {
        write(nalfd, startptr, leftcnt);
        startptr = endptr = NULL;
        leftcnt = 0;
    }

	if (nalfd >= 0) {
		close(nalfd);
	}
	close(bsfd);


	return 0;

err_open_nalname:
	close(bsfd);
err_open_bsfile:
	return -1;
}
