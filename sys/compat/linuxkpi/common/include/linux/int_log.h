/* Public domain */

#ifndef LINUXKPI_INT_LOG_H
#define LINUXKPI_INT_LOG_H

unsigned int linuxkpi_intlog2(uint32_t v);
#define	intlog2(v)	linuxkpi_intlog2(v)

unsigned int linuxkpi_intlog10(uint32_t v);
#define	intlog10(v)	linuxkpi_intlog10(v)

#endif /* LINUKPI_INT_LOG_H */
