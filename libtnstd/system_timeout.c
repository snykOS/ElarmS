#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

/* system_timeout() - a system() that will timeout if the process does
	not finish its task within a specified timeout. All zombie children
	will be killed and subsequently reaped. See description below.

	Written by Paul Friberg, ISTI.com

	This is not thread safe.
*/

extern char **environ;

int timed_out=0;

/* private signal handler for alarm signal */
void alarm_sighandler(int i) {
#ifdef DEBUG
        fprintf(stderr, "alarm signal trapped\n");
#endif
        timed_out=1;
}


/* system_timeout() - just like system() except that the child is killed after timeout_secs 
	so this is a non-blocking system()

	RETURNS (exit status of command upon success)
	-1 if the child was killed due to a timeout (using SIGKILL)
	-2 if the child was killed due to some signal other than SIGKILL

	NOTE: this function uses alarm() and traps SIGALARM.
 */

int system_timeout(char *command_str, int timeout_secs) 
{
        pid_t child_pid;
	int retVal;

#ifdef DEBUG
        fprintf(stderr, "Command line to be executed:\n%s\n", command_str);
        fprintf(stderr, "Timeout (secs):%d\n", timeout_secs);
#endif

	timed_out = 0; /* need to initialize this first, before setting alarm */ 
        if (timeout_secs > 0) {
		/* need to mask off child from alarms signal */
                signal(SIGALRM, &alarm_sighandler);
                alarm(timeout_secs);
        }




#ifdef SOLARIS
        if ( (child_pid=fork1()) == -1) 
#else
        if ( (child_pid=fork()) == -1) 
#endif
	{
                alarm(0);
                return -2;	/* fork failed */
        }
        if (child_pid == 0) 
	{
                /* we are the child */
               	char *argv[4];
              	argv[0] = "sh";
               	argv[1] = "-c";
               	argv[2] = command_str;
               	argv[3] = 0;
		/* need to mask off child from alarms signal */
#ifdef DEBUG
        	fprintf(stderr, "In the child, ready to execve\n");
#endif

               	retVal = execve("/bin/sh", argv, environ);

                if (retVal == -1)
                {
                	fprintf (stderr, "  Command execution error: %s\n", strerror(errno));
               		exit(127);
                } 
        } else {
                /* we are the parent, we must wait for the child */
#ifdef DEBUG
        	fprintf(stderr, "In the parent waiting for pid %d to finish\n", child_pid);
#endif

                while (waitpid(child_pid, &retVal, WNOHANG) != child_pid) 
		{
                        sleep(1);
                        if (timed_out==1)
			{
				/* need to kill the child here  and loop again 
					to reap the process */
#ifdef DEBUG
		        	fprintf(stderr, "Child timed out, attempting SIGKILL to child!\n");
#endif
                		alarm(0);
				kill(child_pid, SIGKILL);
				/* return (-1); */
			}
                }

                alarm(0);
		/* the child exited normally without the alarm going off */
		if (WIFEXITED(retVal) != 0) {
			/* the child exited normally without the alarm going off */
#ifdef DEBUG
        		fprintf(stderr, "Child exited normally with exit value = %d\n", WEXITSTATUS(retVal));
#endif
			return WEXITSTATUS(retVal);
		}
		if (WIFSIGNALED(retVal)) 
		{
			int causative_signal;
			
			causative_signal = WTERMSIG(retVal);
#ifdef DEBUG
        		fprintf(stderr, "Child exited abnormally because of signal = %d\n",
					causative_signal);
#endif
			if (timed_out==1 && causative_signal == SIGKILL) {
#ifdef DEBUG
        			fprintf(stderr, "Child was SIGKILL killed due to timeout\n");
#endif
				return -1;	/* timed out */
			} else {
				return -2;	/* something else killed it */
			}
		}
        }
        alarm(0);
        if (timed_out==1) 
	{
#ifdef DEBUG
        	fprintf(stderr, "Child exited abnormally and timed out\n");
#endif
                return -1;
        }

        return  0;
}

/* this function is the exact same one as system_timeout() above except that stdout and stderr
	are redirected to files.
*/
int system_timeout2(char *command_str, int timeout_secs, char *stdout_file, char *stderr_file) 
{
        pid_t child_pid;
	int retVal;

#ifdef DEBUG
        fprintf(stderr, "Command line to be executed:\n%s\n", command_str);
        fprintf(stderr, "Timeout (secs):%d\n", timeout_secs);
#endif

	timed_out = 0; /* need to initialize this first, before setting alarm */ 
        if (timeout_secs > 0) {
		/* need to mask off child from alarms signal */
                signal(SIGALRM, &alarm_sighandler);
                alarm(timeout_secs);
        }




#ifdef SOLARIS
        if ( (child_pid=fork1()) == -1) 
#else
        if ( (child_pid=fork()) == -1) 
#endif
	{
                alarm(0);
                return -2;	/* fork failed */
        }
        if (child_pid == 0) 
	{
               	char *argv[4];
		int dup_stdout, dup_stderr;
		/* dup stdout and stderr and reopen as files */
		if ((dup_stdout = dup(fileno(stdout))) == -1) 
		{
                	fprintf (stderr, "system_timeout2():  dup() of stdout error: %s\n", strerror(errno));
               		exit(127);
		}
		if (freopen(stdout_file,"w",stdout) == NULL) 
		{
        		fclose(stdout);
        		fdopen(dup_stdout,"w");
                	fprintf (stderr, "system_timeout2():  FAILURE in freopen() of stdout file: %s\n", stdout_file);
		}
		if ((dup_stderr = dup(fileno(stderr))) == -1) 
		{
                	fprintf (stderr, "system_timeout2():  dup() of stderr error: %s\n", strerror(errno));
               		exit(127);
		}
		if (freopen(stderr_file,"w",stderr) == NULL) 
		{
        		fclose(stderr);
        		fdopen(dup_stderr,"w");
                	fprintf (stderr, "system_timeout2():  FAILURE in freopen() of stderr file: %s\n", stderr_file);
		}
		
                /* we are the child */
              	argv[0] = "sh";
               	argv[1] = "-c";
               	argv[2] = command_str;
               	argv[3] = 0;
		/* need to mask off child from alarms signal */
#ifdef DEBUG
        	fprintf(stderr, "In the child, ready to execve\n");
#endif

               	retVal = execve("/bin/sh", argv, environ);

                if (retVal == -1)
                {
                	fprintf (stderr, "  Command execution error: %s\n", strerror(errno));
               		exit(127);
                } 
        } else {
                /* we are the parent, we must wait for the child */
#ifdef DEBUG
        	fprintf(stderr, "In the parent waiting for pid %d to finish\n", child_pid);
#endif

                while (waitpid(child_pid, &retVal, WNOHANG) != child_pid) 
		{
                        sleep(1);
                        if (timed_out==1)
			{
				/* need to kill the child here  and loop again 
					to reap the process */
#ifdef DEBUG
		        	fprintf(stderr, "Child timed out, attempting SIGKILL to child!\n");
#endif
                		alarm(0);
				kill(child_pid, SIGKILL);
				/* return (-1); */
			}
                }

                alarm(0);
		/* the child exited normally without the alarm going off */
		if (WIFEXITED(retVal) != 0) {
			/* the child exited normally without the alarm going off */
#ifdef DEBUG
        		fprintf(stderr, "Child exited normally with exit value = %d\n", WEXITSTATUS(retVal));
#endif
			return WEXITSTATUS(retVal);
		}
		if (WIFSIGNALED(retVal)) 
		{
			int causative_signal;
			
			causative_signal = WTERMSIG(retVal);
#ifdef DEBUG
        		fprintf(stderr, "Child exited abnormally because of signal = %d\n",
					causative_signal);
#endif
			if (timed_out==1 && causative_signal == SIGKILL) {
#ifdef DEBUG
        			fprintf(stderr, "Child was SIGKILL killed due to timeout\n");
#endif
				return -1;	/* timed out */
			} else {
				return -2;	/* something else killed it */
			}
		}
        }
        alarm(0);
        if (timed_out==1) 
	{
#ifdef DEBUG
        	fprintf(stderr, "Child exited abnormally and timed out\n");
#endif
                return -1;
        }

        return  0;
}
