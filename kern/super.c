#include <linux/init.h>
#include <linux/module.h>

static int __init aufs_init(void)
{
	pr_debug("aufs module loaded\n");
	return 0;
}

static void __exit aufs_fini(void)
{
	pr_debug("aufs module unloaded\n");
}

module_init(aufs_init);
module_exit(aufs_fini);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("kmu");
