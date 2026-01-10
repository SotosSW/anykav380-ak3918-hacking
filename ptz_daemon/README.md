# This is a modified version of
https://github.com/kuhnchris/IOT-ANYKA-PTZdaemon
and
https://github.com/MuhammedKalkan/Anyka-Camera-Firmware/

Thank you to both [_@kuhnchris_](https://github.com/kuhnchris/IOT-ANYKA-PTZdaemon/) and [_@MuhammedKalkan_](https://github.com/MuhammedKalkan/Anyka-Camera-Firmware/) for their work.

DISCLAIMER: I'm a trained C# developer but by no means am I an experienced C/C++ developer. In fact I **hate** C and its primordial memory management system. My modifications are by no means optimal and not a lot of bug testing/logical error traps have been done. Use at your own discretion and feel free to open issue tickets and pull requests with ideas/additions/optimizations.

DISCLAIMER 2: The compiled version of this app may or may not work depending on your camera configuration/onboard devices. If you need help with certain errors related to this app open an issue ticket. This app is VERY dependent on the libplat_drv library also found in my repository. 

## PTZ tracker/daemon

This is tailored to the functions in the module AK_MOTOR, wrapped by plat_drv, for ANYKA "CPU"s.

The 'ptz' app (that's the main thing) provides a fifo/pipe in /tmp to control the camera's motors.

A MQTT client has been implemented in addition to the fifo/pipe control system by implementing the [_paho.mqtt.c_](https://github.com/eclipse-paho/paho.mqtt.c) library.

A rough Home Assistant integration is implemented with the ability to publish a discovery packet and currently only 1 functioning sensor which is the camera's availability. More might be added but I currently implement my camera in HA using WebRTC addon which provides integration with the RTSP protocol and ptz support using my MQTT implementation.

Included is also the PTZ position storing functionallity provided by MuhammedKalkan.

How to run?

Create a folder named `ptz` in your SD card. Copy the folder `lib` and files `ptz` and `ptz.sh` from the folder ptz_daemon to the ptz folder in your SD card.
Afterwards copy the file `mqtt.ini` from the folder ini to the folder /etc/jffs2/ in your camera either through FTP or by copying the file from your SD card to the camera using a terminal.
Note that the application won't start without a properly configured mqtt.ini

Open a serial/telnet/ssh connection to your camera and run.

```
 cd mnt/ptz
 ./ptz/ptz.sh 
```

If you get any permission errors run the following. (Replace `root` with your user if you don't use root)

```
 chown root ptz.sh
 chown root ptz
 chmod 755 ptz
 chmod 755 ptz.sh
```

The camera should imediatelly start rotating

Afterwards you can either set the speed of the camera or use the commands to oscilate the camera around. Please note that some functions are temporary broken and I haven't gotten around to fixing them (yet?). Note that the "_fully" commands override the PTZ position storing system as the camera doesn't provid current position feedback (at least mine doesn't).

##Issuing commands using FIFO/pipe

use `echo "CMD" >> /tmp/ptz.daemon` eg. echo "left" >> /tmp/ptz.daemon

```
- if (cmd[0] == "init")     --> Intializes the module
- if (cmd[0] == "left")     --> Move the camera left by x amount of degrees as stated in the relativemove variable
- if (cmd[0] == "right")    --> Move the camera left -//-
- if (cmd[0] == "up")       --> Move the camera up -//- 
- if (cmd[0] == "down")     --> Move the camera down -//- 
- if (cmd[0] == "left_full")--> Move the camera fully to the left
- if (cmd[0] == "right_full")--> Move the camera fully to the right
- if (cmd[0] == "up_full")  --> Move the camera fully up
- if (cmd[0] == "down_full")--> Move the camera fully down
- if (cmd[0] == "stop_h")     --> stops the camera horizontal rotation
- if (cmd[0] == "stop_v")     --> stops the camera vertical rotation
- if (cmd[0] == "stop")     --> stops the camera rotation
- if (cmd[0] == "q")        --> quit, exits daemon
- if (cmd[0] == "ping")     --> returns "pong"

Untested:
- if (cmd[0] == "setar")    --> set angle rate
- if (cmd[0] == "setdeg")   --> set degrees (?)
- if (cmd[0] == "t2p")      --> turn to position - main function to control cam
- if (cmd[0] == "setspeed") --> sets the speed at which the camera turns
- if (cmd[0] == "t")        --> turn 
```
##Using and issuing commands using MQTT
Before starting the application, make sure to check the mqtt.ini settings for your desired input. No WebSocket/SSL support is added yet but should be pretty straightforward to do it using the examples provided by the paho.mqtt.c library.

By default, the camera listens for commands on the "*topic* + /ptz_cmd" topic. Using the ini default values that is `anyka_camera/ptz_cmd`

The currently available commands are:

```
- init      --> Intializes the module
- discovery --> Send a discovery packet to HA's default discovery topic.
- left      --> Move the camera left by x amount of degrees as stated in the relativemove variable
- right     --> Move the camera left -//-
- up        --> Move the camera up -//- 
- down      --> Move the camera down -//- 
- left_full --> Move the camera fully to the left
- right_full--> Move the camera fully to the right
- up_full   --> Move the camera fully up
- down_full --> Move the camera fully down
- stop_h    --> stops the camera horizontal rotation
- stop_v    --> stops the camera vertical rotation
- stop      --> stops the camera rotation
- exit      --> quit, exits daemon

```

You can always go around and check the libplat_* files to check for more functions you may want to use.
This libplat_drv.so provided by the `LSC Smart Rotatable Camera` provides the following objects:

```
> arm-arm926ejs-linux-uclibcgnueabi-nm libs/libplat_drv.so | grep ak
         U ak_check_file_exist
         U ak_cmd_exec
00003af0 T ak_drv_i2c_close
00003804 T ak_drv_i2c_open
0000388c T ak_drv_i2c_read
00003984 T ak_drv_i2c_write
0000199c T ak_drv_ir_get_input_level
00001b5c T ak_drv_ir_get_threshold
00001990 T ak_drv_ir_get_version
00001d18 T ak_drv_ir_init
00001ebc T ak_drv_irled_get_version
00001f30 T ak_drv_irled_get_working_stat
00001ec8 T ak_drv_irled_init
00001fd8 T ak_drv_irled_set_working_stat
00001b48 T ak_drv_ir_set_check_mode
00001c1c T ak_drv_ir_set_ircut
00001bc4 T ak_drv_ir_set_threshold
00003660 T ak_drv_key_close
0000347c T ak_drv_key_get_event
00003414 T ak_drv_key_get_version
00003420 T ak_drv_key_open
00002770 T ak_drv_ptz_check_self
00002ad8 T ak_drv_ptz_check_self_ex
00003354 T ak_drv_ptz_close
00003224 T ak_drv_ptz_get_status
00002710 T ak_drv_ptz_get_step_pos
00002588 T ak_drv_ptz_get_version
00002594 T ak_drv_ptz_open
00002ee8 T ak_drv_ptz_reset_dg
00002ec0 T ak_drv_ptz_set_angle_rate
00002ed4 T ak_drv_ptz_set_degree
000032c0 T ak_drv_ptz_set_speed
0000269c T ak_drv_ptz_setup_step_param
00002f6c T ak_drv_ptz_turn
0000319c T ak_drv_ptz_turn_stop
00003064 T ak_drv_ptz_turn_to_pos
000037c0 T ak_drv_pwm_close
000036c0 T ak_drv_pwm_open
00003740 T ak_drv_pwm_set
00003c68 T ak_drv_wdt_close
00003c0c T ak_drv_wdt_feed
00003b10 T ak_drv_wdt_get_version
00003b1c T ak_drv_wdt_open
0000d1a8 d akmotor
```
You can easily adapt this project and do whatever you want with it, all of this is CC-BY-SA.
All the libs are copyright by their respective owners tho - so in this case I assume ANYKA? Or TUYA? Not entirely sure as there are various mixed licences in there.

Also, if you try to do it with a different ANYKA-based camera, please make sure to switch out the libs/ from the actual system (libdl, libm, libplat*, libpthread, libstdc++, libuclibc)

To *compile* this application you need a viable compiler. I used arm-anykav200-crosstool found in the qiwen [_Anycloud SDK_](https://github.com/helloworld-spec/qiwen/tree/main/anycloud39ev300).
I didn't have good success using generic arm compilers and other corsstools so I suggest you start from this one and save yourself some time.

Unfortunately due to the MQTT library, you'll have to install and compile OpenSSL too. This one was an **absolute pain in the arse** to compile for the camera properly. Lots of compiler errors and missing symbol references until I got it working so I'll pop the straightforward instructions bellow.

##Compiling/Installing OpenSSL

NOTE: I only managed to build successful, functioning libraries using the arm-anykav200-crosstool. If you are a more experienced user feel free to try another crosstool.

Start by cloning the OpenSSL repository to a folder of your choice and configure the build options for `armv4`

```
 git clone https://github.com/helloworld-spec/qiwen/tree/main/anycloud39ev300
 ./Configure linux-armv4 no-async --cross-compile-prefix=/usr/bin/ --prefix=/opt/openssl
```

Then open the Makefile and search for: _User defined commands and flags_
Asuming you are using the anyka cross tool installed in /opt/, replace the following settings from CROSS_COMPILE to RCFLAGS

```
CROSS_COMPILE=/opt/arm-anykav200-crosstool/usr/bin/
CC=$(CROSS_COMPILE)arm-anykav200-linux-uclibcgnueabi-gcc
CXX=$(CROSS_COMPILE)arm-anykav200-linux-uclibcgnueabi-g++
CPPFLAGS=
CFLAGS=-Wall -O3 -std=gnu99
CXXFLAGS=-Wall -O3
LDFLAGS=
EX_LIBS= 
OBJCOPY=objcopy

MAKEDEPEND=

PERL=/usr/bin/perl

AR=$(CROSS_COMPILE)arm-anykav200-linux-uclibcgnueabi-ar
ARFLAGS= qc
RANLIB=$(CROSS_COMPILE)arm-anykav200-linux-uclibcgnueabi-ranlib
RC= $(CROSS_COMPILE)windres
RCFLAGS= 
```

Then open a terminal in the openssl git folder and do the following:
```
make
sudo make install
```
Please note that the OpenSSL library is **enormous** and compilation times can take minutes (10 minutes in my resource-limited VM in my case). After this step you should be ready to compile your project.
Note: The most important parts for a successful build are configuring the build with the no-async option and adding the -std=gnu99 flag in the C compiler flags.



---

## Compile/Build: 
I use Eclipse IDE for working on my code and compiling so the Makefile provided is autogenerated by the IDE given my parameters. You may find it in the ptz_daemon folder.

You should only need to change the compiler/linker, library and include paths in `makefile` and `subdir.mk` 

I may later make a more standard makefile file but for the time being I apologize as my time is currently limited and it's not in my priorities.

---

## PTZ?

**P**an **T**ilt **Z**oom - basically the camera controls.
