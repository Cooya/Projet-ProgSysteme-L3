#include <unistd.h>
#include <stdbool.h>

#define MAXPIPELINE 256
#define MAXPROCNAME 256
#define MAXJOBNAME 256


/*
 * Structure of proc_t
 *
 */
typedef struct
{
    char fn[MAXPROCNAME];
    pid_t pid;
    //int fd[2];
    int exitcode;
    bool stopped;
} proc_t;
/*
 * Structure of job_t
 *
 */
typedef struct
{
    char name[MAXJOBNAME];
    proc_t p[MAXPIPELINE];
} job_t;

/*
 * printing running jobs status
 *
 */
int job_jobs();

/*
 * Adding a new job
 *
 */
/* int job_new( int pipe_count, char ***pipe_args ); */
int job_new( char *name );

/*
 * Checking the jobs
 *
 */
int job_check_childs();

/*
 * Interrupting job by id
 *
 */
int job_interrupt( int i );
/*
 * Suspending job by id
 *
 */
int job_suspend( int i );
/*
 * Resuming job by id
 *
 */
int job_resume( int i );
/*
 * Resuming & putting in foreground job by id
 *
 */
int job_fg( int i );
/*
 * Resuming & putting in background job by id
 *
 */
int job_bg( int i );
/*
 * Getting current job number
 *
 */
int job_current();
/*
 * Getting total jobs number
 *
 */
int job_total();

int job_purge();
int job_exit_code( int j );
int job_add_pid(int j, pid_t pid, char* name);
int job_wait(int j); //, bool bg);
