/*
 * Copyright © 2012 Red Hat
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *      Dave Airlie <airlied@redhat.com>
 *      Rob Clark <rob.clark@linaro.org>
 *
 */

#include <linux/export.h>
#include <linux/dma-buf.h>
#include "drmP.h"

/*
 * DMA-BUF/GEM Object references and lifetime overview:
 *
 * On the export the dma_buf holds a reference to the exporting GEM
 * object. It takes this reference in handle_to_fd_ioctl, when it
 * first calls .prime_export and stores the exporting GEM object in
 * the dma_buf priv. This reference is released when the dma_buf
 * object goes away in the driver .release function.
 *
 * On the import the importing GEM object holds a reference to the
 * dma_buf (which in turn holds a ref to the exporting GEM object).
 * It takes that reference in the fd_to_handle ioctl.
 * It calls dma_buf_get, creates an attachment to it and stores the
 * attachment in the GEM object. When this attachment is destroyed
 * when the imported object is destroyed, we remove the attachment
 * and drop the reference to the dma_buf.
 *
 * Thus the chain of references always flows in one direction
 * (avoiding loops): importing_gem -> dmabuf -> exporting_gem
 *
 * Self-importing: if userspace is using PRIME as a replacement for flink
 * then it will get a fd->handle request for a GEM object that it created.
 * Drivers should detect this situation and return back the gem object
 * from the dma-buf private.
 */

struct drm_prime_member {
	struct list_head entry;
	struct dma_buf *dma_buf;
	uint32_t handle;
};

struct drm_prime_attachment {
	struct sg_table *sgt;
	enum dma_data_direction dir;
};

static int drm_gem_map_attach(struct dma_buf *dma_buf,
			      struct device *target_dev,
			      struct dma_buf_attachment *attach)
{
	struct drm_prime_attachment *prime_attach;
	struct drm_gem_object *obj = dma_buf->priv;
	struct drm_device *dev = obj->dev;

	prime_attach = kzalloc(sizeof(*prime_attach), GFP_KERNEL);
	if (!prime_attach)
		return -ENOMEM;

	prime_attach->dir = DMA_NONE;
	attach->priv = prime_attach;

	if (!dev->driver->gem_prime_pin)
		return 0;

	return dev->driver->gem_prime_pin(obj);
}

static void drm_gem_map_detach(struct dma_buf *dma_buf,
			       struct dma_buf_attachment *attach)
{
	struct drm_prime_attachment *prime_attach = attach->priv;
	struct drm_gem_object *obj = dma_buf->priv;
	struct drm_device *dev = obj->dev;
	struct sg_table *sgt;

	if (dev->driver->gem_prime_unpin)
		dev->driver->gem_prime_unpin(obj);

	if (!prime_attach)
		return;

	sgt = prime_attach->sgt;
	if (sgt) {
		if (prime_attach->dir != DMA_NONE)
			dma_unmap_sg(attach->dev, sgt->sgl, sgt->nents,
					prime_attach->dir);
		sg_free_table(sgt);
	}

	kfree(sgt);
	kfree(prime_attach);
	attach->priv = NULL;
}


static struct sg_table *drm_gem_map_dma_buf(struct dma_buf_attachment *attach,
		enum dma_data_direction dir)
{
	struct drm_prime_attachment *prime_attach = attach->priv;
	struct drm_gem_object *obj = attach->dmabuf->priv;
	struct sg_table *sgt;

	if (WARN_ON(dir == DMA_NONE || !prime_attach))
		return ERR_PTR(-EINVAL);

	/* return the cached mapping when possible */
	if (prime_attach->dir == dir)
		return prime_attach->sgt;

	/*
	 * two mappings with different directions for the same attachment are
	 * not allowed
	 */
	if (WARN_ON(prime_attach->dir != DMA_NONE))
		return ERR_PTR(-EBUSY);

	sgt = obj->dev->driver->gem_prime_get_sg_table(obj);

	if (!IS_ERR(sgt)) {
		if (!dma_map_sg(attach->dev, sgt->sgl, sgt->nents, dir)) {
			sg_free_table(sgt);
			kfree(sgt);
			sgt = ERR_PTR(-ENOMEM);
		} else {
			prime_attach->sgt = sgt;
			prime_attach->dir = dir;
		}
	}

	return sgt;
}

static void drm_gem_unmap_dma_buf(struct dma_buf_attachment *attach,
		struct sg_table *sgt, enum dma_data_direction dir)
{
	/* nothing to be done here */
}

void drm_gem_dmabuf_release(struct dma_buf *dma_buf)
{
	struct drm_gem_object *obj = dma_buf->priv;

	/* drop the reference on the export fd holds */
	drm_gem_object_unreference_unlocked(obj);
}
EXPORT_SYMBOL(drm_gem_dmabuf_release);

static void *drm_gem_dmabuf_kmap_atomic(struct dma_buf *dma_buf,
		unsigned long page_num)
{
	return NULL;
}

static void drm_gem_dmabuf_kunmap_atomic(struct dma_buf *dma_buf,
		unsigned long page_num, void *addr)
{

}


static void drm_gem_dmabuf_kunmap(struct dma_buf *dma_buf,
		unsigned long page_num, void *addr)
{

}

static int drm_gem_dmabuf_mmap(struct dma_buf *dma_buf,
		struct vm_area_struct *vma)
{
	struct drm_gem_object *obj = dma_buf->priv;
	struct drm_device *dev = obj->dev;

	if (!dev->driver->gem_prime_mmap)
		return -ENOSYS;

	return dev->driver->gem_prime_mmap(obj, vma);
}

static void *drm_gem_dmabuf_vmap(struct dma_buf *dma_buf)
{
	struct drm_gem_object *obj = dma_buf->priv;
	struct drm_device *dev = obj->dev;

	return dev->driver->gem_prime_vmap(obj);
}

static void drm_gem_dmabuf_vunmap(struct dma_buf *dma_buf, void *vaddr)
{
	struct drm_gem_object *obj = dma_buf->priv;
	struct drm_device *dev = obj->dev;

	dev->driver->gem_prime_vunmap(obj, vaddr);
}

static void *drm_gem_dmabuf_kmap(struct dma_buf *dma_buf,
		unsigned long page_num)
{
	return NULL;
}


static const struct dma_buf_ops drm_gem_prime_dmabuf_ops =  {
	.attach = drm_gem_map_attach,
	.detach = drm_gem_map_detach,
	.map_dma_buf = drm_gem_map_dma_buf,
	.unmap_dma_buf = drm_gem_unmap_dma_buf,
	.release = drm_gem_dmabuf_release,
	.kmap = drm_gem_dmabuf_kmap,
	.kmap_atomic = drm_gem_dmabuf_kmap_atomic,
	.kunmap = drm_gem_dmabuf_kunmap,
	.kunmap_atomic = drm_gem_dmabuf_kunmap_atomic,
	.mmap = drm_gem_dmabuf_mmap,
	.vmap = drm_gem_dmabuf_vmap,
	.vunmap = drm_gem_dmabuf_vunmap,
};

int drm_gem_prime_handle_to_fd(struct drm_device *dev,
		struct drm_file *file_priv, uint32_t handle, uint32_t flags,
		int *prime_fd)
{
	struct drm_gem_object *obj;
	void *buf;

	obj = drm_gem_object_lookup(dev, file_priv, handle);
	if (!obj)
		return -ENOENT;

	mutex_lock(&file_priv->prime.lock);
	/* re-export the original imported object */
	if (obj->import_attach) {
		get_dma_buf(obj->import_attach->dmabuf);
		*prime_fd = dma_buf_fd(obj->import_attach->dmabuf, flags);
		drm_gem_object_unreference_unlocked(obj);
		mutex_unlock(&file_priv->prime.lock);
		return 0;
	}

	if (obj->export_dma_buf) {
		get_dma_buf(obj->export_dma_buf);
		*prime_fd = dma_buf_fd(obj->export_dma_buf, flags);
		drm_gem_object_unreference_unlocked(obj);
	} else {
		buf = dev->driver->gem_prime_export(dev, obj, flags);
		if (IS_ERR(buf)) {
			/* normally the created dma-buf takes ownership of the ref,
			 * but if that fails then drop the ref
			 */
			drm_gem_object_unreference_unlocked(obj);
			mutex_unlock(&file_priv->prime.lock);
			return PTR_ERR(buf);
		}
		obj->export_dma_buf = buf;
		*prime_fd = dma_buf_fd(buf, flags);
	}
	mutex_unlock(&file_priv->prime.lock);
	return 0;
}
EXPORT_SYMBOL(drm_gem_prime_handle_to_fd);

int drm_gem_prime_fd_to_handle(struct drm_device *dev,
		struct drm_file *file_priv, int prime_fd, uint32_t *handle)
{
	struct dma_buf *dma_buf;
	struct drm_gem_object *obj;
	int ret;

	dma_buf = dma_buf_get(prime_fd);
	if (IS_ERR(dma_buf))
		return PTR_ERR(dma_buf);

	mutex_lock(&file_priv->prime.lock);

	ret = drm_prime_lookup_imported_buf_handle(&file_priv->prime,
			dma_buf, handle);
	if (!ret) {
		ret = 0;
		goto out_put;
	}

	/* never seen this one, need to import */
	obj = dev->driver->gem_prime_import(dev, dma_buf);
	if (IS_ERR(obj)) {
		ret = PTR_ERR(obj);
		goto out_put;
	}

	ret = drm_gem_handle_create(file_priv, obj, handle);
	drm_gem_object_unreference_unlocked(obj);
	if (ret)
		goto out_put;

	ret = drm_prime_add_imported_buf_handle(&file_priv->prime,
			dma_buf, *handle);
	if (ret)
		goto fail;

	mutex_unlock(&file_priv->prime.lock);
	return 0;

fail:
	/* hmm, if driver attached, we are relying on the free-object path
	 * to detach.. which seems ok..
	 */
	drm_gem_object_handle_unreference_unlocked(obj);
out_put:
	dma_buf_put(dma_buf);
	mutex_unlock(&file_priv->prime.lock);
	return ret;
}
EXPORT_SYMBOL(drm_gem_prime_fd_to_handle);

int drm_prime_handle_to_fd_ioctl(struct drm_device *dev, void *data,
				 struct drm_file *file_priv)
{
	struct drm_prime_handle *args = data;
	uint32_t flags;

	if (!drm_core_check_feature(dev, DRIVER_PRIME))
		return -EINVAL;

	if (!dev->driver->prime_handle_to_fd)
		return -ENOSYS;

	/* check flags are valid */
	if (args->flags & ~DRM_CLOEXEC)
		return -EINVAL;

	/* we only want to pass DRM_CLOEXEC which is == O_CLOEXEC */
	flags = args->flags & DRM_CLOEXEC;

	return dev->driver->prime_handle_to_fd(dev, file_priv,
			args->handle, flags, &args->fd);
}

int drm_prime_fd_to_handle_ioctl(struct drm_device *dev, void *data,
				 struct drm_file *file_priv)
{
	struct drm_prime_handle *args = data;

	if (!drm_core_check_feature(dev, DRIVER_PRIME))
		return -EINVAL;

	if (!dev->driver->prime_fd_to_handle)
		return -ENOSYS;

	return dev->driver->prime_fd_to_handle(dev, file_priv,
			args->fd, &args->handle);
}

/*
 * drm_prime_pages_to_sg
 *
 * this helper creates an sg table object from a set of pages
 * the driver is responsible for mapping the pages into the
 * importers address space
 */
struct sg_table *drm_prime_pages_to_sg(struct page **pages, int nr_pages)
{
	struct sg_table *sg = NULL;
	struct scatterlist *iter;
	int i;
	int ret;

	sg = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
	if (!sg)
		goto out;

	ret = sg_alloc_table(sg, nr_pages, GFP_KERNEL);
	if (ret)
		goto out;

	for_each_sg(sg->sgl, iter, nr_pages, i)
		sg_set_page(iter, pages[i], PAGE_SIZE, 0);

	return sg;
out:
	kfree(sg);
	return NULL;
}
EXPORT_SYMBOL(drm_prime_pages_to_sg);

/* helper function to cleanup a GEM/prime object */
void drm_prime_gem_destroy(struct drm_gem_object *obj, struct sg_table *sg)
{
	struct dma_buf_attachment *attach;
	struct dma_buf *dma_buf;
	attach = obj->import_attach;
	if (sg)
		dma_buf_unmap_attachment(attach, sg, DMA_BIDIRECTIONAL);
	dma_buf = attach->dmabuf;
	dma_buf_detach(attach->dmabuf, attach);
	/* remove the reference */
	dma_buf_put(dma_buf);
}
EXPORT_SYMBOL(drm_prime_gem_destroy);

void drm_prime_init_file_private(struct drm_prime_file_private *prime_fpriv)
{
	INIT_LIST_HEAD(&prime_fpriv->head);
	mutex_init(&prime_fpriv->lock);
}
EXPORT_SYMBOL(drm_prime_init_file_private);

void drm_prime_destroy_file_private(struct drm_prime_file_private *prime_fpriv)
{
	struct drm_prime_member *member, *safe;
	list_for_each_entry_safe(member, safe, &prime_fpriv->head, entry) {
		list_del(&member->entry);
		kfree(member);
	}
}
EXPORT_SYMBOL(drm_prime_destroy_file_private);

int drm_prime_add_imported_buf_handle(struct drm_prime_file_private *prime_fpriv, struct dma_buf *dma_buf, uint32_t handle)
{
	struct drm_prime_member *member;

	member = kmalloc(sizeof(*member), GFP_KERNEL);
	if (!member)
		return -ENOMEM;

	member->dma_buf = dma_buf;
	member->handle = handle;
	list_add(&member->entry, &prime_fpriv->head);
	return 0;
}
EXPORT_SYMBOL(drm_prime_add_imported_buf_handle);

int drm_prime_lookup_imported_buf_handle(struct drm_prime_file_private *prime_fpriv, struct dma_buf *dma_buf, uint32_t *handle)
{
	struct drm_prime_member *member;

	list_for_each_entry(member, &prime_fpriv->head, entry) {
		if (member->dma_buf == dma_buf) {
			*handle = member->handle;
			return 0;
		}
	}
	return -ENOENT;
}
EXPORT_SYMBOL(drm_prime_lookup_imported_buf_handle);

/* export an sg table into an array of pages and addresses
   this is currently required by the TTM driver in order to do correct fault
   handling */
int drm_prime_sg_to_page_addr_arrays(struct sg_table *sgt, struct page **pages,
				     dma_addr_t *addrs, int max_pages)
{
	unsigned count;
	struct scatterlist *sg;
	struct page *page;
	u32 len, offset;
	int pg_index;
	dma_addr_t addr;

	pg_index = 0;
	for_each_sg(sgt->sgl, sg, sgt->nents, count) {
		len = sg->length;
		offset = sg->offset;
		page = sg_page(sg);
		addr = sg_dma_address(sg);

		while (len > 0) {
			if (WARN_ON(pg_index >= max_pages))
				return -1;
			pages[pg_index] = page;
			if (addrs)
				addrs[pg_index] = addr;

			page++;
			addr += PAGE_SIZE;
			len -= PAGE_SIZE;
			pg_index++;
		}
	}
	return 0;
}
EXPORT_SYMBOL(drm_prime_sg_to_page_addr_arrays);

/**
 * drm_gem_prime_export - helper library implementation of the export callback
 * @dev: drm_device to export from
 * @obj: GEM object to export
 * @flags: flags like DRM_CLOEXEC
 *
 * This is the implementation of the gem_prime_export functions for GEM drivers
 * using the PRIME helpers.
 */
struct dma_buf *drm_gem_prime_export(struct drm_device *dev,
				     struct drm_gem_object *obj, int flags)
{
	return dma_buf_export(obj, &drm_gem_prime_dmabuf_ops, obj->size, flags);
}
EXPORT_SYMBOL(drm_gem_prime_export);

/**
 * drm_gem_prime_import - helper library implementation of the import callback
 * @dev: drm_device to import into
 * @dma_buf: dma-buf object to import
 *
 * This is the implementation of the gem_prime_import functions for GEM drivers
 * using the PRIME helpers.
 */
struct drm_gem_object *drm_gem_prime_import(struct drm_device *dev,
					    struct dma_buf *dma_buf)
{
	struct dma_buf_attachment *attach;
	struct sg_table *sgt;
	struct drm_gem_object *obj;
	int ret;

	if (!dev->driver->gem_prime_import_sg_table)
		return ERR_PTR(-EINVAL);

	if (dma_buf->ops == &drm_gem_prime_dmabuf_ops) {
		obj = dma_buf->priv;
		if (obj->dev == dev) {
			/*
			 * Importing dmabuf exported from out own gem increases
			 * refcount on gem itself instead of f_count of dmabuf.
			 */
			drm_gem_object_reference(obj);
			return obj;
		}
	}

	attach = dma_buf_attach(dma_buf, dev->dev);
	if (IS_ERR(attach))
		return ERR_CAST(attach);

	get_dma_buf(dma_buf);

	sgt = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR_OR_NULL(sgt)) {
		ret = PTR_ERR(sgt);
		goto fail_detach;
	}

	obj = dev->driver->gem_prime_import_sg_table(dev, dma_buf->size, sgt);
	if (IS_ERR(obj)) {
		ret = PTR_ERR(obj);
		goto fail_unmap;
	}

	obj->import_attach = attach;

	return obj;

fail_unmap:
	dma_buf_unmap_attachment(attach, sgt, DMA_BIDIRECTIONAL);
fail_detach:
	dma_buf_detach(dma_buf, attach);
	dma_buf_put(dma_buf);

	return ERR_PTR(ret);
}
EXPORT_SYMBOL(drm_gem_prime_import);


void drm_prime_remove_imported_buf_handle(struct drm_prime_file_private *prime_fpriv, struct dma_buf *dma_buf)
{
	struct drm_prime_member *member, *safe;

	mutex_lock(&prime_fpriv->lock);
	list_for_each_entry_safe(member, safe, &prime_fpriv->head, entry) {
		if (member->dma_buf == dma_buf) {
			list_del(&member->entry);
			kfree(member);
		}
	}
	mutex_unlock(&prime_fpriv->lock);
}
EXPORT_SYMBOL(drm_prime_remove_imported_buf_handle);
