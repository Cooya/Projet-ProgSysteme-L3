#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#include "job.h"

static int currentJob = -1; // currentJob job
static job_t *jobList = NULL; // jobs list
static int jobNumber = 0; // jobs number

/**
* Creating job and attributing job name
* @param name name of job
**/
int job_new( char *name )
{
    
    job_t *pj = NULL;
    // find a free cell in list for new job
    for( int i = 0; i < jobNumber; i++ )
    {
        if( jobList[i].p[0].pid <= 0 )
        {
            pj = &(jobList[i]);
            currentJob = i;
            break;
        }
    }

    if( ! pj )   // there is no free cell in list, need to add new item
    {
        if(jobList==NULL)
            jobList = malloc( sizeof(job_t)*1);
        else
            jobList = realloc( jobList, sizeof( job_t ) * ( jobNumber + 1 ) );
        pj = &(jobList[jobNumber]);
        memset( pj, 0, sizeof( job_t ) );
        currentJob = jobNumber;
        jobNumber++;
    }

    strcpy( jobList[ currentJob ].name, name );
    return currentJob;
    
}

/**
* Binding a PID to the job j
* @param j job number
* @param p pid of the pid to be added
* @param name name of the extern command with args
**/
int job_add_pid( int j, pid_t p, char *name )
{
#ifdef DEBUG
    fprintf( stderr, "%s( %d, %d, %s )\n", __FUNCTION__, j, p, name );
    fflush( stderr );
#endif
    for( int i = 0; i < MAXPIPELINE; i++ )
    {
        if( jobList[j].p[i].pid <= 0 )
        {
            jobList[j].p[i].pid = p;
            strcpy( jobList[j].p[i].fn, name );
            return i;
        }
    }
    return -1;
}


extern int ctrl_c_flag;
extern int ctrl_z_flag;

/**
*
* Waiting and get the exit code of the group of PID of the job j
* @param j job number
**/
int job_wait( int j )
{
#ifdef DEBUG
    fprintf( stderr, "%s( %d )\n", __FUNCTION__, j );
    fflush( stderr );
#endif

    while( 1 )
    {

        if( ctrl_z_flag )
        {
            //stop_signals();
            //fprintf(stdout, "ctrl_z_flag! PID = %d ; SIGSTOP\n",jobList[j].p[0].pid);
            ctrl_z_flag = 0;
            kill( jobList[j].p[0].pid, SIGSTOP);

            jobList[j].p[0].stopped=true;
            //fprintf( stderr, "job %d [%s] stopped", j, jobList[j].name );
            return -1;
        }
        /*
        if ( ctrl_c_flag )
        {
            fprintf(stdout, "Ctrl-C flag; PID = %d \n", jobList[j].p[0].pid);
            ctrl_c_flag = 0;
            kill (jobList[j].p[0].pid, SIGINT);
            return -1;
        }
        */

        int freecells = 0;

        for( int i = MAXPIPELINE-1; i >= 0; i-- )
        {

            if( jobList[j].p[i].pid <= 0 )  // foreach dead childs
            {
                freecells++;
                continue;
            }

            if( freecells >= MAXPIPELINE ) // if no running pid return 0
                return 0;

            int st;
            int p = waitpid( jobList[j].p[i].pid, &st, WNOHANG );
            if( p == jobList[j].p[i].pid ) // if child is dead
            {

                if( WIFSIGNALED( st ) ) // if killed by a signal
                {
                    if( WTERMSIG( st ) == SIGTSTP ) // and it was killed by SIGTSTP signal, then returns exit code as -SIGTSTP
                        return -SIGSTOP;
                }

                /*
                fprintf( stderr, "job %d: process %s (pid: %d) finished; exit code %d\n",
                         j, jobList[j].p[i].fn, jobList[j].p[i].pid, WEXITSTATUS( st ) );
                fflush( stderr );
                */

                // mark it as finished
                jobList[j].p[i].pid = -jobList[j].p[i].pid;
                freecells++;
                // remember exit code
                jobList[j].p[i].exitcode = WEXITSTATUS( st );
            }
            usleep( 10000 );
        }



        if (freecells >= MAXPIPELINE) // || bg)
            break;
    }

    return job_exit_code( j );
}


/**
*
* Checking the child status
*
**/
int job_check_childs()
{
    /*
        fprintf( stderr, "%s()\n", __FUNCTION__ );
        fflush( stderr );
    */

    int jn = jobNumber;
    for( int j = jobNumber-1; j >= 0; j-- ) // foreach childs
    {

        int freecells = 0;
        if( ! jobList[j].name[0] ) // if job has no name, go to next iteration
            continue;

        for( int i = MAXPIPELINE-1; i >= 0; i-- ) // foreach childs
        {

            if( jobList[j].p[i].pid <= 0 ) // count dead childs
            {
                freecells++;
                continue;
            }

            int st;
            int p = waitpid( jobList[j].p[i].pid, &st, WNOHANG );
            if( p == jobList[j].p[i].pid ) // if child is dead
            {

                /*
                fprintf( stderr, "job %d: process %s (pid: %d) finished; exit code %d\n",
                         j, jobList[j].p[i].fn, jobList[j].p[i].pid, WEXITSTATUS( st ) );
                fflush( stderr );
                */

                // mark it as finished
                jobList[j].p[i].pid = -jobList[j].p[i].pid;
                freecells++;
                // remember exit code
                jobList[j].p[i].exitcode = WEXITSTATUS( st );
            }

        }

        if( freecells >= MAXPIPELINE )
        {
            //fprintf( stderr, "job %d [%s] terminated; exit code %d\n", j, jobList[j].name, job_exit_code( j ) );
            jobList[j].name[0] = 0;
            // if job terminated - shrink list
            if( j == currentJob ) currentJob = -1;
            jn--;
        }
    }

    return jn;
}

/**
* Get the exit code of a job group
* @param j job number
**/
int job_exit_code( int j )
{

    if( j < 0 ) return 0;
    // exit code of the job is the exit code of last process
    for( int i = MAXPIPELINE-1; i >= 0; i-- )
    {
        if( jobList[j].p[i].pid != 0 )
        {
            // this is the last process of the job
            return jobList[j].p[i].exitcode;
        }
    }
    return -1; // unknown job or job is not finished;
}

/**
*
* Clean the job list
*
**/
int job_purge()
{
    for( int j = jobNumber-1; j >= 0; j-- )
    {

        int freecells = 0;
        if( ! jobList[j].name[0] )
        {
            continue;
        }

        for( int i = MAXPIPELINE-1; i >= 0; i-- )
        {

            if( jobList[j].p[i].pid <= 0 )
            {
                freecells++;
                continue;
            }
        }

        if( freecells >= MAXPIPELINE )
        {
            jobList[j].name[0] = 0;
            // if job terminated - shrink list and free the cell
            if( j == currentJob ) currentJob = -1;
            jobNumber--;
        }
    }

    return 0;
}

/**
*
* Prints the jobs status
*
**/
int job_jobs()
{

    for( int i = 0; i < jobNumber; i++ )
    {
        // prints the jobs status
        if(jobList[i].p[0].pid > 0)
            printf( "%d: %s [%s]\n", i, ( jobList[i].p[0].stopped ) ? "Stopped" : "Running", jobList[i].name );
        else if(jobList[i].p[0].pid == 0)
            printf( "%d: %s [%s]\n", i, "Terminated", jobList[i].name );
    }

    return 0;
}

/**
*
* Send an SIGINT signal to the job
* @param i job number to interrupt
**/ 
int job_interrupt( int i )
{
    if( i < 0 ) // no currentJob active job
        return -1;
    if( jobList[ i ].p[0].pid < 0 ) // no active process to stop
        return -1;
    return kill( jobList[ i ].p[0].pid, SIGINT );
}

/**
* Sends a SIGSTOP signal
* @param i job number to suspend
**/ 
int job_suspend( int i )
{
    if( i < 0 ) // wrong job number
        return -1;
    if( jobList[ i ].p[0].pid < 0 ) // no active process to stop
        return -1;
    int rc = kill( jobList[ i ].p[0].pid, SIGSTOP );
    if( rc == 0 )   // kill succeeded
    {
        if( i == currentJob ) // if the job we manage is current job we need to interrupt wait loop
            currentJob = -1;
    }
    return rc;
}

/**
*
* Resuming a job
* @param i job number to resume
**/
int job_resume( int i )
{
    //fprintf(stderr, "job_resume( %d )\n", i );
    if( i < 0 ) // incorrect job number
        return 1;
    if( jobList[ i ].p[0].pid < 0 ){ // no active process to resume
        fprintf(stderr, "No active process to resume PID=%d\n",jobList[i].p[0].pid);
        return 0;
    }
    //fprintf(stderr, "PID = %d ; NAME = %s ; \n", jobList[i].p[0].pid, jobList[i].name);
    jobList[i].p[0].stopped = false;
    return kill( jobList[ i ].p[0].pid, SIGCONT );
}

/**
*
* Resumes and puts a job in the foreground
* @param i job number to put in foreground
**/
int job_fg( int i )
{
    int rc = job_resume( i );
    if( rc < 0 )
        return -1;
    if( rc == 0 )
        currentJob = i;
    //fprintf(stderr, "coucou FG going to wait! \n");
    return job_wait(i);
}

/**
*
* Resumes and puts a job in the background
* @param i job number to put in background
**/
int job_bg( int i )
{
    int rc = job_resume( i );
    if (rc < 0)
        return -1;
    currentJob = -1;
    return rc;
}

/**
*
* Returns the current job number
*
**/
int job_current()
{
    return currentJob;
}

/**
*
*   Returns the total job number
*
**/ 
int job_total()
{
    return jobNumber;
}


/*
 * Disown a job
 *
 * @param j job number
 */

int job_disown(int j)
{
    if(jobList[j].p[0].pid>0)
    {
        jobList[j].p[0].pid = -1;
        return EXIT_SUCCESS;
    }
    else
    {
        fprintf(stderr, "-bash : disown: %d: no such job\n", j);
        return EXIT_FAILURE;
    }
}


/*
 *
 * Freeing jobList
 */
void job_uninit()
{
    currentJob=-1;
    jobNumber=0;
    free(jobList);
}
