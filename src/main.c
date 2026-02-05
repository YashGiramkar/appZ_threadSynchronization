
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/random/random.h>
#include <string.h>

#define PRODUCER_STACKSIZE                   512
#define CONSUMER_STACKSIZE                   512
#define MUTEX_THREAD0_STACKSIZE              512
#define MUTEX_THREAD1_STACKSIZE              512

#define PRODUCER_PRIORITY                    5
#define CONSUMER_PRIORITY                    5

#define MUTEX_THREAD0_PRIORITY               5
#define MUTEX_THREAD1_PRIORITY               5

#define COMBINED_TOTAL                       40
int32_t increment_count = 0;
int32_t decrement_count = COMBINED_TOTAL;

// Define semaphore to monitor instances of available resource
K_SEM_DEFINE(instance_monitor_sem, 10, 10);
// Define mutex to protect shared code section
K_MUTEX_DEFINE(test_mutex);

// Initialize the available instances of this resource
volatile uint32_t available_instance_count = 10;

// Shared code run by both threads
void shared_code_section(void)
{
   uint8_t race_condition = 0;
   int32_t increment_count_copy = 0;
   int32_t decrement_count_copy = 0;

   k_mutex_lock(&test_mutex, K_FOREVER);

   increment_count += 1;
   increment_count = increment_count % COMBINED_TOTAL;

   decrement_count -= 1;
   if (decrement_count == 0)
   {
      decrement_count = COMBINED_TOTAL;
   }

       if (increment_count + decrement_count != COMBINED_TOTAL) {
        race_condition = 1;
        increment_count_copy = increment_count;
        decrement_count_copy = decrement_count;
    }

   if( race_condition )
   {
      printk("Race condition happend!\n");
      printk("Increment_count (%d) + Decrement_count (%d) = %d \n", increment_count_copy,
         decrement_count_copy, (increment_count_copy + decrement_count_copy));
      k_msleep(400 + sys_rand32_get() % 10);
   }

}

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

void thread0(void)
{
   printk("Thread 0 started\n");
   while (1) {
      shared_code_section();
   }
}

void thread1(void)
{
   printk("Thread 1 started\n");
   while (1) {
      // shared_code_section();
   }
}

// Define and initialize threads
K_THREAD_DEFINE(producer_id, PRODUCER_STACKSIZE, producer, NULL, NULL, NULL, PRODUCER_PRIORITY, 0,0);
K_THREAD_DEFINE(consumer_id, CONSUMER_STACKSIZE, consumer, NULL, NULL, NULL, CONSUMER_PRIORITY, 0,0);
K_THREAD_DEFINE(mutex_thread0_id, MUTEX_THREAD0_STACKSIZE, thread0, NULL, NULL, NULL, MUTEX_THREAD0_PRIORITY, 0,0);
K_THREAD_DEFINE(mutex_thread1_id, MUTEX_THREAD1_STACKSIZE, thread1, NULL, NULL, NULL, MUTEX_THREAD1_PRIORITY, 0,0);