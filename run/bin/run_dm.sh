#! /bin/sh
#
# DM                This application shell script takes care of directly
#                   starting and stopping DM.
#
RETVAL=0

# App details
APP="dm"

cd `dirname $0`/..
APP_DIR=`pwd`
APP_RUNDIR="${APP_DIR}"
APP_BINDIR="${APP_RUNDIR}/bin"
APP_CFGDIR="${APP_RUNDIR}/params"
APP_LOGDIR="${APP_RUNDIR}/logs"
APP_CMD="${APP_BINDIR}/${APP} ${APP_CFGDIR}/${APP}.conf"
APP_LOG="/app/share/bin/conlog -l ${APP_LOGDIR}/${APP}"

# Location of PID file(s)
PIDDIR="${APP_RUNDIR}/pids"

# Kill running process(es)
kill_proc() {
  RC=2

  # Find pid.
  pid=`pid_of_proc $1 $2`

  # Kill it.
  if [ "$pid" != "" ] ; then
    if ps -p "$pid">/dev/null 2>&1; then
    # TERM first, then KILL if not dead
      kill -TERM $pid 2>/dev/null
      sleep 1
      if ps -p "$pid" >/dev/null 2>&1 ; then
        sleep 1
        if ps -p "$pid" >/dev/null 2>&1 ; then
          sleep 3
          if ps -p "$pid" >/dev/null 2>&1 ; then
            kill -KILL $pid 2>/dev/null
          fi
        fi
      fi
    fi
    ps -p "$pid" >/dev/null 2>&1
    RC=$?
  else
    RC=1
  fi
  # Remove PID file if no more process(es)
  [ $RC = 1 ] && rm -f ${PIDDIR}/$1.pid
  return $RC
}

# Determine state of process(es)
check_proc() {
  RC=2

  # Find pid.
  pid=`pid_of_proc $1 $2`

  if [ "$pid" != "" ] ; then
    RC=0
  else
    RC=1
  fi
  return $RC
}

# Get the PID of running process(es)
pid_of_proc() {
  pid=""

  # Look for PID file
  if [ -f ${PIDDIR}/$1.pid ] ; then
    pid=`tail -n 1 ${PIDDIR}/$1.pid`
  fi

  if [ "$pid" != "" ] ; then
    bup=`ps -o args -p $pid | fgrep "$2"`
    if [ $? != 0 ] ; then
      pid=""
    fi
  fi

  echo $pid
}

# Report application run status based on return code
report_run_status() {

  case "$1" in
  0) echo -n "UP"
     ;;
  1) echo -n "DOWN"
     ;;
  2) echo -n "UNKNOWN"
     ;;
  *) echo -n "ERROR"
     ;;
  esac
}

# Start function
start() {
        echo -n $"Starting ${APP}: "

        # Determine if process is already running.
        pid=`pid_of_proc ${APP} "${APP_CMD}"`

        if [ "$pid" = "" ] ; then
	  # Setup environment before running program.
	  . ${APP_BINDIR}/${APP}_env.sh
	  echo "${APP_CMD} |& ${APP_LOG} &"
          ${APP_CMD} |& ${APP_LOG} &
          PID=`jobs -p`
          RETVAL=$?
          if [ $RETVAL = 0 ] && [ $PID != "" ]; then
            echo $PID > ${PIDDIR}/${APP}.pid
          fi
        fi

        report_run_status $RETVAL
        echo
        return $RETVAL
}

# Stop function
stop() {
        echo -n $"Stopping ${APP}: "

	kill_proc ${APP} "${APP_CMD}"
        RETVAL=$?
        report_run_status $RETVAL

        # Swap 0 and 1 return values, because internally 1 means process stopped
        # but exiting with 0 means successful attempt by script to stop process,
        # and 0 means process not stopped, so exit with 1, meaning unsuccessful.
        case "$RETVAL" in
        0) RETVAL=1
           ;;
        1) RETVAL=0
           ;;
        esac

        echo
        return $RETVAL
}

# Status function
status() {
        echo -n $"Status of ${APP}: "

        check_proc ${APP} "${APP_CMD}"
        RETVAL=$?

        report_run_status $RETVAL
        echo
        return $RETVAL
}

# See how we were called.
case "$1" in
  start)
        start
        ;;
  stop)
        stop
        ;;
  status)
        status
        ;;
  *)
        echo "Usage: $0 {start|stop|status}"
        RETVAL=1
        ;;
esac
exit $RETVAL
