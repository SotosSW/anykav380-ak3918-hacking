#! /bin/sh

restart_process()
{
  echo '(re)starting ptz...'
  export LD_LIBRARY_PATH=/mnt/ptz/lib
  /mnt/ptz/ptz
}

check_process_health()
{
  myresult=$( top -n 1 | grep snapshot | grep -v grep | grep -v daemon )
  echo $myresult
  if [[ ${#myresult} -lt 5 ]]; then
    restart_process
  fi
}

#load kernel modules for camera
#insmod /usr/modules/sensor_h63.ko
#insmod /usr/modules/akcamera.ko
#insmod /usr/modules/ak_info_dump.ko

#while [ 1 ]; do
  #check_process_health
  restart_process
  sleep 5
#done
