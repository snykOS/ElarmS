# ElarmS install
---
## run:<br>
`ElarmS.install.bash`<br>
## untar params files and update parameters:<br>
### in run/params:
earthworm stream input (e.g. slink2ew.d)
email in statmgr.conf
### in run/bin:
E2.conf, EWP2.conf, WP.conf
### in activemq/conf:
activemq.xml, camel.xml
## Troublshooting
If apt update is failing with Java:
```
E: Repository 'http://ppa.launchpad.net/webupd8team/java/ubuntu bionic InRelease' changed its 'Label' value from 'Oracle Java (JDK) 8 / 9 Installer PPA' to 'Oracle Java (JDK) 8 Installer PPA' 
N: This must be accepted explicitly before updates for this repository can be applied. See apt-secure(8) manpage for details.
```
solution from [askubuntu.com](https://askubuntu.com/questions/1085166/how-to-update-from-webupd8-team-ppa-oracle-jdk-8-9-that-got-inconsistent-with):
```
sudo apt-get --allow-releaseinfo-change update
sudo apt-get update
```

