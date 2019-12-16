#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>

#include "i265e.h"

#define I265E_EXT_MAX_NAL_CNT       5

static int startProcess = 0;

typedef struct i265e_extern_bs {
    int bsBufSize;
    uint8_t *bsBuf;
    int bsFd;
    int bsFileSize;
    uint8_t *startPtr;
    uint8_t *endPtr;
    int bsBufOccupy;

    uint8_t *nalBuf;
    unsigned int nalBufOccupy;
    i265e_nal_t *nal;
    int nalCnt;

    /* sync context */
    pthread_cond_t enc_start_cond;
    pthread_mutex_t enc_start_mutex;
    pthread_cond_t enc_end_cond;
    pthread_mutex_t enc_end_mutex;
    int tiggerEncStartFlag;
    int tiggerEncEndFlag;
} i265e_extern_bs_t;

i265e_extern_bs_t *i265e_extern_bs_init(int bsBufSize, char *bsname)
{
    struct stat stat_buf;
    i265e_extern_bs_t *h = calloc(1, sizeof(i265e_extern_bs_t));
    if (h == NULL) {
        printf("i265ext:calloc i265e_extern_bs_t failed\n");
        goto err_calloc_i265e_extern_bs_t;
    }

    h->bsBufSize = bsBufSize;
    h->bsBuf = malloc(h->bsBufSize);
    if (h->bsBuf == NULL) {
        printf("i265ext:malloc bsBuf failed\n");
        goto err_malloc_bsBuf;
    }

    h->bsFd = open(bsname, O_RDONLY);
    if (h->bsFd < 0) {
        printf("i265ext:open %s failed:%s\n", bsname, strerror(errno));
        goto err_open_bsname;
    }

    if (fstat(h->bsFd, &stat_buf) < 0) {
        printf("i265ext:fstat %s failed:%s\n", bsname, strerror(errno));
        goto err_fstat_bsFd;
    }

    h->bsFileSize = stat_buf.st_size;
    h->startPtr = NULL;
    h->endPtr = h->bsBuf;
    h->bsBufOccupy = 0;

    h->nalBuf = NULL;
    h->nalBufOccupy = 0;

    h->nal = calloc(I265E_EXT_MAX_NAL_CNT, sizeof(i265e_nal_t));;
    if (h->nal == NULL) {
        printf("i265ext:calloc h->nal failed:%s\n", strerror(errno));
        goto err_calloc_nal;
    }
    h->nalCnt = 0;

    /* sync context */
    h->tiggerEncStartFlag = 1;
    h->tiggerEncEndFlag = 0;
    pthread_mutex_init(&h->enc_start_mutex, NULL);
    pthread_cond_init(&h->enc_start_cond, NULL);
    pthread_mutex_init(&h->enc_end_mutex, NULL);
    pthread_cond_init(&h->enc_end_cond, NULL);

    return h;

err_calloc_nal:
err_fstat_bsFd:
    close(h->bsFd);
err_open_bsname:
    free(h->bsBuf);
err_malloc_bsBuf:
    free(h);
err_calloc_i265e_extern_bs_t:
    return NULL;
}

void i265e_extern_bs_deinit(i265e_extern_bs_t *h)
{
    if (h) {
        pthread_mutex_destroy(&h->enc_start_mutex);
        pthread_cond_destroy(&h->enc_start_cond);
        pthread_mutex_destroy(&h->enc_end_mutex);
        pthread_cond_destroy(&h->enc_end_cond);
        if (h->nal) free(h->nal);
        if (h->bsFd >= 0) close(h->bsFd);
        if (h->bsBuf) free(h->bsBuf);
        free(h);
    }
}

void i265e_extern_dump_nal(i265e_extern_bs_t *h)
{
	if (h) {
		int i = 0;

		printf("-----------%s(%d) start, h->nalCnt=%d --------\n", __func__, __LINE__, h->nalCnt);
		for (i = 0; i < h->nalCnt; i++) {
			printf("[%d], i_type=%d, p_payload=%p, i_payload=%d\n", i, h->nal[i].i_type, h->nal[i].p_payload, h->nal[i].i_payload);
		}
		printf("-----------%s(%d) end,  h->nalCnt=%d --------\n", __func__, __LINE__, h->nalCnt);
	}
}

int i265e_extern_bs_slice_write(i265e_extern_bs_t *h, uint8_t *nal_buf)
{
    int readCnt = 0;
    h->nalBuf = nal_buf;
    h->nalBufOccupy = 0;
    h->nalCnt = 0;
    memset(h->nal, 0, sizeof(i265e_nal_t));

	while (1) {
        /*fill the h->bsBufSize */
        if ((h->bsBufOccupy <= 5)) {
            readCnt = read(h->bsFd, h->endPtr + h->bsBufOccupy, h->bsBufSize - (h->endPtr + h->bsBufOccupy - h->bsBuf));
            if (readCnt < 0 && errno != EINTR) {
                printf("readCnt=%d, errno=%d:%s\n", readCnt, errno, strerror(errno));
                abort();
            }
            if (readCnt < 0 && errno == EINTR) {
                continue;
            } else if (readCnt == 0) {	//To the EndOfFile
                lseek(h->bsFd, 0, SEEK_SET);
                continue;
            } else { /* readCnt > 0*/
                h->bsBufOccupy += readCnt;
            }
        }


		while (h->bsBufOccupy >= 5) {
			if ((h->endPtr[0] == 0x00) && (h->endPtr[1] == 0x00) && ((h->endPtr[2] == 0x01)
						|| ((h->endPtr[2] == 0x00) && (h->endPtr[3] == 0x01)))) {
                if (h->startPtr == NULL) { // start nal
                    h->startPtr = h->endPtr;
                    if ((h->endPtr[0] == 0x00) && (h->endPtr[1] == 0x00) && (h->endPtr[2] == 0x01)) {
                        /* Init nal info */
                        h->nal[h->nalCnt].i_type = (h->endPtr[3] >> 1) & 0x3f;
                        h->endPtr += 4;
                        h->bsBufOccupy -= 4;
                    } else {
                        h->nal[h->nalCnt].i_type = (h->endPtr[4] >> 1) & 0x3f;
                        h->endPtr += 5;
                        h->bsBufOccupy -= 5;
                    }

                    /* Init nal info */
                    h->nal[h->nalCnt].i_payload = 0;
                    h->nal[h->nalCnt].p_payload = h->nalBuf + h->nalBufOccupy;
                } else { /* end nal */
                    memcpy(h->nalBuf + h->nalBufOccupy, h->startPtr, h->endPtr - h->startPtr);
                    h->nalBufOccupy += h->endPtr - h->startPtr;

                    h->nal[h->nalCnt].i_payload = h->nalBuf + h->nalBufOccupy - h->nal[h->nalCnt].p_payload;
                    h->nalCnt++;
                    h->startPtr = NULL;

                    if ((h->nal[h->nalCnt - 1].i_type == I265E_NAL_CODED_SLICE_IDR_W_RADL)
							|| (h->nal[h->nalCnt - 1].i_type == I265E_NAL_CODED_SLICE_TRAIL_R)) {
                        if (h->bsBufOccupy > 0) {
                            memmove(h->bsBuf, h->endPtr, h->bsBufOccupy);
							h->endPtr = h->bsBuf;
                        }
						i265e_extern_dump_nal(h);
                        return 0;
                    }
                }
            } else {
                h->endPtr++;
                h->bsBufOccupy--;
            }
        }
	}

    return -1;
}

int i265e_extern_bs_enc(i265e_extern_bs_t *h, uint8_t *nal_buf)
{
    pthread_mutex_lock(&h->enc_start_mutex);
    if (h->tiggerEncStartFlag == 0) {
        pthread_cond_wait(&h->enc_start_cond, &h->enc_start_mutex);
    }
    h->tiggerEncStartFlag = 0;
    pthread_mutex_unlock(&h->enc_start_mutex);

    /* to do */
    i265e_extern_bs_slice_write(h, nal_buf);

    pthread_mutex_lock(&h->enc_end_mutex);
    h->tiggerEncEndFlag = 1;
    pthread_cond_signal(&h->enc_end_cond);
    pthread_mutex_unlock(&h->enc_end_mutex);
    return 0;
}

int i265e_extern_bs_get_bitstream(i265e_extern_bs_t *h, i265e_nal_t **pp_nal, int *pi_nal, uint8_t **nal_buf)
{
    pthread_mutex_lock(&h->enc_end_mutex);
    if (h->tiggerEncEndFlag == 0) {
        pthread_cond_wait(&h->enc_end_cond, &h->enc_end_mutex);
    }
    h->tiggerEncEndFlag = 0;
    pthread_mutex_unlock(&h->enc_end_mutex);

    *pp_nal = h->nal;
    *pi_nal = h->nalCnt;
    *nal_buf = h->nalBuf;

    return 0;
}

int i265e_extern_bs_release_bitstream(i265e_extern_bs_t *h)
{
    pthread_mutex_lock(&h->enc_start_mutex);
    h->tiggerEncStartFlag = 1;
    pthread_cond_signal(&h->enc_start_cond);
    pthread_mutex_unlock(&h->enc_start_mutex);

    return 0;
}

void *i265e_extern_bs_enc_thread(void *arg)
{
    void **thread_arg = arg;
    i265e_extern_bs_t *h = thread_arg[0];
    uint8_t *nal_buf = thread_arg[1];

    while (startProcess) {
        i265e_extern_bs_enc(h, nal_buf);
    }
    return NULL;
}


int main(int argc, char *argv[])
{
    int bsBufSize = 0, savecnt = 0;
    char *bsname = NULL;
    char *savename = NULL;
    i265e_extern_bs_t *h = NULL;
    pthread_t tid;
    int errnum = 0;
    int i = 0, j = 0;
    i265e_nal_t *p_nal = NULL;
    int i_nal = 0;
    uint8_t *nal_buf = NULL;
    uint8_t *bs_buf = NULL;
    int save_fd = -1;
    void *thread_arg[2];

    if (argc < 5) {
        printf("Usage:%s bsBufSize savecnt bsname savename\n", argv[0]);
        goto err_invalid_cmdline;
    }

	bsBufSize = atoi(argv[1]);
    savecnt = atoi(argv[2]);
    bsname = argv[3];
    savename = argv[4];
    printf("bsBufSize=%d,savecnt=%d,bsname=%s,savename=%s\n", bsBufSize, savecnt, bsname, savename);

    nal_buf = malloc(bsBufSize);
    if (nal_buf == NULL) {
        printf("malloc nal_buf failed\n");
        goto err_malloc_nal_buf;
    }

    save_fd = open(savename, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (save_fd < 0) {
        printf("open %s failed:%s\n", savename, strerror(errno));
        goto err_open_savename;
    }

    h = i265e_extern_bs_init(bsBufSize, bsname);
    if (h == NULL) {
        printf("i265e_extern_bs_init failed\n");
        goto err_i265e_extern_bs_init;
    }

    thread_arg[0] = h;
    thread_arg[1] = nal_buf;
    startProcess = 1;
    if ((errnum = pthread_create(&tid, NULL, i265e_extern_bs_enc_thread, (void *)thread_arg)) != 0) {
        printf("pthread_create i265e_extern_bs_enc_thread failed:%s\n", strerror(errnum));
        goto err_pthread_create_i265e_extern_bs_enc_thread;
    }

    for (i = 0; i < savecnt; i++) {
        i265e_extern_bs_get_bitstream(h, &p_nal, &i_nal, &bs_buf);
        for (j = 0; j < i_nal; j++) {
            write(save_fd, p_nal[j].p_payload, p_nal[j].i_payload);
        }

        i265e_extern_bs_release_bitstream(h);
    }

    startProcess = 0;
    pthread_join(tid, NULL);
    i265e_extern_bs_deinit(h);

    return 0;

err_pthread_create_i265e_extern_bs_enc_thread:
    i265e_extern_bs_deinit(h);
err_i265e_extern_bs_init:
    close(save_fd);
err_open_savename:
    free(nal_buf);
err_malloc_nal_buf:
err_invalid_cmdline:
    return -1;
}
