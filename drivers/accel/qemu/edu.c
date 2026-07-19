// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2026 wuliyong3155@163.com.
 */

#include <linux/cdev.h>
#include <linux/firmware.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/pci.h>
#include <linux/printk.h>

#include <uapi/drm/drm.h>

#include <drm/drm_accel.h>
#include <drm/drm_drv.h>
#include <drm/drm_gem.h>
#include <drm/drm_ioctl.h>
#include <drm/drm_prime.h>

#include "edu.h"

#define EDU_NAME			"edu"
#define EDU_DESC			"qemu npu simulation"
#define EDU_VENDOR_ID		(0x1234)
#define EDU_BAR0			(0)
#define EDU_MIN_VECS		(1)
#define EDU_MAX_VECS		(2)
#define EDU_DMA_BUFFER_SIZE	(1024)
#define EDU_DMA_RUN			(0x1)
#define EDU_DMA_IRQ			(0x4)

#define INIT_EDU_NVEC_INFO(n, h)		\
{						\
	.name = n,				\
	.handle = h,				\
}

enum {
	EDU_ENABLE = 0,
	EDU_DISABLE,
};

struct edu_device {
	/* device-wide serialization lock */
	struct mutex lock;
	struct pci_dev *pdev;
	struct drm_device ddev;

	const struct firmware *fw;

	struct mutex client_lock;
	struct list_head client_list;

	void __iomem *bar_0;
	int nr_vec;
	dma_addr_t dma_handle;
	void *dma_buf;

	struct hrtimer hrtimer;
};


struct edu_nvec_info {
	char name[16];
	irq_handler_t handle;
};

struct edu_buffer {
	void *vaddr;
	size_t size;
};

struct edu_bo {
};

static irqreturn_t edu_irq_handler(int irq, void *data)
{
	return IRQ_HANDLED;
}

static enum hrtimer_restart edu_timer(struct hrtimer *hrtimer)
{
	return HRTIMER_RESTART;
}

static struct edu_nvec_info nvec_info[] = {
	INIT_EDU_NVEC_INFO("irq_tx", edu_irq_handler),
	INIT_EDU_NVEC_INFO("irq_rx", edu_irq_handler),
};

/*
static int edu_device_enable(struct edu_device *edev,
				  struct edu_cmd *cmd)
{
	pr_info("bar0 offset0: %u\n", readl(edev->iomem[EDU_BAR0]));

	hrtimer_start(&edev->hrtimer,
		      ns_to_ktime(1000000), HRTIMER_MODE_REL);
	memset(edev->dma_buf, 0xa5, EDU_DMA_BUFFER_SIZE);
	writel(edev->dma_handle, edev->iomem[EDU_BAR0] + 0x80);
	writel(0x40000, edev->iomem[EDU_BAR0] + 0x88);
	writel(EDU_DMA_BUFFER_SIZE, edev->iomem[EDU_BAR0] + 0x90);
	writel(EDU_DMA_RUN | EDU_DMA_IRQ, edev->iomem[EDU_BAR0] + 0x98);
	return sizeof(*cmd);
}

static int edu_dma_transfer(struct edu_device *edev,
				 struct edu_buffer *buffer)
{
	struct pci_dev *pdev = edev->pdev;
	dma_addr_t dma_addr;

	dma_addr = dma_map_single(&pdev->dev, buffer->vaddr,
				  buffer->size, DMA_TO_DEVICE);
	dma_sync_single_for_device(&pdev->dev, dma_addr,
				   buffer->size, DMA_TO_DEVICE);
	writel(dma_addr, edev->iomem[EDU_BAR0] + 0x80);
	writel(0x40000, edev->iomem[EDU_BAR0] + 0x88);
	writel(EDU_DMA_BUFFER_SIZE, edev->iomem[EDU_BAR0] + 0x90);
	writel(EDU_DMA_RUN | EDU_DMA_IRQ, edev->iomem[EDU_BAR0] + 0x98);
}

static int edu_device_disable(struct edu_device *edev,
				   struct edu_cmd *cmd)
{
	struct pci_dev *pdev = edev->pdev;
	void *buffer = NULL;
	u32 buf_len = 1024 * 1024;

	buffer = kmalloc(buf_len, GFP_KERNEL);
	memset(buffer, 0x5a, buf_len);
	dma_addr = dma_map_single(&pdev->dev, buffer, buf_len, DMA_TO_DEVICE);

	return sizeof(*cmd);
}

static int edu_execute_cmd(struct edu_device *edev,
				struct edu_cmd *cmd)
{
	int retval = 0;
	u8 action = cmd->action;

	mutex_lock(&edev->lock);
	switch (action) {
	case EDU_ENABLE:
		retval = edu_device_enable(edev, cmd);
		break;
	case EDU_DISABLE:
		retval = edu_device_disable(edev, cmd);
		break;
	default:
		pr_err("unknown action %u\n", action);
		retval = -EINVAL;
	}
	mutex_unlock(&edev->lock);
	return retval;
}

static ssize_t edu_write(struct file *filp, const char __user *buf,
			      size_t count, loff_t *f_pos)
{
	struct miscdevice *mdev = filp->private_data;
	struct edu_device *edev = container_of(mdev, struct edu_device, mdev);
	struct edu_cmd cmd;

	memset(&cmd, 0x00, sizeof(cmd));
	if (copy_from_user(&cmd, buf, count) > 0)
		return -EINVAL;

	return edu_execute_cmd(edev, &cmd);
}

static long edu_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	void __user *ubuf = (void __user *)arg;
	struct ioctl_packet packet;
	unsigned int type = _IOC_TYPE(cmd);
	unsigned int size = _IOC_SIZE(cmd);

	if (type != EDU_MAGIC)
		return -EINVAL;

	if (size != sizeof(struct ioctl_packet))
		return -EINVAL;

	if (copy_from_user(&packet, ubuf, sizeof(packet)))
		return -EFAULT;

	switch (cmd) {
	case EDU_VERSION:
		if (copy_to_user(ubuf, &packet, sizeof(packet)))
			return -EFAULT;
		break;
	}
	return 0;
}
*/

DEFINE_DRM_ACCEL_FOPS(edu_accel_fops);

static inline struct edu_device* to_edu_dev(struct drm_device *ddev)
{
	return container_of(ddev, struct edu_device, ddev);
}

static struct edu_client* edu_client_create(struct edu_device *edev)
{
	struct edu_client *client = NULL;

	client = kmalloc(sizeof(*client), GFP_KERNEL);
	if (!client)
		return NULL;

	client->edev = edev;
	mutex_lock(&edev->client_lock);
	list_add(&client->list, &edev->client_list);
	mutex_unlock(&edev->client_lock);
	return client;
}

static void edu_client_destory(struct edu_client *client)
{
	struct edu_device *edev = client->edev;

	mutex_lock(&edev->client_lock);
	list_del(&client->list);
	mutex_unlock(&edev->client_lock);
	kfree(client);
}

static int edu_open(struct drm_device *ddev, struct drm_file *file)
{
	struct edu_device *edev = to_edu_dev(ddev);
	struct edu_client *client = NULL;

	client = edu_client_create(edev);
	if (!client)
		return -ENOMEM;

	file->driver_priv = client;
	return 0;
}

static void edu_postclose(struct drm_device *ddev, struct drm_file *file)
{
	struct edu_client *client = file->driver_priv;

	edu_client_destory(client);
	file->driver_priv = NULL;
}

static int edu_manage_ioctl(struct drm_device *dev, void *data,
			    struct drm_file *file_priv)
{
	return 0;
}

static int edu_create_bo_ioctl(struct drm_device *dev, void *data,
			       struct drm_file *file_priv)
{
	struct edu_bo *bo;

	bo = kzalloc(sizeof(*bo), GFP_KERNEL);
	if (!bo)
		return -ENOMEM;

	// dma_alloc_coherent();
	return 0;
}

static const struct drm_ioctl_desc edu_ioctls[] = {
	DRM_IOCTL_DEF_DRV(EDU_MANAGE, edu_manage_ioctl, 0),
	DRM_IOCTL_DEF_DRV(EDU_CREATE_BO, edu_create_bo_ioctl, 0),
};

static struct drm_gem_object* edu_prime_import(struct drm_device *dev,
						    struct dma_buf *dma_buf)
{
	return NULL;
}

static const struct drm_driver edu_drm_driver = {
	.driver_features	= DRIVER_GEM | DRIVER_COMPUTE_ACCEL,

	.name			= EDU_NAME,
	.desc			= EDU_DESC,
	.date			= "20260620",

	.fops			= &edu_accel_fops,
	.open			= edu_open,
	.postclose		= edu_postclose,

	.ioctls			= edu_ioctls,
	.num_ioctls		= ARRAY_SIZE(edu_ioctls),
	.prime_fd_to_handle	= drm_gem_prime_fd_to_handle,
	.gem_prime_import	= edu_prime_import,
};

static int edu_pci_init(struct pci_dev *pdev, struct edu_device *edev)
{
	int ret = 0;
	u32 i = 0;

	ret = pci_enable_device(pdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to enable device\n");
		return ret;
	}

	edev->bar_0 = pcim_iomap(pdev, EDU_BAR0, 0);
	if (!edev->bar_0)
		return -ENOSPC;

	ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to set dma %d\n", ret);
		goto pci_disable;
	}

	edev->dma_buf = dmam_alloc_attrs(&pdev->dev, EDU_DMA_BUFFER_SIZE,
					&edev->dma_handle, GFP_KERNEL,
					DMA_ATTR_WRITE_COMBINE);
	if (IS_ERR_OR_NULL(edev->dma_buf)) {
		if (!edev->dma_buf)
			ret = -ENOMEM;
		else
			ret = PTR_ERR(edev->dma_buf);
		goto pci_disable;
	}

	edev->nr_vec = pci_alloc_irq_vectors(pdev, EDU_MIN_VECS,
					    EDU_MAX_VECS,
					    PCI_IRQ_MSIX | PCI_IRQ_MSI |
					    PCI_IRQ_LEGACY);
	if (edev->nr_vec < EDU_MIN_VECS) {
		dev_err(&pdev->dev, "failed to alloc irq %d\n", edev->nr_vec);
		ret = -ENOSPC;
		goto pci_disable;
	}

	for (i = 0; i < edev->nr_vec; i++) {
		ret = request_irq(pci_irq_vector(pdev, i), nvec_info[i].handle,
				  0, nvec_info[i].name, edev);
		if (ret < 0) {
			dev_err(&pdev->dev, "failed to requeset irq\n");
			goto free_irq_vector;
		}
	}

	pci_set_master(pdev);
	pci_set_drvdata(pdev, edev);
	return 0;

free_irq_vector:
	while (i--)
		free_irq(pci_irq_vector(pdev, i), edev);
	pci_free_irq_vectors(pdev);
pci_disable:
	pci_disable_device(pdev);
	return ret;
}

static int edu_dev_init(struct edu_device *edev, struct pci_dev *pdev)
{
	INIT_LIST_HEAD(&edev->client_list);

	hrtimer_init(&edev->hrtimer,
		     CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	edev->hrtimer.function = edu_timer;
	mutex_init(&edev->lock);
	mutex_init(&edev->client_lock);
	edev->pdev = pdev;
	return 0;
}

static int edu_fw_init(struct edu_device *edev, struct pci_dev *pdev)
{
	int ret = 0;

	ret = request_firmware(&edev->fw, "edu_firmware.bin", &pdev->dev);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to request firmware %d\n", ret);
		dump_stack();
	}

	return 0;
}

static int edu_pci_driver_probe(struct pci_dev *pdev,
				 const struct pci_device_id *id)
{
	int ret = 0;
	struct edu_device *edev = NULL;

	pr_info("%s\n", __func__);

	edev = devm_drm_dev_alloc(&pdev->dev, &edu_drm_driver,
				 struct edu_device, ddev);
	if (!edev)
		return -ENOMEM;

	ret = edu_pci_init(pdev, edev);
	if (ret < 0)
		return ret;

	ret = edu_dev_init(edev, pdev);
	if (ret < 0)
		return ret;

	ret = edu_fw_init(edev, pdev);
	if (ret < 0)
		return ret;

	return drm_dev_register(&edev->ddev, 0);
}

static void edu_pci_driver_remove(struct pci_dev *pdev)
{
	struct edu_device *edev = pci_get_drvdata(pdev);
	u32 i = 0;

	drm_dev_unregister(&edev->ddev);

	for (i = 0; i < edev->nr_vec; i++)
		free_irq(pci_irq_vector(pdev, i), edev);

	pci_free_irq_vectors(pdev);
	pci_disable_device(pdev);
}

static struct pci_device_id edu_ids[] = {
	{ PCI_DEVICE(EDU_VENDOR_ID, 0x11e8) },
	{ 0, },
};
MODULE_DEVICE_TABLE(pci, edu_ids);

static struct pci_driver edu_driver = {
	.name = EDU_NAME,
	.id_table = edu_ids,
	.probe = edu_pci_driver_probe,
	.remove = edu_pci_driver_remove,
};

module_pci_driver(edu_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wuliyong3155@163.com");
MODULE_DESCRIPTION("QEMU EDU Driver");
MODULE_VERSION("1.0");
