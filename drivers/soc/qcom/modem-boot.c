#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <soc/qcom/subsystem_restart.h>

static void *modem_pil = NULL;

static ssize_t boot_modem_store(struct kobject *kobj, struct kobj_attribute *attr,
                               const char *buf, size_t count)
{
    int boot = 0;
    if (sscanf(buf, "%d", &boot) == 1) {
        if (boot == 1 && !modem_pil) {
            pr_info("modem_boot: Calling subsystem_get('modem')...\n");
            modem_pil = subsystem_get("modem");
            if (IS_ERR(modem_pil)) {
                pr_err("modem_boot: subsystem_get('modem') failed: %ld\n", PTR_ERR(modem_pil));
                modem_pil = NULL;
            } else {
                pr_info("modem_boot: Modem booted successfully! handle=%p\n", modem_pil);
            }
        } else if (boot == 0 && modem_pil) {
            pr_info("modem_boot: Calling subsystem_put(modem_pil)...\n");
            subsystem_put(modem_pil);
            modem_pil = NULL;
        }
    }
    return count;
}

static ssize_t boot_modem_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", modem_pil ? 1 : 0);
}

static struct kobj_attribute boot_attr = __ATTR(boot, 0664, boot_modem_show, boot_modem_store);

static struct attribute *attrs[] = {
    &boot_attr.attr,
    NULL,
};

static struct attribute_group attr_group = {
    .attrs = attrs,
};

static struct kobject *modem_kobj = NULL;

static int __init modem_boot_init(void)
{
    int ret;
    modem_kobj = kobject_create_and_add("boot_modem", kernel_kobj);
    if (!modem_kobj)
        return -ENOMEM;
    ret = sysfs_create_group(modem_kobj, &attr_group);
    if (ret) {
        kobject_put(modem_kobj);
        return ret;
    }
    pr_info("modem_boot: sysfs /sys/kernel/boot_modem/boot created\n");
    return 0;
}

static void __exit modem_boot_exit(void)
{
    if (modem_pil)
        subsystem_put(modem_pil);
    if (modem_kobj) {
        sysfs_remove_group(modem_kobj, &attr_group);
        kobject_put(modem_kobj);
    }
}

module_init(modem_boot_init);
module_exit(modem_boot_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Modem Boot PIL sysfs trigger");
