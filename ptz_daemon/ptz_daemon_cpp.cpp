#define __cpp
#include "ak_common.h"
#include "ak_extern.h"
#include "ak_ini.h"
#include "MQTTClient.h"
#include "mqtt.h"
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define FIFOPATH "/tmp/ptz.daemon"
#define FILE_PATH "/etc/jffs2/ptz.ini"

#define BUF_SIZE (100)
#define CONFIG_VALUE_BUF_SIZE		200
#define CONFIG_STR_BUF_SIZE		20
#define CONFIG_MQTT_FILE_NAME  	"/etc/jffs2/mqtt.ini"

std::vector<std::string> operations = std::vector<std::string>();
int shuttingDown = 0;
int commandThreadDone = 0;
int xpos = 0;
int ypos = 0;
#define relativemove 15

struct mqtt_broker {
	int enabled;
	char user[20];
	char password[20];
	char ip[40];
	char port[6];
	char topic[20];
	char device_name[20];
	char manufacturer[20];
};

//char *mqtt_cmd = (char*)malloc(sizeof(char)*20);

struct sys_config {
	void *handle;

	struct mqtt_broker *mqtt_b;
};
static struct sys_config config = {NULL};



int mqtt_config_read() {
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	config.handle = ak_ini_init(CONFIG_MQTT_FILE_NAME);
	if(NULL == config.handle) {
		ak_print_normal_ex("mqtt config not found\n");
		return -1;
	} else {
		ak_print_normal_ex("mqtt config ok.\n\n");
	}
	config.mqtt_b = (struct mqtt_broker *)calloc(1,sizeof(struct mqtt_broker));
	if(ak_ini_get_item_value(config.handle, "mqtt_b", "enable_mqtt", value) == 0){
		if(atoi(value)==0){
			config.mqtt_b->enabled=0;
			ak_print_normal_ex("mqtt disabled. skipping config\n\n");
			return 0;
		}
		config.mqtt_b->enabled=1;
	}
	if(ak_ini_get_item_value(config.handle, "mqtt_b", "user", value) == 0){
		strcpy(config.mqtt_b->user, value);
	}
	if(ak_ini_get_item_value(config.handle, "mqtt_b", "password", value) == 0){
		strcpy(config.mqtt_b->password, value);
	}
	if(ak_ini_get_item_value(config.handle, "mqtt_b", "address", value) == 0){
		strcpy(config.mqtt_b->ip, value);
	}
	if(ak_ini_get_item_value(config.handle, "mqtt_b", "port", value) == 0){
		strcpy(config.mqtt_b->port, value);
	}
	if(ak_ini_get_item_value(config.handle, "mqtt_b", "topic", value) == 0){
		strcpy(config.mqtt_b->topic, value);
	}
	if(ak_ini_get_item_value(config.handle, "mqtt_b", "device_name", value) == 0){
		strcpy(config.mqtt_b->device_name, value);
	}
	if(ak_ini_get_item_value(config.handle, "mqtt_b", "manufacturer", value) == 0){
		strcpy(config.mqtt_b->manufacturer, value);
	}
	if(config.mqtt_b->ip == NULL || config.mqtt_b->port == NULL || config.mqtt_b->topic == NULL){
		return -1;
	}
	return 0;
	//ak_print_normal_ex("mqtt settings received with data username:%s\npassword:%s\nip:%s\nport:%s\ntopic:%s\n",config.mqtt_b->user,config.mqtt_b->password,config.mqtt_b->ip,config.mqtt_b->port,config.mqtt_b->topic);
}

void mqtt_config_release_ini()
{
	if (config.mqtt_b) {
		free(config.mqtt_b);
		config.mqtt_b = NULL;
	}
	ak_ini_destroy(config.handle);
}

void storePtzToFile() {
    FILE *file = fopen(FILE_PATH, "w");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%d %d\n", xpos, ypos);

    fclose(file);
    printf("Integers stored successfully in %s\n", FILE_PATH);
    printf("Stored values: %d %d\n", xpos, ypos);
}

void getStoredPtz() {
    FILE *file = fopen(FILE_PATH, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    if (fscanf(file, "%d %d", &xpos, &ypos) == 2) {
        printf("Stored values: %d %d\n", xpos, ypos);
    } else {
        printf("No stored values found. Run the program with integers as arguments to store values.\n");
    }

    fclose(file);
}

void initPTZ() {
	ak_drv_ptz_get_version();
	ak_drv_ptz_open();
	ak_drv_ptz_check_self(0);
	//ak_drv_ptz_set_degree(0x168, 0xc0);
	//ak_drv_ptz_set_angle_rate(0x40000000, 0x40000000);
	//ak_drv_ptz_get_status(0,0);
	getStoredPtz();
	if(xpos>0){
		ak_drv_ptz_turn(PTZ_TURN_RIGHT, xpos);
	}
	else if(xpos<0){
		ak_drv_ptz_turn(PTZ_TURN_LEFT, -xpos);
	}
	ak_drv_ptz_turn(PTZ_TURN_UP, -ypos);
}

/* stolen from
 * https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
 */
std::vector<std::string> splitString(const std::string &str, char delim) {
  std::vector<std::string> tokens;
  if (str == "")
    return tokens;
  std::string currentToken;
  std::stringstream ss(str);
  while (std::getline(ss, currentToken, delim)) {
    tokens.push_back(currentToken);
  }
  return tokens;
}

void process_command(const std::string l) {
  std::cout << "spawned processing command thread."
            << "\n";

  std::vector<std::string> cmd = splitString(l, ' ');
  std::cout << "parameters: \n";
  for (auto i : cmd) {
    std::cout << i << "\n";
  }

  if (cmd[0] == "init") {
    std::cout << "init ptz driver (anyka)"
              << "\n";
    initPTZ();
  } else if (cmd[0] == "setar") {
    std::cout << "set angle rate"
              << "\n";
    //ak_drv_ptz_set_angle_rate(atoll(cmd[1].c_str()), atoll(cmd[2].c_str()));
  } else if (cmd[0] == "setdeg") {
    std::cout << "set degree"
              << "\n";
    //ak_drv_ptz_set_degree(atoll(cmd[1].c_str()), atoll(cmd[2].c_str()));

  } else if (cmd[0] == "t2p") {
    std::cout << "turn to position"
              << "\n";
    xpos = atoll(cmd[1].c_str());
    ypos = atoll(cmd[2].c_str());
    ak_drv_ptz_turn_to_pos(xpos, ypos);
    storePtzToFile();
  } else if (cmd[0] == "down") {
    std::cout << "turn down"
              << "\n";
    if (ypos <= -relativemove){
      ypos += relativemove;
    }
    ak_drv_ptz_turn(PTZ_TURN_DOWN,relativemove);
    storePtzToFile();
  } else if (cmd[0] == "up") {
    std::cout << "turn up"
              << "\n";
    if (ypos >= relativemove-90){
      ypos -= relativemove;
    }
    ak_drv_ptz_turn(PTZ_TURN_UP,relativemove);
    storePtzToFile();
  } else if (cmd[0] == "left") {
    std::cout << "turn left"
              << "\n";
    if (xpos >= relativemove-170){
      xpos -= relativemove;
    }
    ak_drv_ptz_turn(PTZ_TURN_LEFT,relativemove);
    storePtzToFile();
  } else if (cmd[0] == "right") {
    std::cout << "turn right"
              << "\n";
    if (xpos <= 170-relativemove){
      xpos += relativemove;
    }
    ak_drv_ptz_turn(PTZ_TURN_RIGHT,relativemove);
    storePtzToFile();
  } else if (cmd[0] == "up_full") {
	std::cout << "turn up full"
			  << "\n";
	ak_drv_ptz_turn(PTZ_TURN_UP,90);
  } else if (cmd[0] == "down_full") {
	std::cout << "turn down full"
	          << "\n";
	ak_drv_ptz_turn(PTZ_TURN_DOWN,90);
  } else if (cmd[0] == "left_full") {
	std::cout << "turn left full"
	          << "\n";
	ak_drv_ptz_turn(PTZ_TURN_LEFT,360);
  } else if (cmd[0] == "right_full") {
	std::cout << "turn right full"
	          << "\n";
	ak_drv_ptz_turn(PTZ_TURN_RIGHT,360);
  } else if (cmd[0] == "stop_h") {
	std::cout << "stop horizontal"
	          << "\n";
	ak_drv_ptz_turn_stop(PTZ_TURN_RIGHT);
  } else if (cmd[0] == "stop_v") {
	std::cout << "stop vertical"
	          << "\n";
	ak_drv_ptz_turn_stop(PTZ_TURN_UP);
  } else if (cmd[0] == "stop") {
	std::cout << "stop all"
	          << "\n";
	ak_drv_ptz_turn_stop(PTZ_TURN_UP);
	ak_drv_ptz_turn_stop(PTZ_TURN_RIGHT);
  } else if (cmd[0] == "irgettres") {
    std::cout << "ir get treshold"
              << "\n";
    int a = ak_drv_ir_get_threshold(0, 0);
    std::cout << "ir get treshold: " << a << "\n";
  } else if (cmd[0] == "irinit") {
    std::cout << "ir init"
              << "\n";
    ak_drv_ir_init();
  } else if (cmd[0] == "irgetinputlevel") {
    std::cout << "get ir input level"
              << "\n";
    int a = ak_drv_ir_get_input_level(0, 0, 0, 0);

    std::cout << "get ir input level: " << a << "\n";
  } else if (cmd[0] == "irsetcheckmode") {
    std::cout << "ir set checkmode"
              << "\n";
    ak_drv_ir_set_check_mode(atoi(cmd[1].c_str()));
  } else if (cmd[0] == "irsetircut") {
    std::cout << "ir set ircut " << cmd[1] << "\n";
    ak_drv_ir_set_ircut(atoi(cmd[1].c_str()));
  } else if (cmd[0] == "irsettres") {
    std::cout << "ir set treshold " << cmd[1] << "\n";
    ak_drv_ir_set_threshold((void *)cmd[1].c_str(), (void *)cmd[2].c_str());
  } else if (cmd[0] == "t") {
    std::cout << "turn"
              << "\n";
    ptz_turn_direction tcmd = static_cast<ptz_turn_direction>(atoi(cmd[1].c_str()));
    ak_drv_ptz_turn(tcmd, atoi(cmd[2].c_str()));
    storePtzToFile();
  } else if (cmd[0] == "discovery") {
	  std::cout << "home assistant discovery"
	            << "\n";
	  if(config.mqtt_b->enabled==1){
		  ha_discovery(config.mqtt_b->topic, config.mqtt_b->manufacturer, config.mqtt_b->device_name);
	  }
  } else if (cmd[0] == "q") {
    shuttingDown = 1;
  } else if (cmd[0] == "ping") {
    std::cout << "pong"
              << "\n";
  }
  commandThreadDone = 1;
}

void thread_ready() {
  std::cout << "This thread is ready to be torn down!\n";
  commandThreadDone = 1;
}

void handle_fifo() {

  std::ifstream fifo;

  while (shuttingDown == 0) {
    fifo.open(FIFOPATH, std::ifstream::in);
    std::string l;
    while (std::getline(fifo, l)) {
      if (l.length() > 0)
        std::cout << "received something in the FIFO:" << l << "\n";
      else
        continue;

      std::cout << "Added operation to buffer\n";
      operations.push_back(l);
    }
    fifo.close();
    sleep(1);
  }
}

int process_mqtt_cmd(char* recv_msg){
	//Process MQTT commands.
	if(strcmp(recv_msg,"init")== 0){
		operations.push_back(recv_msg);
	}
	else if (strcmp(recv_msg,"discovery") == 0){
		operations.push_back(recv_msg);
	}
	else if (strcmp(recv_msg,"left")== 0){
		operations.push_back(recv_msg);
	}
	else if (strcmp(recv_msg,"right")== 0){
		operations.push_back(recv_msg);
	}
	else if (strcmp(recv_msg,"up")== 0){
		operations.push_back(recv_msg);
	}
	else if (strcmp(recv_msg,"down")== 0){
		operations.push_back(recv_msg);
	}
	else if (strcmp(recv_msg,"left_full")== 0){
		operations.push_back(recv_msg);
	}
	else if (strcmp(recv_msg,"right_full")== 0){
		operations.push_back(recv_msg);
	}
	else if (strcmp(recv_msg,"up_full")== 0){
		operations.push_back(recv_msg);
	}
	else if (strcmp(recv_msg,"down_full")== 0){
		operations.push_back(recv_msg);
	}
	else if (strcmp(recv_msg,"stop_h")== 0){
		operations.push_back(recv_msg);
	}
	else if (strcmp(recv_msg,"stop_v")== 0){
		operations.push_back(recv_msg);
	}
	else if (strcmp(recv_msg,"stop")== 0){
		operations.push_back(recv_msg);
	}
	else if (strcmp(recv_msg,"exit")== 0){
			shuttingDown=1;
		}
	else {
		ak_print_normal_ex("Unknown MQTT command received\n");
		return -1;
	}

	return 0;
}

int main(int argc, char **argv) {

  FILE *file = fopen(FILE_PATH, "a");
    
  if (file != NULL) {
        // File opened successfully, do something with the file

        // Close the file when done
    fclose(file);
  }

  struct stat buf;
  if (stat(FIFOPATH, &buf) != 0) {
    std::cout << "creating FIFO in /tmp..."
              << "\n";

    if ((mkfifo(FIFOPATH, 0700)) != 0) {
      std::cerr << "could not create fifo @ " << FIFOPATH << "\n";
      return 1;
    }
  }
    //std::cout << "init ptz driver (anyka)\n";
    //ak_drv_ptz_open();
    //ak_drv_ptz_check_self(0);
    //Broken function? Needs more debugging
    //ak_drv_ptz_setup_step_param(0,2048,1024,-1);
    //ak_drv_ptz_setup_step_param(1,2048,1024,-1);
    //Obsolete functions
    //ak_drv_ptz_set_degree(0x168, 0xc0);
    //ak_drv_ptz_set_angle_rate(0x40000000, 0x40000000);
    //getStoredPtz();
    //ak_drv_ptz_turn_to_pos(xpos, ypos);
  std::cout << "running as daemon now. awaiting commands in FIFO\n";
  if(mqtt_config_read() == -1){
	  ak_print_error_ex("Error retrieving mqtt data from ini. Please check the file mqtt.ini in jffs2");
	  return -1;
  }
  //Enable MQTT if the flag is set to 1 in the ini
  if(config.mqtt_b->enabled==1){
	  init_mqtt(config.mqtt_b->ip, config.mqtt_b->port, config.mqtt_b->user, config.mqtt_b->password, config.mqtt_b->topic, &process_mqtt_cmd);
  }
  else {
	  ak_print_normal_ex("MQTT client disabled. Skipping.\n");
  }
  initPTZ();
  //_TEST_ak_read_values();

  
  shuttingDown = 0;
  std::thread t = std::thread(thread_ready);
  std::thread handleFIFO = std::thread(handle_fifo);
  while (shuttingDown == 0) {
    if (operations.size() > 0 && (!t.joinable() || commandThreadDone == 1)) {
      commandThreadDone = 0;
      std::cout << "thread is done - joining thread...\n";
      if (t.joinable())
        t.join();
      if (operations.size() > 0) {
        std::string line = operations.front();
        operations.erase(operations.begin());
        t = std::thread(process_command, line.c_str());
      }
    } else if (operations.size() > 0) {
      std::cout << "previous job is still running...\n";
    }
 
    sleep(1);
  }
  std::cout << "Received shutdown command. Exiting\n";
  mqtt_config_release_ini();
  _destroy_mqtt();
  ak_drv_ptz_close();
  return 0;
}
