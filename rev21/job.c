#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#include "job.h"

static int current = -1; // current job
static job_t *jlist = NULL; // jobs list
static int jnum = 0; // jobs number


int job_new( int pipe_count, char ***pipe_args ) {

   job_t *pj = NULL;

#ifdef DEBUG
   for( int i = 0; i < pipe_count; i++ ) {
      int c = 0;
      while( pipe_args[i][c] ) {
       printf( "%d: %d: %s\n", i, c, pipe_args[i][c] );
       c++;
    }
 }
#endif

   // find a free cell in list for new job 
 for( int i = 0; i < jnum; i++ ) {
   if( jlist[i].p[0].pid <= 0 ) {
    pj = &(jlist[i]);
    current = i;
    break;
 }
}

   if( ! pj ) { // there is no free cell in list, need to add new item
      jlist = realloc( jlist, sizeof( job_t ) * ( jnum + 1 ) );
      pj = &(jlist[jnum]);
      memset( pj, 0, sizeof( job_t ) );
      current = jnum;
      jnum++;
   }

   int c = 0; // process counter 
   // make the proper pipeline sequence 
   for( int i = pipe_count-1; i >= 0; i-- )
   {
      char **args = pipe_args[i];
      printf( "args: [%s]\n", args[0] );

      // create pipes for child process 
      pipe( pj->p[c].fd );

      // make dedicated process 
      pj->p[c].pid = fork();
      if( pj->p[c].pid < 0 ) {
       exit( errno );
    }

    if( pj->p[c].pid == 0 ) {
    // child process 

       if( c != 0 ) {
       // for 1-st process we don't need to redirect stdin 

       // close current stdin
          close( STDIN_FILENO );
       // redirecting stdout of previous process to current process stdin
          dup2( pj->p[ c - 1 ].fd[0], STDIN_FILENO );
       }

       if( c < (pipe_count-1) ) {
       // for last process we don't need to redirect stdout

       // if not not last, redirect standard output 
       // close stdout
          close( STDOUT_FILENO );
       // redirecting stdout of current process to stdin of the next process
          dup2( pj->p[c].fd[1], STDOUT_FILENO );
       }

    // replace current process with external program 
       execvp( args[0], args );


    // error executing external program
       int e = errno;
       fprintf(stderr, "\"%s\": %s\n", args[0], strerror( e ) );

       exit( e );

    } else {

    // parent 
    close( pj->p[c].fd[1] ); // close write end of the pipe 

    // make process name 
    strcpy( pj->p[c].fn, args[0] );

    c++; // one more child

    // make job name 
    if( pj->name[0] )
      strcat( pj->name, " | " );
   strcat( pj->name, args[0] );
}
}

return 0;
}


int job_check_childs() {

   int jn = jnum; 
   for( int j = jnum-1; j >= 0; j-- ) {

      int freecells = 0;
      if( ! jlist[j].name[0] )
         continue;
      
      for( int i = MAXPIPELINE-1; i >= 0; i-- ) {

       if( jlist[j].p[i].pid <= 0 ) { // count number of process
          freecells++;
          continue;
       }

       int st;
       int p = waitpid( jlist[j].p[i].pid, &st, WNOHANG );
       if( p == jlist[j].p[i].pid ) {

       // child is dead 
          if( jn > 1 ) {
          // if there was a lot of jobs 
             printf( "job %d: process %s (pid: %d) finished; exit code %d\n",
               j, jlist[j].p[i].fn, jlist[j].p[i].pid, WIFEXITED( st ) );
          } else {
          // only one job
             printf( "process %s (pid: %d) finished; exit code %d\n",
               jlist[j].p[i].fn, jlist[j].p[i].pid, WIFEXITED( st ) );
          }
       // mark it as finished 
          jlist[j].p[i].pid = -1;
          freecells++;
          if( i == current )
            current = -1;
       }

    }

    if( freecells >= MAXPIPELINE ) {
       jlist[j].name[0] = 0;
    // if job terminated - shrink list 
       jnum--;
    }
 }

 return jnum;
}


int job_jobs() {

   for( int i = 0; i < jnum; i++ ) {
      // check the first element of pipeline, if it exists - job is not finished 
      printf( "%d: %s %s\n", i, ( jlist[i].p[0].pid > 0 ) ? "running" : "terminated", jlist[i].name );
   }

   return 0;
}


int job_interrupt( int i ) {
   if( i < 0 ) // no current active job 
      return 1; 
   if( jlist[ i ].p[0].pid < 0 ) // no active process to stop 
      return 1; 
   return kill( jlist[ i ].p[0].pid, SIGINT );
}


int job_suspend( int i ) {
   if( i < 0 ) // no current active job 
      return 1; 
   if( jlist[ i ].p[0].pid < 0 ) // no active process to stop 
      return 1; 
   int rc = kill( jlist[ i ].p[0].pid, SIGSTOP );
   if( rc == 0 ) { // kill succeeded

   if( i == current ) // if the job we manage is current job we need to interrupt wait loop 
      current = -1;
}
return rc;
}


int job_resume( int i ) {
   if( i < 0 ) // incorrect job number 
      return 1; 
   if( jlist[ i ].p[0].pid < 0 ) // no active process to resume 
      return 0; 
   return kill( jlist[ i ].p[0].pid, SIGCONT );
}


int job_fg( int i ) {
   int rc = job_resume( i );
   if( rc == 0 )
      current = i;
   return rc;
}


int job_bg( int i ) {
   int rc = job_resume( i );
   current = -1;
   return rc;
}


int job_current() {
   return current;
}


int job_total() {
   return jnum;
}
