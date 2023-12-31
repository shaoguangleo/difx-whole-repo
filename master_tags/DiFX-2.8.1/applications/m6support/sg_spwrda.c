/*
 * (c) Massachusetts Institute of Technology, 2013..2023
 * (c) Geoffrey B. Crew, 2013..2023
 *
 * $Id: sg_spwrda.c 5755 2023-03-26 16:47:18Z gbc $
 *
 * Code to support thread creation to use readhead to supplement
 *
 * addr is the address relative to the start of mapped memory,
 * and we are interested in len more bytes.
 *
 * NOTION: create a new ADVICE method that creates a pool of threads once
 * and then merely tasks them with the read-ahead work.  More efficient....
 * NOTION: investigate omp.h for cheap thread usage
 *
 * vdifuse_trace is available in this file.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include "vdifuse.h"
//#include "sg_access.h"

#define RHMULT 4

typedef struct pidex { pthread_t tid; pthread_mutex_t tmx; } Pidex;
extern Pidex *sg_advice_pthreads;

/* to hold the work assignment */
typedef struct rh_task_data { int fd; off64_t offset; size_t count; } Rhtd;
typedef struct th_task_data { int f; void *a; size_t c; long p; } Thtd;

/*
 * This call blocks until the requested data is read into memory via
 * the (Linux) readahead(2) system call which does the actual work.
 * Note that pthread_exit() does not return to the caller.  The rv
 * value is formally available via a pthread_join, but we do not care.
 */
void *readahead_task(void *arg)
{
    Rhtd *work = (Rhtd *)arg;
    int rv = readahead(work->fd, work->offset, work->count);
    pthread_exit(&rv);
    return(0);
}

/*
 * Launch a thread to use the readahead system call (which blocks)
 * The thread created does the reads-ahead and exits.
 */
void spawn_readahead_thread(int fd, off_t addr, size_t len, size_t size)
{
    pthread_t tid;
    Rhtd task_data;

    /* save arguments */
    task_data.fd = fd;
    task_data.offset = (off64_t)addr;
    task_data.count = len;

    /* ask for several times as much */
    if (task_data.offset + RHMULT*task_data.count < size)
        task_data.count *= RHMULT;

    (void)pthread_create(&tid, NULL, &readahead_task, &task_data);
    vdiftrace(-1,VDT("RH-%lu on %02d at %lu for %lu\n"),
        tid, task_data.fd, task_data.offset, task_data.count);
}

/*
 * Thread task to touch pages.  We don't care about the
 * result or blocking on page faults, as long as we get the
 * kernel reading ahead (before the real reader catches up).
 * Again the return value is ignored.
 *
 * FIXME:  it is not clear that this toucher method works....
 */
void *toucher_task(void *arg)
{
    Thtd *work = (Thtd *)arg;
    long cnt = work->c, cc, tot = 0L;
    void *addr = work->a;
    size_t page = work->p;
    int ii, f = work->f;
    struct timespec req;

    req.tv_sec  = 0;
    req.tv_nsec = 100;

    for (cc = 0; ++cc < cnt/10; ) {
        pthread_mutex_lock(&sg_advice_pthreads[f].tmx);
        for (ii = 0; ii < 10; addr += page)
            tot += *(char *)addr;
        pthread_mutex_unlock(&sg_advice_pthreads[f].tmx);
        /* open hole for cancellability in sg_advice_term() */
        (void)nanosleep(&req, 0);
    }
    pthread_exit(&tot);
    return(0);
}

/*
 * Launch a thread to step through and touch the pages we expect to use.
 */
void spawn_toucher_thread(int fd, void *addr, size_t len, long page)
{
    pthread_t *tidp = &(sg_advice_pthreads[fd].tid);
    Thtd task_data;

    /* save arguments */
    task_data.f = fd;
    task_data.a = addr;
    task_data.c = len / page;
    task_data.p = page;

    (void)pthread_create(tidp, NULL, &toucher_task, &task_data);
    vdiftrace(-1,VDT("TH-%lu on %02d at %p on %lu pages of %lu\n"),
        *tidp, task_data.f, task_data.a, task_data.c, task_data.p);
}

/*
 * eof
 */
