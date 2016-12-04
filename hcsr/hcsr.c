#include "./hcsr.h"


struct hcsr_data hcsr_data_inst_1;
struct hcsr_data hcsr_data_inst_2;

int hcsr_init(struct device *device)  /*Initialisation function*/
{
	struct hcsr_config *config = device->config->config_info;	
	device_sync_call_init((device_sync_call_t *)&config->hcsr_sem);
    	return 0;
};

/*Per device Configuration Data Structure for HCSR1*/
static struct hcsr_config hcsr_config_val_1 = {
    	.trigger_pin    = CONFIG_HCSR_1_TRIGGER,
    	.echo_pin       = CONFIG_HCSR_1_ECHO,
};

/*Per device Configuration Data Structure for HCSR1*/
static struct hcsr_config hcsr_config_val_2 = {
        .trigger_pin    = CONFIG_HCSR_2_TRIGGER,
        .echo_pin       = CONFIG_HCSR_2_ECHO,
};

/*Per device API*/
static struct hcsr_api hcsr_api_funcs = {
      .reset    = hcsr_reset, 
      .read     = hcsr_read,
      .write    = hcsr_write,
};

/*Device Init function for for HCSR1*/
DEVICE_AND_API_INIT(hcsr_driver_1, CONFIG_HCSR_DRV_1_NAME, hcsr_init, &hcsr_data_inst_1, &hcsr_config_val_1, PRE_KERNEL_1, HCSR_DRIVER_PRIORITY, &hcsr_api_funcs);
/*Device Init function for for HCSR2*/
DEVICE_AND_API_INIT(hcsr_driver_2, CONFIG_HCSR_DRV_2_NAME, hcsr_init, &hcsr_data_inst_2, &hcsr_config_val_2, PRE_KERNEL_1, HCSR_DRIVER_PRIORITY, &hcsr_api_funcs);


