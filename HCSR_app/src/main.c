/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <zephyr.h>
#include <misc/shell.h>
#include <misc/printk.h>
#include <device.h>
#include <stdlib.h>
#include "../../../drivers/hcsr/hcsr.h"

#define DEVICE_NAME "device test"
#define MY_SHELL_MODULE "myshell"
#define HELP_MOD "Help here"

static int hcsr_selected;
struct device *dev_hcsr_1, *dev_hcsr_2;

static int shell_cmd_enable(int argc, char *argv[])
{
	printk("Enable the device\n");
	if(argc == 0)
	{
		printk("Enter an argument 0, 1 or 2\n");
		printk("Enter 0 to enable No device\nEnter 1 to enable HCSR1\nEnter 2 to enable HCSR2\n");	
		return 1;
	}
	
	hcsr_selected = atoi(argv[1]); /*Saving value in global variable  for the device being used*/
	
	if(hcsr_selected == 1)
	{
		dev_hcsr_1 = device_get_binding("HCSR1");
        	printk("dev %p name %s\n", dev_hcsr_1, dev_hcsr_1->config->name);
	}
	
	if(hcsr_selected == 2)
	{
		dev_hcsr_2 = device_get_binding("HCSR2");
		printk("dev %p name %s\n", dev_hcsr_2, dev_hcsr_2->config->name);
	}
	
	return 0;
}

static int shell_cmd_start(int argc, char *argv[])
{
	printk("Start writing to EEPROM\n");
	if(argc == 0 || atoi(argv[1]) > 512)
        {
                printk("Enter an argument p<=512\n");
	}
	if(!hcsr_selected)
	{
		printk("No HCSR device selected\n");
	}
	int page;
	int p = atoi(argv[1]);
	
	if (hcsr_selected == 1)
	{
		hcsr_reset(dev_hcsr_1);
		while(page < p)
		{
			hcsr_write(dev_hcsr_1); 	//Read from Sensor HCSR1
			//write to EEPROM page from dev_hcsr_1->driver_data
		}
	}
	if (hcsr_selected == 2)
        {       
		hcsr_reset(dev_hcsr_2);
                while(page < p)
                {       
                        hcsr_write(dev_hcsr_2);	//Read from Sensor HCSR2
                        //write to EEPROM page from dev_hcsr_2->driver_data
                }
        }
	
	return 0;
}

static int shell_cmd_dump(int argc, char *argv[])
{
	
	printk("Dump readings from page1 to page 2\n");
	if(argc == 0 || atoi(argv[2]) < atoi(argv[1]))
        {
                printk("Enter an arguments p1 and p2 with p2 > p1\n");
        }
	int start_page, end_page;
	struct hcsr_data data1, data2;
	start_page = atoi(argv[2]);
	end_page = atoi(argv[1]);
	if (hcsr_selected == 1)
        {
                while(start_page <= end_page)
		{
			hcsr_read( dev_hcsr_1,&data1);
			start_page++;
		}
	}
	if (hcsr_selected == 2)
        {
                while(start_page <= end_page)
                {
                        hcsr_read(dev_hcsr_2, &data2);
                        start_page++;
        	}
	}
	return 0;
}

static const struct shell_cmd commands[] = {

	{ "Enable", shell_cmd_enable, HELP_MOD },
	{ "Start", shell_cmd_start , HELP_MOD},
	{ "Dump", shell_cmd_dump , HELP_MOD},
	{ NULL, NULL }
};

void main(void)
{
	printk("Hello World! %s\n", CONFIG_ARCH);
	SHELL_REGISTER(MY_SHELL_MODULE, commands);
	shell_init("console>");

}
