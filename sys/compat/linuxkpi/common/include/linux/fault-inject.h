/*
 * Copyright (c) 2026 Lutz Bichler
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _LINUXKPI_FAULT_INJECT_H_
#define _LINUXKPI_FAULT_INJECT_H_

#define	DECLARE_FAULT_ATTR(attr)

#define	should_fail(attr, size)	false

#define	fault_create_debugfs_attr(name, parent, attr)

#endif /* _LINUKPI_FAULT_INJECT_H_ */ 
