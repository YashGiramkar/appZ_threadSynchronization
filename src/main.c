
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/random/random.h>
#include <string.h>

#define PRODUCER_STACKSIZE 1024
#define CONSUMER_STACKSIZE 1024

#define PRODUCER_PRIORITY        5
#define CONSUMER_PRIORITY        5

// Define semaphore to monitor instances of available resource
K_SEM_DEFINE(instance_monitor_sem, 10, 10);

// Initialize the available instances of this resource
volatile uint32_t available_instance_count = 10;

// Function for getting access of resource
void get_access(void)
{
	// Take semaphore when getting access to the resource *
   k_sem_take(&instance_monitor_sem, K_FOREVER);
   printk("Resource taken and available_instance_count = %d\n", k_sem_count_get(&instance_monitor_sem));

	// Decrement available resource
   // available_instance_count--;
   // printk("Resource taken and available_instance_count = %d\n",  available_instance_count);
}

// Function for releasing access of resource
void release_access(void)
{
	// Give semaphore when releasing access to resource
   k_sem_give(&instance_monitor_sem);
	printk("Resource given and available_instance_count = %d\n", k_sem_count_get(&instance_monitor_sem));

	// Increment available resource
   // available_instance_count++;
   // printk("Resource given and available_instance_count = %d\n", available_instance_count);
}

// Producer thread relinquishing access to instance
void producer(void)
{
	printk("Producer thread started\n");
	while (1) {
		release_access();
		k_msleep(sys_rand32_get() % 10);
	}
}

// Consumer thread obtaining access to instance
void consumer(void)
{
	printk("Consumer thread started\n");
	while (1) {
		get_access();
		k_msleep(sys_rand32_get() % 10);
	}
}

// Define and initialize threads
K_THREAD_DEFINE(producer_id, PRODUCER_STACKSIZE, producer, NULL, NULL, NULL, PRODUCER_PRIORITY, 0,
		0);

K_THREAD_DEFINE(consumer_id, CONSUMER_STACKSIZE, consumer, NULL, NULL, NULL, CONSUMER_PRIORITY, 0,
		0);