#include <linux/kernel.h> /* необходим для pr_info() */
#include <linux/module.h> /* необходим для всех модулей */
int init_module(void)
{
pr_info("Welcome to the Tomsk State University\n");
/* Если вернётся не 0, значит, init_module провалилась; модули загрузить не
получится. */
return 0;
}
void cleanup_module(void)
{
pr_info("Tomsk State University forever!\n");
}
MODULE_LICENSE("GPL");
