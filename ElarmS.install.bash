#!/bin/bash
#
# Code for installing Epic
# by Ran N. Nof @ GSI, 2019
#
# Usage: $0 {all|earthworm|activemq|epic|elvis}


EEWSDIR=`echo $PWD`
echo "Hi ${USER}"

test_internet(){
  # Make sure we have internet connection
  # trying to ping to apache.org server as a test.
  wget --spider -q http://archive.apache.org
  if [ $? -ne 0 ]; then
    echo "ERROR. Some problem with internet connection."
    exit 1
  fi
return 0
}

install_dependencies(){
  ##### dependencies
  echo "Updating Dependencies"
  sudo apt-get update &> update.log && sudo apt-get -y install make autoconf automake libtool libssl-dev gcc gfortran g++ gcc-multilib default-jre default-jdk libxerces-c-dev zlib1g-dev git uuid-dev libcppunit-dev libapr1 libapr1-dev python-pip mailutils curl vim screen gawk 2>&1 | tee dependencies_install.log
  if [ $? -ne 0 ]; then
    echo "Failed to update system"
    return 1
  fi
return 0
}

install_java(){
  # install Java
  which java > /dev/null
  if [ $? -ne 0 ]; then
    echo "Installing Java"
    sudo add-apt-repository ppa:webupd8team/java -y 2>&1 | tee -a dependencies_install.log
    sudo apt-get update 2>&1 | tee -a dependencies_install.log
    sudo apt-get install -y oracle-java8-installer | tee -a dependencies_install.log
    sudo apt-get install -y oracle-java8-set-default | tee -a dependencies_install.log
  else
    echo "Java is already installed"
  fi
  which java > /dev/null
  return $?
}


make_dirs(){
  cd ${EEWSDIR}
  ##### Directories
  echo "Creating Elarms directories"
  mkdir -p run
  mkdir -p run/params
  mkdir -p run/log
  mkdir -p run/log/EWP2
  mkdir -p run/log/EWP2r
  mkdir -p run/log/E2
  mkdir -p run/log/E2r
  mkdir -p run/log/DM
  mkdir -p run/data
  mkdir -p run/bin
  mkdir -p run/lib
}


###### EARTHWORM
EWDIR="earthworm-working"
get_earthworm(){
  cd ${EEWSDIR}
  # check if earthworm is included in package
  EWPAGE="http://www.earthwormcentral.org/distribution/" # EW distro page
  if [ -e earthworm*gz ]; then
    EWFILE=`ls -t earthworm*gz | awk '{print $0; exit}'`
  else
  # or try to download
    echo "Getting current Earthworm file from ${EWPAGE}"
    EWFILE=`curl -s ${EWPAGE}  | grep earthworm | sed 's/href=\"/\n/;s/<\/a>/\n/' | grep earthworm  | sed 's/".*//' | grep 'src.*gz' | sort -r | awk '{print $0;exit}'`
    echo "Downloading current Earthworm file ${EWFILE}"
    wget ${EWPAGE}${EWFILE}
  fi
  EWDIR=`tar -tvf ${EWFILE} | awk '{print $6; exit}' | sed 's/\///'`
  if [ -e ${EWFILE} ] && [ ! -e ${EWDIR} ]; then
    echo "Unpacking ${EWFILE}"
    tar -xf ${EWFILE}
  fi
}

install_earthworm(){
  cd ${EEWSDIR}
  if [ ! -e ${EWDIR} ]; then
	  get_earthworm
  fi
  echo "Exporting variables"
  export EW_INSTALL_HOME=${EEWSDIR}
  export EW_INSTALL_VERSION=${EWDIR}
  export EW_HOME=${EEWSDIR}
  export EW_LOG=${EEWSDIR}/run/log
  export EW_PARAMS=${EEWSDIR}/run/params
  export EW_DATA_DIR=${EEWSDIR}/run/data
  if [ "`ls ${EWDIR}/lib/* | wc -l`" -eq "1" ]; then
    echo "Setting earthworm environment parameters"
    cd ${EWDIR}
    # update for 64bits
    #sed -i 's/export EW_BITS=.*/export EW_BITS=64/' environment/ew_linux.bash
    # update EW version
    sed -i 's/#export EW_VERSION=.*/export EW_VERSION="'${EWDIR}'"/' environment/ew_linux.bash
    # source parameters
    . environment/ew_linux.bash
    cd src
    # make sure logit.o is compiled too
    sed -i 's/#\(.*logit.o\)/\1/' libsrc/util/makefile.unix
    sed -i 'N;/\n\s*logit.o/{s/\n/ \\\n/};P;D' libsrc/util/makefile.unix
    echo "Making EarthWorm. check earthworm_make.log"
    echo "Compiling..."
    make unix &> ${EEWSDIR}/earthworm_make.log
    if [ $? -ne 0 ]; then
      echo "ERROR. Failed to compile Earthworm."
    fi
    echo "Done."
    cp ../environment/ew_linux.bash $EEWSDIR/run/params/.
  else
    . ${EWDIR}/environment/ew_linux.bash
  fi
  echo "Adding environment variables to .bashrc as needed"
  grep EW_HOME ~/.bashrc &> /dev/null
  if [ $? -ne 0 ]; then
    echo "export EW_HOME=\"${EW_HOME}\"" >> ~/.bashrc
  fi
  grep EW_INSTALL_HOME ~/.bashrc &> /dev/null
  if [ $? -ne 0 ]; then
    echo "export EW_INSTALL_HOME=\"${EW_INSTALL_HOME}\"" >> ~/.bashrc
  fi
  grep EW_RUN_DIR ~/.bashrc &> /dev/null
  if [ $? -ne 0 ]; then
    echo "export EW_RUN_DIR=\"${EW_INSTALL_HOME}/run\"" >> ~/.bashrc
  fi
  echo "Earthworm shold be able to run now."
  cd ${EEWSDIR}
}


##### Java activeMQ
install_activemq(){
  cd ${EEWSDIR}
  echo "Installing activemq Java software"
  if [ ! -e activemq/bin ]; then
    # get latest version
    amqver=`wget -qO- http://archive.apache.org/dist/activemq/ | grep "\[DIR\]" | grep -v activemq | grep -v apache | sed 's/.*\(a href=\"\)\(.*\)\/\">.*/\2/' | awk -F"." 'BEGIN{lastver="";lastval=0};{curval=$1*1000000+$2*1000+$3;lastver=(lastval>curval)?lastver:$0;lastval=(lastval>curval)?lastval:curval};END{print lastver}'`
    # download activemq server
    if [ ! -e apache-activemq-${amqver}-bin.tar.gz ]; then
      wget http://archive.apache.org/dist/activemq/${amqver}/apache-activemq-${amqver}-bin.tar.gz
    fi
    # extract activemq server
    mkdir -p activemq
    tar -C activemq --strip-components 1 -xf apache-activemq-${amqver}-bin.tar.gz
  else
    echo "ActiveMQ Java server is installed. delete activemq/bin folder to force reinstall"
  fi
  export ACTIVEMQ_JAR_DIR=${EEWSDIR}/activemq/lib
  cd ${EEWSDIR}
}

## update ActiveMQ ssl keys
#cd ${EW_INSTALL_HOME}/activemq/conf
## remove old cert files
#rm broker.ks broker_cert client.ks client.ts
## Using keytool,  create a certificate for the broker
#keytool -genkey -alias broker -keystore broker.ks
## Export the broker's certificate so it can be shared with clients
#keytool -export -alias broker -keystore broker.ks -file broker_cert
## create a certificate/keystore for client
#keytool -genkey -alias client -keystore client.ks
## Create a truststore for the client and import the broker's certificate
## This establishes that the client "trusts" the broker
#keytool -import -alias broker -keystore client.ts -file broker_cert
## Copy client.ts to Application (i.e. USERDISPLAY) ssl.ts
#cp client.ts ssl.ts
#cd -

##### CPP activeMQ
install_activemq_cpp(){
  # checking for libactivmq-cpp.so
  cd ${EEWSDIR}
  ldconfig -p | grep libactivemq-cpp.so &> /dev/null
  if [ $? -ne 0 ]; then
    if [ ! -e activemq-cpp ]; then
      echo "Downloading activemq CPP libs"
      git clone http://github.com/apache/activemq-cpp.git
    fi
    cd activemq-cpp/activemq-cpp
    echo "Compiling activemq CPP libs..."
    ./build.sh configure &> ${EEWSDIR}/activemq-cpp_configure.log
    ./build.sh compile &> ${EEWSDIR}/activemq-cpp_compile.log
    echo "Installing activemq CPP libs..."
    cd build
    sudo make install &> ${EEWSDIR}/activemq-cpp_install.log
    sudo ldconfig
  else
    echo "activemq-cpp is installed."
  fi  
  amqcppver=`ls -td /usr/local/include/activemq-cpp* | awk '{print $0; exit}' | cut -d- -f3`
  echo activemq-cpp is version $amqcppver
  cd ${EEWSDIR}
}

##### EPIC: EWP2, E2, DM
install_epic(){
  install_earthworm	
  install_activemq_cpp
  cd ${EEWSDIR}/trunk
  # update Make.include.Linux for versions and path
  sed -i 's/include\/apr-1$/include\/apr-1.0/' Make.include.Linux
  sed -i 's/^EWDIR.*/EWDIR=\$(EW_HOME)\/'$EWDIR'/' Make.include.Linux
  sed -i 's/^# ACTIVEMQ$/ACTIVEMQ=\/usr\/local/' Make.include.Linux
  sed -i 's/activemq-cpp-.*/activemq-cpp-'${amqcppver}'/' Make.include.Linux
  sed -i 's/-Wall -Wextra/-Wall -Wextra -fpermissive/' Make.include.Linux
  # apply any additional patches
  for P in `ls ${EEWSDIR}/patches/*patch`; do
    echo "applying patche: ${P}"
    git apply ${P} &>> patches.log
    if [ $? -ne 0 ]; then
        echo "Error. Failed to apply patch: ${P}"
    fi
  done
  export JAVA_TOOL_OPTIONS="-Dfile.encoding=UTF8"
  # source BUILD-TOOLS environment
  . SOURCE_ME
  # compile
  echo "Compiling EPIC. check epic_make.log and decimod_make.log"
  echo "Compiling..."
  make epic &> ${EEWSDIR}/epic_make.log
  if [ $? -ne 0 ]; then
    echo "Error. Failed to compile Epic"
  fi
  make dm &> ${EEWSDIR}/decimod_make.log
  if [ $? -ne 0 ]; then
    echo "Error. Failed to compile Decimod"
  fi
  echo "Done."
  # install
  export INSTALL_ELARMS_BIN_DIR=${EEWSDIR}/run/bin
  export INSTALL_ELARMS_LIB_DIR=${EEWSDIR}/run/lib
  export INSTALL_DM_BIN_DIR=${EEWSDIR}/run/bin
  echo "Installing EPIC. check epic_install.log and decimod_install.log"
  echo "Installing..."
  make install-epic &> ${EEWSDIR}/epic_install.log
  if [ $? -ne 0 ]; then
    echo "Error. Failed to install Epic"
  fi  
  make install-dm &> ${EEWSDIR}/decimod_install.log
  if [ $? -ne 0 ]; then
    echo "Error. Failed to install Decimod"
  fi
  echo "Done."
  cd ${EEWSDIR}
  ## update E2Email if available
  if [ -e run/bin/E2Email ]; then
    sed -i "s|/app/elarms|${EEWSDIR}|;s|/bin/mailx|`which mailx`|" run/bin/E2Email
    #sed -i 's|E2tmp|tmp/E2tmp|' run/bin/E2Email
    chmod +x run/bin/E2Email
  fi
  ## params update
  cd ${EEWSDIR}/run/bin
  export `set | grep ^HOSTNAME=`
  for tmp in `ls -d *tmp`; do 
    f=`basename $tmp .tmp`; 
    if [ -e $f ]; then
	echo "WARNING $f already exists."; 
    else 
	echo "making $f.tmp"; 
	envsubst < $tmp > $f;
    fi; 
  done
  # E2Email is no longer available
  ## tmpdir for alerts 
  #mkdir -p `grep TmpDir E2.conf | awk '{print $2}'`
  cd ${EEWSDIR}
  echo 'Elarms is installed, but config files needs to be edit!'
  echo 'run/bin/*.cfg.tmp'
  echo 'run/params/*.tmp'
}


##### ElViS
install_elvis(){
  cd ${EEWSDIR}
  if [ ! -e ElViS ]; then
    echo "Installing ElViS"
    git clone https://github.com/rannof/ElViS.git
    mkdir -p ElViS/maps
  fi
  cd ElViS
  sudo apt-get install -y ubuntu-gnome-desktop &> ${EEWSDIR}/ElViS_install.log
  # For ubuntu 16.04:
  #sudo apt-get install -y pkg-config cython swig libblas-dev liblapack-dev python-dev python-qt4 python-qt4-dev pyqt4-dev-tools libfreetype6-dev libpng12-dev libxml2-dev libxslt1-dev 2>&1 | tee -a ${EEWSDIR}/ElViS_install.log
# For Ubuntu 18.04:
  sudo apt-get install -y pkg-config cython swig libblas-dev python-httplib2 liblapack-dev python-dev python-qt4 python-qt4-dev pyqt4-dev-tools libfreetype6-dev libpng-dev libxml2-dev libxslt1-dev &>> ${EEWSDIR}/ElViS_install.log
  sudo apt-get install -y python-numpy python-scipy python-matplotlib ipython python-pandas python-sympy python-nose &>> ${EEWSDIR}/ElViS_install.log
  export LC_ALL=C
  sudo pip install -U stomp.py &>> ${EEWSDIR}/ElViS_install.log
  sudo pip install -U obspy &>> ${EEWSDIR}/ElViS_install.log
  make &>> ${EEWSDIR}/ElViS_install.log
  if [ $? -ne 0 ]; then
    echo "Error. Failed to install ElViS"
  else
    echo "ElViS is in the building"
  fi    
  cd ${EEWSDIR}
}

##### INIT SCRIPT (using screen)
create_init_script(){
echo "Creating init script"
cat << ENDCAT > ElarmsInit.bash
#!/bin/bash
export EW_INSTALL_HOME=${EEWSDIR}
export EW_INSTALL_VERSION=${EWDIR}
# running using detached screen
if [[ ! \`ps -ef | grep -v grep | grep \$PWD | grep startstop\` ]]; then
  echo Starting EarthWorm
  screen -dmS Earthworm bash -i -c ". \${EW_INSTALL_HOME}/run/params/ew_linux.bash;startstop"
else
  echo "Earthworm already running"
fi
if [[ ! \`ps -ef | grep -v grep | grep \$PWD | grep activemq.jar\` ]]; then
  echo Starting ActiveMQ
  screen -dmS ActiveMQ bash -i -c "java -jar \${EW_INSTALL_HOME}/activemq/bin/activemq.jar start"
else
  echo "ActiveMQ already running"
fi  
if [[ ! \`ps -ef | grep -v grep | grep \$PWD | grep DM\` ]]; then
  echo Starting DM
  screen -dmS DeciMod bash -i -c "cd \${EW_INSTALL_HOME}/run/bin;./dm dm.conf | ./termlog.awk -v PROG=\"DM\""
else
  echo "DM already running"
fi   
if [[ ! \`ps -ef | grep -v grep | grep \$PWD | grep "E2 E2.conf"\` ]]; then
  echo Starting E2
  screen -dmS ElarmS2 bash -i -c "cd \${EW_INSTALL_HOME}/run/bin;./E2 E2.conf | ./termlog.awk -v PROG=\"E2\""
else
  echo "E2 already running"
fi    
if [[ ! \`ps -ef | grep -v grep | grep \$PWD | grep "EWP2 EWP2.conf"\` ]]; then
  echo Starting EWP2
  screen -dmS ElarmSWP2 bash -i -c ". \${EW_INSTALL_HOME}/run/params/ew_linux.bash; cd \${EW_INSTALL_HOME}/run/bin;./EWP2 EWP2.conf | ./termlog.awk -v PROG=\"EWP2\""
else
  echo "EWP2 already running"
fi 
exit 0
# openning gnome-terminal if DISPLAY is available
if [ -e \$DISPLAY ]; then
  echo no display is available
  exit 0
else
  function find_screen {
    if screen -S "\$1" -X select . &> /dev/null; then
      return 0
    else
      return 1
    fi
  }
  find_screen Earthworm
  if [ \$? -eq 0 ]; then
    gnome-terminal --window -t Earthworm -e "bash -i -c \"screen -dr Earthworm\"" &
  else
    echo cannot locate Earthworm screen.
  fi
  find_screen ActiveMQ
  if [ \$? -eq 0 ]; then
    gnome-terminal --window -t ActiveMQ -e "bash -i -c \"screen -dr ActiveMQ\"" &
  else
    echo cannot locate ActiveMQ screen.
  fi
  find_screen DeciMod
  if [ \$? -eq 0 ]; then
    gnome-terminal --window -t DeciMod -e "bash -i -c \"screen -dr DeciMod\"" &
  else
    echo cannot locate DeciMod screen.
  fi
  find_screen ElarmS2
  if [ \$? -eq 0 ]; then
    gnome-terminal --window -t ElarmS2 -e "bash -i -c \"screen -dr ElarmS2\"" &
  else
    echo cannot locate ElarmS2 screen.
  fi
  find_screen ElarmSWP2
  if [ \$? -eq 0 ]; then
    gnome-terminal --window -t ElarmSWP2 -e "bash -i -c \"screen -dr ElarmSWP2\"" &
  else
    echo cannot locate ElarmSWP2 screen.
  fi
fi 
ENDCAT
chmod +x ElarmsInit.bash
sed -e 's/E2/E2r/g;s/EWP2/EWP2r/g' ElarmsInit.bash > ElarmsInitReplay.bash
chmod +x ElarmsInitReplay.bash
}

test_internet
# Check for parameters
case "$1" in
  earthworm)
          install_earthworm
          ;;
  activemq)
          install_activemq
          ;;
  epic)
          install_epic
          ;;
  elvis)
          install_elvis
          ;;
  all)
          install_dependencies
          install_java
          make_dirs
          install_earthworm
          install_activemq
          install_activemq_cpp
          install_epic
          install_elvis
          create_init_script
          ;;
  *)
          echo "Usage: $0 {all|earthworm|activemq|epic|elvis}"
          RETVAL=1
          ;;
esac
exit $RETVAL

cd ${EEWSDIR}
./fdsn2chnlist.py -i ".*\..*\..*\.[BHCSE][LHNC][ENZ012]$" -u "http://199.71.138.12:8181"
sort -k1,1 -k 2,2 -k3,3 -k4,4 -k11,11r -u channels.cfg | sort -k1,1 -k 2,2 -k3,3 -k4,4 -u > run/bin/channels.cfg
rm channels.cfg
awk '{print $1,$2,$5,$6}' run/bin/channels.cfg | sort -u > ElViS/stations.cfg
cp run/params/slink2ew.d.tmp run/params/slink2ew.d
awk '{print "Stream "$1"_"$2}' run/bin/channels.cfg | sort -u | grep -v "#" >> run/params/slink2ew.d
sed -i 's/\(Stream IS_MM[ABC].$\)/#\1/' run/params/slink2ew.d
echo "Need to update run/params/slink2ew.d host IP"
# autostart vbox:
# https://blogging.dragon.org.uk/start-stop-virtualbox-with-systemd/
# https://ubuntuforums.org/showthread.php?t=2387221
# add auto restart to crontab:
{ crontab -l ; echo "* * * * * cd ~/ElarmS && ./ElarmsInit.bash > ~/ElarmS/.cron.log"; } | crontab -


