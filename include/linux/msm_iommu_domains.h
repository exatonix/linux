/*
 * Copyright (c) 2011-2014, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _LINUX_MSM_IOMMU_DOMAINS_H
#define _LINUX_MSM_IOMMU_DOMAINS_H

#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/genalloc.h>
#include <linux/rbtree.h>
#include <linux/dma-buf.h>

#define MSM_IOMMU_DOMAIN_SECURE		0x1

enum {
	VIDEO_DOMAIN,
	CAMERA_DOMAIN,
	DISPLAY_READ_DOMAIN,
	DISPLAY_WRITE_DOMAIN,
	ROTATOR_SRC_DOMAIN,
	ROTATOR_DST_DOMAIN,
	MAX_DOMAINS
};

enum {
	VIDEO_FIRMWARE_POOL,
	VIDEO_MAIN_POOL,
	GEN_POOL,
};

struct mem_pool {
	struct mutex pool_mutex;
	struct gen_pool *gpool;
	phys_addr_t paddr;
	unsigned long size;
	unsigned long free;
	unsigned int id;
};

struct msm_iommu_domain {
	/* iommu domain to map in */
	struct iommu_domain *domain;
	/* total number of allocations from this domain */
	atomic_t allocation_cnt;
	/* number of iova pools */
	int npools;
	/*
	 * array of gen_pools for allocating iovas.
	 * behavior is undefined if these overlap
	 */
	struct mem_pool *iova_pools;
};

struct msm_iommu_domain_name {
	char *name;
	int domain;
};

struct iommu_domains_pdata {
	struct msm_iommu_domain *domains;
	int ndomains;
	struct msm_iommu_domain_name *domain_names;
	int nnames;
	unsigned int domain_alloc_flags;
};

struct msm_iova_partition {
	unsigned long start;
	unsigned long size;
};

struct msm_iova_layout {
	struct msm_iova_partition *partitions;
	int npartitions;
	const char *client_name;
	unsigned int domain_flags;
	unsigned int is_secure;
};

#if defined(CONFIG_QCOM_IOMMU)
extern void msm_iommu_set_client_name(struct iommu_domain *domain,
				      char const *name);
extern struct iommu_domain *msm_get_iommu_domain(int domain_num);
extern int msm_allocate_iova_address(unsigned int iommu_domain,
				     unsigned int partition_no,
				     unsigned long size,
				     unsigned long align,
				     unsigned long *iova);

extern void msm_free_iova_address(unsigned long iova,
				  unsigned int iommu_domain,
				  unsigned int partition_no,
				  unsigned long size);

extern int msm_use_iommu(void);

extern int msm_iommu_map_extra(struct iommu_domain *domain,
			       unsigned long start_iova,
			       phys_addr_t phys_addr,
			       unsigned long size,
			       unsigned long page_size,
			       int cached);

extern void msm_iommu_unmap_extra(struct iommu_domain *domain,
				  unsigned long start_iova,
				  unsigned long size,
				  unsigned long page_size);

extern int msm_iommu_map_contig_buffer(phys_addr_t phys,
				       unsigned int domain_no,
				       unsigned int partition_no,
				       unsigned long size,
				       unsigned long align,
				       unsigned long cached,
				       dma_addr_t *iova_val);

extern void msm_iommu_unmap_contig_buffer(dma_addr_t iova,
					  unsigned int domain_no,
					  unsigned int partition_no,
					  unsigned long size);

extern int msm_register_domain(struct msm_iova_layout *layout);
extern int msm_unregister_domain(struct iommu_domain *domain);

int msm_map_dma_buf(struct dma_buf *dma_buf, struct sg_table *table,
		    int domain_num, int partition_num, unsigned long align,
		    unsigned long iova_length, unsigned long *iova,
		    unsigned long *buffer_size,
		    unsigned long flags, unsigned long iommu_flags);

void msm_unmap_dma_buf(struct sg_table *table, int domain_num,
		       int partition_num);
#else
static inline void msm_iommu_set_client_name(struct iommu_domain *domain,
					     char const *name)
{
}

static inline struct iommu_domain *msm_get_iommu_domain(int subsys_id)
{
	return NULL;
}

static inline int msm_allocate_iova_address(unsigned int iommu_domain,
					    unsigned int partition_no,
					    unsigned long size,
					    unsigned long align,
					    unsigned long *iova)
{
	return -ENOMEM;
}

static inline void msm_free_iova_address(unsigned long iova,
					 unsigned int iommu_domain,
					 unsigned int partition_no,
					 unsigned long size) { }

static inline int msm_use_iommu(void)
{
	return 0;
}

static inline int msm_iommu_map_extra(struct iommu_domain *domain,
				      unsigned long start_iova,
				      phys_addr_t phys_addr,
				      unsigned long size,
				      unsigned long page_size,
				      int cached)
{
	return -ENODEV;
}

static inline void msm_iommu_unmap_extra(struct iommu_domain *domain,
					 unsigned long start_iova,
					 unsigned long size,
					 unsigned long page_size)
{
}

static inline int msm_iommu_map_contig_buffer(phys_addr_t phys,
					      unsigned int domain_no,
					      unsigned int partition_no,
					      unsigned long size,
					      unsigned long align,
					      unsigned long cached,
					      dma_addr_t *iova_val)
{
	*iova_val = phys;
	return 0;
}

static inline void msm_iommu_unmap_contig_buffer(dma_addr_t iova,
						 unsigned int domain_no,
						 unsigned int partition_no,
						 unsigned long size)
{
}

static inline int msm_register_domain(struct msm_iova_layout *layout)
{
	return -ENODEV;
}

static inline int msm_unregister_domain(struct iommu_domain *domain)
{
	return -ENODEV;
}

static inline int msm_map_dma_buf(struct dma_buf *dma_buf,
				  struct sg_table *table,
				  int domain_num, int partition_num,
				  unsigned long align,
				  unsigned long iova_length,
				  unsigned long *iova,
				  unsigned long *buffer_size,
				  unsigned long flags,
				  unsigned long iommu_flags)
{
	return -ENODEV;
}

static inline void msm_unmap_dma_buf(struct sg_table *table, int domain_num,
				     int partition_num)
{

}

#endif /* CONFIG_QCOM_IOMMU */
#endif /* _LINUX_MSM_IOMMU_DOMAINS_H */