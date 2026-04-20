#include <linux/module.h>    // included for all kernel modules
#include <linux/kernel.h>    // included for KERN_INFO
#include <linux/init.h>      // included for __init and __exit macros
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("sneeker");
MODULE_DESCRIPTION("A Simple WiFi control shell module");

void mmc_rescan_sdio(void);
void wifi_power(int on);

/*
 * 7000s need a duration between power down and up for completing reset if
 * hardware is unstable, this value should be decreased gradully to verify
 * hardware. Eventually we should make this value zero.
 */
static int p_delay = 10;
module_param(p_delay,int,0);

/*
 * 500ms should be enough, but 3000ms could rescue 7000s that might have 1 sec
 * delay of initial defaultly
 */
static int s_delay = 30;
module_param(s_delay,int,0);

int __init hello_init(void)
{

    printk(KERN_INFO "Hello world!\n");

    wifi_power(0);

    /* necessary delay for completing reset */
    printk(KERN_INFO "Start reset delay : %d ms\n", p_delay*100);
    mdelay(p_delay*100);

    wifi_power(1);

    /* wait for signal stablizaton, or 7000s might have one second delay of initial defaultly */
    printk(KERN_INFO "Start signal stablization delay : %d ms\n", s_delay*100);
    mdelay(s_delay*100);

    /* inform host controller to scan SDIO device */
    mmc_rescan_sdio();

    /* delay 7000s access */
    mdelay(500);
    return 0;    // Non-zero return means that the module couldn't be loaded.
}

void __exit hello_cleanup(void)
{
    wifi_power(0);
    printk(KERN_INFO "Cleaning up module.\n");
}

module_init(hello_init);
module_exit(hello_cleanup);
