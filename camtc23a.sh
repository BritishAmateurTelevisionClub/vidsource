#!/bin/bash

#camtc23a.sh 

# displays up to 100 animated testcards or the Pi camera
# designed for 720x576 screens

# camtc23a.sh		main program shell
# tcdata0.txt		testcard definition file
# tcdata73.txt		testcard definition file
# tcprog			the program that displays the testcard
# testcard640.jpg	plain testcard image
# tcindex			stores the current tcdata number for power up
#					created if it doesn't exist

# requires wiringPi
# install into /boot/testcard
# ground header pin 16 through a resistor (1-5k) to display the Pi camera
# ground and release header pin 18 through a resistor to move to the next testcard
# pins 14 and 20 are grounds


trapit()
{
  sudo kill $pid
  echo ""
  exit
}

trap trapit SIGINT
trap trapit SIGTERM

CAM=4                    # Wiring Pi 4, Physical Pin 16 for camera
NEXT=5                   # Wiring Pi 5  Physical Pin 18 for next testcard
gpio mode $CAM up        # Set the pull-up on (active low)
gpio mode $NEXT up       # Set the pull-up on (active low)

sudo killall raspivid
sudo killall fbi
clear
index=0

indexfile="tcindex"
callfile="callsign.txt"
read index < $indexfile   # tcindex stores the last active card
read callsign < $callfile
while true; do            # Main loop

  sw1=$(gpio read $CAM)   # Camera loop
  if [ $sw1 = 0 ]; then
    raspivid -t 0 -fps 25 -w 720 -h 576 -ae 40,0xff,0x808000 -a "\n$callsign                      " & export pid=$!
    sleep 0.1s

    shutdown_count=0
    sw1=$(gpio read $CAM)
    while [ $sw1 = 0 ]; do       # While camera switch active
      sw1=$(gpio read $CAM)
      sw2=$(gpio read $NEXT)
      if [ $sw2 = 0 ]; then
        let shutdown_count=$shutdown_count+1
      else
        shutdown_count=0
      fi

      if [ $shutdown_count -gt 19 ]; then  # Shutdown on 2 second press of selector
        sudo kill $pid
        echo ShutDown
        sudo fbi -T 1 -noverbose -a /home/pi/vidsource/shutdown.jpg >/dev/null 2>/dev/null
        sleep 2
        sudo shutdown now
        exit
      fi
      sleep 0.1s
    done

    sudo kill $pid
    clear
  fi

  sw1=$(gpio read $CAM)
  if [ $sw1 = 1 ]; then                # Not camera, but testcards
    exists=0
    while [ $exists = 0 ]; do
      filename1="./tcdata"
      filename2=".txt"
      filename="$filename1$index$filename2"

      if [ -f $filename ]; then
        exists=1
        rm $indexfile
        echo $index > $indexfile
      else
        exists=0
        ((index++))
        if [ $index -ge 100 ]; then
          index=0
        fi
      fi
    done
  fi

  #echo $filename

    { read p1; read p2; read p3; read p4; read p5; read p6; } < $filename
    quote="\""
    space=" "
    position="$p3$space$p4"
    command="./"
    command2="& export pid=\$!"
    command="$command$p1$space$quote$p2$quote$space$position$space$quote$p5$quote$space$quote$p6$quote$space$command2"
    #echo $command
    eval $command

  #echo $pid

  sw1=$(gpio read $CAM)
  sw2=$(gpio read $NEXT)
  (( sw = sw1 * sw2 ))

  while [ $sw = 1 ]; do          # Wait here while button is inactive and camera deselected
    sleep 0.1s
    sw1=$(gpio read $CAM)
    sw2=$(gpio read $NEXT)
    (( sw = sw1 * sw2 ))
  done

  #echo $pid
  sudo kill $pid                # Finished tcanim, so kill the process
  clear

  sw1=$(gpio read $CAM)

  if [ $sw1 = 1 ]; then         # Not camera
    ((index++))
    sleep 0.1s
    sw2=$(gpio read $NEXT)
    shutdown_count=0

    while [ $sw2 = 0 ]; do
      sw2=$(gpio read $NEXT)
      if [ $sw2 = 0 ]; then
        let shutdown_count=$shutdown_count+1
      else
        shutdown_count=0
      fi

      if [ $shutdown_count -gt 19 ]; then  # Shutdown on 2 second press of selector
        echo ShutDown
        sudo fbi -T 1 -noverbose -a /home/pi/vidsource/shutdown.jpg >/dev/null 2>/dev/null
        sleep 2
        sudo shutdown now
        exit
      fi
      sleep 0.1s
    done
  fi
done


