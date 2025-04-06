/* Public domain */

#ifndef LINUXKPI_IRQFLAGS_H_
#define LINUXKPI_IRQFLAGS_H_

#define local_irq_save(flags) 		do { (void)flags; preempt_disable(); } while (0)
#define local_irq_restore(flags)	do { (void)flags; preempt_enable(); } while (0)

#endif /* LINUXKPI_IRQFLAGS_H_ */
