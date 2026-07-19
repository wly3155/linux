/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2026 wuliyong3155@163.com.
 */

#include <linux/ioctl.h>
#include <linux/types.h>

#define DRM_EDU_MANAGE	0x00
#define DRM_EDU_BO		0x01

#define DRM_IOCTL_EDU_MANAGE		\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_EDU_MANAGE, struct edu_msg)
#define DRM_IOCTL_EDU_CREATE_BO		\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_EDU_BO,	struct edu_bo)

struct edu_cmd {
	u8 action;
	u8 reserved;
};

struct edu_msg {
};

struct edu_client {
	struct list_head list;
	struct edu_device *edev;
};

struct ioctl_packet {
	struct edu_cmd cmd;
};
