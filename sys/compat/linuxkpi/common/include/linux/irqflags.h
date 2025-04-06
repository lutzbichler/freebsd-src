/* Public domain */

#ifndef LINUXKPI_IRQFLAGS_H_
#define LINUXKPI_IRQFLAGS_H_

#define local_irq_disable()             preempt_disable()
#define local_irq_enable()              preempt_enable()

#define	local_irq_save(flags) do {		\
	(flags) = 0;				\
} while (0)

#define	local_irq_restore(flags) do {		\
	(void)(flags);				\
} while (0)

#endif /* LINUXKPI_IRQFLAGS_H_ */
