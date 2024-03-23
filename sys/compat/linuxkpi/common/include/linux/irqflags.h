/* Public domain */

#ifndef LINUXKPI_IRQFLAGS_H_
#define LINUXKPI_IRQFLAGS_H_

#define local_irq_save(flags) 		preempt_disable()
#define local_irq_restore(flags)	preempt_enable()

#endif /* LINUXKPI_IRQFLAGS_H_ */
