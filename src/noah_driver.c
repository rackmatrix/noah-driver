// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Broachlink Noah2/Noah3/Noah4/Noah5/Noah6 board platform driver
 * for front button and LEDs
 *
 * Copyright (c) 2024 Rack Matrix Technology <wwww.rack-matrix.com>
 * Author: David RENÉ <developers_at_rack-matrix.com>
 */

#include <linux/dmi.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/ioport.h>

// ------------- CHARACTERS DRIVER PART ------------
#define MAX_DEV 30

#define MODE_SW1_MINOR 0
#define LED1_MINOR 1
#define LED2_MINOR 2
#define LED3_MINOR 3

// ------------------- GPIO PART -------------------

unsigned short SOC_GPIO_BASE = 0x500;

// FRONT PUSH BUTTON
#define MODE_SW1 1 << 24 // GPIO_S0_SC56
#define MODE_SW1_BIT 24

// FRONT LED
// BIT position in GPIO_S0_SCx
#define LED1_BIT 28
#define LED2_BIT 27
#define LED3_BIT 26

// mask
#define LED1 1 << LED1_BIT // GPIO_S0_SC60
#define LED2 1 << LED2_BIT // GPIO_S0_SC59
#define LED3 1 << LED3_BIT // GPIO_S0_SC58

static int led_status = LED1; // DEFAULT configuration

static void write_IOPort_DWord(int addr, int val)
{
  outl_p(val, addr);
}

static int read_IOPort_DWord(unsigned short reg)
{
  unsigned int value = inl(reg);
  return value;
}

static void init_LED_BUTTON(void)
{
  unsigned int temp = 0;

  // select LED and MODE_SW1 gpio
  temp = read_IOPort_DWord((unsigned short)(SOC_GPIO_BASE + 0x20));
  temp |= (LED1 | LED2 | LED3 | MODE_SW1);
  write_IOPort_DWord((unsigned short)(SOC_GPIO_BASE + 0x20), temp);

  // set LED gpio to 0 (output)
  temp = read_IOPort_DWord((unsigned short)(SOC_GPIO_BASE + 0x24));
  temp &= ~(LED1 | LED2 | LED3);
  write_IOPort_DWord((unsigned short)(SOC_GPIO_BASE + 0x24), temp);
  // set LED to default configuration status (LED1 ON, LED2 OFF, LED3 OFF)
  write_IOPort_DWord((unsigned short)(SOC_GPIO_BASE + 0x28), ~led_status);

  // set MODE_SW1 gpio to 1 (input)
  temp = read_IOPort_DWord((unsigned short)(SOC_GPIO_BASE + 0x24));
  temp |= MODE_SW1;
  write_IOPort_DWord((unsigned short)(SOC_GPIO_BASE + 0x24), temp);

  // set MODE_SW1 gpio to 1 (Trigger Negative Enable)
  temp = read_IOPort_DWord((unsigned short)(SOC_GPIO_BASE + 0x30));
  temp |= MODE_SW1;
  write_IOPort_DWord((unsigned short)(SOC_GPIO_BASE + 0x30), temp);
}

int read_Mode_SW1(void)
{
  // Read the value from the port
  int value = read_IOPort_DWord((unsigned short)(SOC_GPIO_BASE + 0x28));
  // Reverse the value (because the value is not natural 0 = pressed and 1 = unpressed)
  // So we reverse the value to get 1 = pressed and 0 = unpressed
  // Isolate the MODE_SW1 and shift it to get 0 or 1.
  value = ((~value & MODE_SW1) >> MODE_SW1_BIT);
  // printk("Mode_SW1 button [%d] \r\n", value);
  return value;
}

int get_LED_mask(int led)
{
  switch (led)
  {
  case LED1_MINOR:
    return LED1;
  case LED2_MINOR:
    return LED2;
  case LED3_MINOR:
    return LED3;
  default:
    printk(KERN_WARNING "get_LED_mask not found for %d", led);
    return 0;
  }
}

int get_LED_bit(int led)
{
  switch (led)
  {
  case LED1_MINOR:
    return LED1_BIT;
  case LED2_MINOR:
    return LED2_BIT;
  case LED3_MINOR:
    return LED3_BIT;
  default:
    printk(KERN_WARNING "get_LED_bit not found for %d", led);
    return 0;
  }
}

int get_front_LED_status(int led)
{
  unsigned int led_bit;
  led_bit = get_LED_bit(led);

  if (led_bit != 0)
  {
    int result;
    result = (led_status >> led_bit) & 1;
    return result;
  }
  else
    return -1; // Unknown LED
}

void front_LED_ON(int led)
{
  unsigned int led_mask;
  led_mask = get_LED_mask(led);

  if (led_mask != 0)
  {
    // update the bit in led_status, to keep it for the next time
    led_status |= led_mask;
    // write it on the port with reversed value (0 = lighted, 1 = unlighted)
    write_IOPort_DWord((unsigned short)(SOC_GPIO_BASE + 0x28), ~led_status);
  }
}

void front_LED_OFF(int led)
{
  unsigned int led_mask;
  led_mask = get_LED_mask(led);

  if (led_mask != 0)
  {
    // update the bit in led_status, to keep it for the next time
    led_status &= ~led_mask;
    // write it on the port with reversed value (0 = lighted, 1 = unlighted)
    write_IOPort_DWord((unsigned short)(SOC_GPIO_BASE + 0x28), ~led_status);
  }
}

// Noah LEDs file operations
static int noah_led_open(struct inode *inode, struct file *file);
static int noah_led_release(struct inode *inode, struct file *file);
static ssize_t noah_led_read(struct file *file, char __user *buf, size_t count, loff_t *offset);
static ssize_t noah_led_write(struct file *file, const char __user *buf, size_t count, loff_t *offset);

static const struct file_operations noah_led_fops = {
    .owner = THIS_MODULE,
    .open = noah_led_open,
    .release = noah_led_release,
    .read = noah_led_read,
    .write = noah_led_write};

// Noah button file operations
static int noah_button_open(struct inode *inode, struct file *file);
static int noah_button_release(struct inode *inode, struct file *file);
static ssize_t noah_button_read(struct file *file, char __user *buf, size_t count, loff_t *offset);

static const struct file_operations noah_button_fops = {
    .owner = THIS_MODULE,
    .open = noah_button_open,
    .release = noah_button_release,
    .read = noah_button_read,
};

struct noah_device_data
{
  struct cdev cdev;
};

static int dev_major = 0;
static struct class *noah_led_class = NULL;
static struct class *noah_button_class = NULL;

static struct noah_device_data noahdev_data[MAX_DEV];

static const struct dmi_system_id noah_systems[] __initconst = {
    {
        .ident = "noah2",
        .matches = {
            DMI_EXACT_MATCH(DMI_SYS_VENDOR, "Broachlink"),
            DMI_MATCH(DMI_PRODUCT_NAME, "NOAH V2 E3845"),
        },
    },
    {
        .ident = "noah3",
        .matches = {
            DMI_EXACT_MATCH(DMI_SYS_VENDOR, "Broachlink"),
            DMI_MATCH(DMI_PRODUCT_NAME, "NOAH3 V10 E3845"),
        },
    },
    {
        .ident = "noah4",
        .matches = {
            DMI_EXACT_MATCH(DMI_SYS_VENDOR, "Broachlink"),
            DMI_MATCH(DMI_PRODUCT_NAME, "NOAH4 V10 E3845"),
        },
    },
    {
        .ident = "noah5",
        .matches = {
            DMI_EXACT_MATCH(DMI_SYS_VENDOR, "Broachlink"),
            DMI_MATCH(DMI_PRODUCT_NAME, "NOAH5 V10 E3845"),
        },
    },
    {
        .ident = "noah6",
        .matches = {
            DMI_EXACT_MATCH(DMI_SYS_VENDOR, "Broachlink"),
            DMI_MATCH(DMI_PRODUCT_NAME, "NOAH6 V10 E3845"),
        },
    },
    {} // Terminating entry
};

static int noah_uevent(struct device *dev, struct kobj_uevent_env *env)
{
  add_uevent_var(env, "DEVMODE=%#o", 0666);
  return 0;
}

static int __init noah_board_init(void)
{
  const struct dmi_system_id *system;
  int err, i;
  dev_t dev;

  printk(KERN_INFO "Loading Broachlink Noah2/Noah3/Noah4/Noah5/Noah6 board GPIO/LEDs/keys module...\n");

  system = dmi_first_match(noah_systems);
  if (!system)
  {
    printk(KERN_INFO "Broachlink Noah[x] motherboard not detected. Aborting driver loading");
    return -ENODEV;
  }

  printk(KERN_INFO "%s Motherboard detected\n", system->ident);
  err = alloc_chrdev_region(&dev, 0, 4, "noah!");

  dev_major = MAJOR(dev);

  noah_led_class = class_create(THIS_MODULE, "noah!led");
  noah_led_class->dev_uevent = noah_uevent;

  for (i = LED1_MINOR; i < LED1_MINOR + 3; i++)
  {
    cdev_init(&noahdev_data[i].cdev, &noah_led_fops);
    noahdev_data[i].cdev.owner = THIS_MODULE;

    cdev_add(&noahdev_data[i].cdev, MKDEV(dev_major, i), 1);
    device_create(noah_led_class, NULL, MKDEV(dev_major, i), NULL, "noah!led%d", i);
  }

  noah_button_class = class_create(THIS_MODULE, "noah!button");
  noah_button_class->dev_uevent = noah_uevent;

  cdev_init(&noahdev_data[0].cdev, &noah_button_fops);
  noahdev_data[0].cdev.owner = THIS_MODULE;

  cdev_add(&noahdev_data[0].cdev, MKDEV(dev_major, 0), 1);
  device_create(noah_button_class, NULL, MKDEV(dev_major, 0), NULL, "noah!button");

  init_LED_BUTTON();

  return 0;
}

static void __exit noah_board_exit(void)
{
  int i;

  for (i = LED1_MINOR; i < LED1_MINOR + 3; i++)
  {
    device_destroy(noah_led_class, MKDEV(dev_major, i));
  }
  class_unregister(noah_led_class);
  class_destroy(noah_led_class);

  device_destroy(noah_button_class, MKDEV(dev_major, 0));
  class_unregister(noah_button_class);
  class_destroy(noah_button_class);

  unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);

  printk(KERN_INFO "Unloading Broachlink Noah2/Noah3/Noah4/Noah5/Noah6 board GPIO/LEDs/keys module...\n");
}

// Files operations for LEDs

static int noah_led_open(struct inode *inode, struct file *file)
{
  return 0;
}

static int noah_led_release(struct inode *inode, struct file *file)
{
  return 0;
}

static ssize_t noah_led_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
  uint8_t *led_on = "1";
  uint8_t *led_off = "0";
  int status = -1;

  int minor = MINOR(file->f_path.dentry->d_inode->i_rdev);

  if (count > 1)
  {
    count = 1;
  }

  status = get_front_LED_status(minor);

  if (status == 1)
  {
    if (copy_to_user(buf, led_on, count))
    {
      return -EFAULT;
    }
  }
  else if (status == 0)
  {
    if (copy_to_user(buf, led_off, count))
    {
      return -EFAULT;
    }
  }
  else
  {
    return 0;
  }

  return count;
}

static ssize_t noah_led_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
  size_t maxdatalen = 10, ncopied;
  uint8_t databuf[10];
  int minor = MINOR(file->f_path.dentry->d_inode->i_rdev);

  if (count < maxdatalen)
  {
    maxdatalen = count;
  }

  ncopied = copy_from_user(databuf, buf, maxdatalen);

  if (strncmp(databuf, "1", 1) == 0)
  {
    front_LED_ON(minor);
  }
  else if (strncmp(databuf, "ON", 2) == 0)
  {
    front_LED_ON(minor);
  }
  else if (strncmp(databuf, "0", 1) == 0)
  {
    front_LED_OFF(minor);
  }
  else if (strncmp(databuf, "OFF", 3) == 0)
  {
    front_LED_OFF(minor);
  }
  else
  {
    databuf[maxdatalen] = 0;
    printk("Unrecognized received command [%s]\n", databuf);
  }

  return count;
}

// Files operations for button

static int noah_button_open(struct inode *inode, struct file *file)
{
  return 0;
}

static int noah_button_release(struct inode *inode, struct file *file)
{
  return 0;
}

static ssize_t noah_button_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
  uint8_t *data_on = "1";
  uint8_t *data_off = "0";
  int read_value;

  if (count > 1)
  {
    count = 1;
  }

  read_value = read_Mode_SW1();
  if (read_value == 1)
  {
    if (copy_to_user(buf, data_on, count))
    {
      return -EFAULT;
    }
  }
  else
  {
    if (copy_to_user(buf, data_off, count))
    {
      return -EFAULT;
    }
  }

  return count;
}

module_init(noah_board_init);
module_exit(noah_board_exit);

MODULE_AUTHOR("David RENÉ, Rack Matrix Technology <drene@rack-matrix.com>");
MODULE_DESCRIPTION("Broachlink Noah2/Noah3/Noah4/Noah5/Noah6 board GPIO/LEDs/keys driver");
MODULE_LICENSE("GPL");
MODULE_DEVICE_TABLE(dmi, noah_systems);
