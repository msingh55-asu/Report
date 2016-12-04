#include "../../include/gpio.h"
#include "../../include/device.h"
#include "../../include/misc/printk.h"
#include "../../include/zephyr.h"
#include "../../include/gpio.h"
#include "../../include/sys_clock.h"
#include "../../include/misc/util.h"
#include "../../include/legacy.h"
#include <string.h>
#include <kernel.h>
/*Macros *************************************************************************/
#define HCSR_DEVICE_NAME "HCSR"
#define HCSR_DRIVER_PRIORITY 32
#define SLEEPTIME 1 				/*Time specified for 1 microsecond*/ 
#define SLEEPTICKS (SLEEPTIME * sys_clock_ticks_per_sec / 1000000)

#define GPIO_OUT_PIN		16
#define GPIO_INT_PIN		17
#define GPIO_OUT_LED_PIN	18
#define GPIO_NAME		"GPIO_"

#if defined(CONFIG_GPIO_DW_0)
#define GPIO_DRV_NAME CONFIG_GPIO_DW_0_NAME
#elif defined(CONFIG_GPIO_QMSI_0)
#define GPIO_DRV_NAME CONFIG_GPIO_QMSI_0_NAME
#elif defined(CONFIG_GPIO_ATMEL_SAM3)
#define GPIO_DRV_NAME CONFIG_GPIO_ATMEL_SAM3_PORTC_DEV_NAME
#else
#define GPIO_DRV_NAME "GPIO_0"
#endif


/*Calculate time using ticks*/
#define HW_CYCLES_TO_USEC(__hw_cycle__) \
        ( \
                ((uint64_t)(__hw_cycle__) * (uint64_t)sys_clock_us_per_tick) / \
                ((uint64_t)sys_clock_hw_cycles_per_tick) \
        )

/**********************************************************************************/

static inline uint32_t time_delta(uint32_t ts, uint32_t t)
{
        return (t >= ts) ? (t - ts) : (ULONG_MAX - ts + t);
}


/*
struct device {
      struct device_config *config;
      void *driver_api;
      void *driver_data;
};
*/
struct hcsr_data {
        int distance; /*Distance masureed by the device*/
        int64_t time_stamp;/*Time stamp*/
};

typedef int (*hcsr_api_ret)(struct device *device); /*Data type for API return*/
//typedef struct hcsr_data (*hcsr_api_ret_data)(struct device *device);

struct hcsr_api {
    hcsr_api_ret reset; /*Reset Data*/
    hcsr_api_ret read; /*Read from Data in device*/
    hcsr_api_ret write; /*Write device buffersusing readings*/
};


struct hcsr_config {
	uint32_t trigger_pin; /*Device specific trigger pin*/
	uint32_t echo_pin; 	/*Deice specific echopins*/
	device_sync_call_t hcsr_sem; /*Device Synchronization*/
};



int hcsr_reset(struct device *);
int hcsr_read(struct device *, struct hcsr_data *);
int hcsr_write(struct device *);
int hcsr_init(struct device *);

int hcsr_reset(struct device *device)
{
        printk("Calling Call HCSR Reset\n");
	//int64_t k_utime_get();
        
	struct hcsr_data *data = device->driver_data; /*getting data structure pointer*/

	data->distance = 0; /*Resetting values*/
        data->time_stamp = 0;
	return 0;
}

int hcsr_read(struct device *device, struct hcsr_data *data)
{
	//int64_t k_utime_get();
        printk("Calling Call HCSR Read\n");
	data = device->driver_data; /*getting data sructure pointer*/
	if(data->distance == 0)
	{
		hcsr_write(device); /*writing to struture pointer*/
		//push to EEPROM
	}
	return 0;
}


int hcsr_write(struct device *device)
{
    	int val, ret, distance_measured;
	struct device *gpio_dev;
	int64_t duration, start, end, distance;
	struct hcsr_config *dev_config = (struct hcsr_config *)device->config->config_info;
	device_sync_call_init(&dev_config->hcsr_sem); /*Init Semaphore*/
	struct hcsr_data *data = device->driver_data;

	printk("Calling Call HCSR Write\n");
    	
	
	gpio_dev = device_get_binding(GPIO_DRV_NAME); /*getting gpio driver name*/
	if (!gpio_dev) {
		printk("Cannot find %s!\n", GPIO_DRV_NAME);
	}

	/* Setup GPIO output */
	ret = gpio_pin_configure(gpio_dev, GPIO_OUT_PIN, (GPIO_DIR_OUT));
	if (ret) {
		printk("Error configuring " GPIO_NAME "%d!\n", GPIO_OUT_PIN);
	}

	

	/* Setup GPIO input, and triggers on rising edge. */
	ret = gpio_pin_configure(gpio_dev, GPIO_INT_PIN,
			(GPIO_DIR_IN | GPIO_INT_EDGE
			 | GPIO_INT_ACTIVE_HIGH | GPIO_INT_DEBOUNCE));
	if (ret) {
		printk("Error configuring " GPIO_NAME "%d!\n", GPIO_INT_PIN);
	}

	/* Setup GPIO LED output */
	ret = gpio_pin_configure(gpio_dev, GPIO_OUT_LED_PIN, (GPIO_DIR_OUT));
	if (ret) {
		printk("Error configuring " GPIO_NAME "%d!\n", GPIO_OUT_PIN);
	}

	device_sync_call_wait(&dev_config->hcsr_sem); /*Wait semaphore*/

	ret = gpio_pin_write(gpio_dev, GPIO_OUT_PIN, 0);
	if (ret) 
	{
		printk("Error set " GPIO_NAME "%d!\n", GPIO_OUT_PIN);
	}
	//task_sleep(USEC(2));  //Depreciated

	ret = gpio_pin_write(gpio_dev, GPIO_OUT_PIN, 1);
	if (ret) {
		printk("Error set " GPIO_NAME "%d!\n", GPIO_OUT_PIN);
	}
	//task_sleep(USEC(10)); //Depreciated
	ret = gpio_pin_write(gpio_dev, GPIO_OUT_PIN, 0);
	if (ret) {
		printk("Error set " GPIO_NAME "%d!\n", GPIO_OUT_PIN);
	}
	
	/*Reading pulseIn*/
	printk("Reading \n");
	do {
		gpio_pin_read(gpio_dev, GPIO_INT_PIN, &val); 
	} while (val == 0);
	start = sys_cycle_get_32();
	do {
		gpio_pin_read(gpio_dev, GPIO_INT_PIN, &val); 
	} while (val == 1);
	end = sys_cycle_get_32();

	duration = HW_CYCLES_TO_USEC(time_delta(start, end)); //uint32_t duration = end - start;
	printk("duration %lld \n", duration); //duration = (uint32_t)duration / (uint32_t)sys_clock_hw_cycles_per_tick; 
        
	//Calculate the distance (in cm) based on the speed of sound.
	distance = duration / 58;
	printk("Distance %lld \n", distance);
 	distance_measured = distance;
	data->distance = distance_measured;
	data->time_stamp = k_uptime_get();

	device_sync_call_complete(&dev_config->hcsr_sem); /*Close semaphore*/
	
	return distance_measured;
}
