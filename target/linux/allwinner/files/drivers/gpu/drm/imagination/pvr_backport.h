/* SPDX-License-Identifier: GPL-2.0-only OR MIT */
/*
 * Linux 6.6 shims for the 6.18+/drm-misc PowerVR driver and bundled drm_gpuvm.
 *
 * This header is force-included via ccflags-y -include.
 */

#ifndef _PVR_BACKPORT_H_
#define _PVR_BACKPORT_H_

#include <linux/version.h>
#include <linux/slab.h>
#include <linux/xarray.h>
#include <linux/dma-fence.h>

#include <drm/drm_exec.h>
#include <drm/gpu_scheduler.h>

#ifndef kzalloc_obj
#define kzalloc_obj(obj) kzalloc(sizeof(obj), GFP_KERNEL)
#endif

#ifndef kzalloc_objs
#define kzalloc_objs(obj, n) kcalloc((n), sizeof(obj), GFP_KERNEL)
#endif

/* 6.6 shmem helpers are unlocked; 6.8+ renamed them *_locked. */
#define drm_gem_shmem_vmap_locked drm_gem_shmem_vmap
#define drm_gem_shmem_vunmap_locked drm_gem_shmem_vunmap

/*
 * drm_exec_init() gained a prealloc count around 6.11. 6.6 only takes flags.
 * Callers in this backport always pass three arguments.
 */
static inline void pvr_drm_exec_init(struct drm_exec *exec, u32 flags, unsigned int nr)
{
	(void)nr;
	(drm_exec_init)(exec, flags);
}

#define drm_exec_init(exec, flags, nr) pvr_drm_exec_init(exec, flags, nr)

/*
 * drm_exec_for_each_locked_object() dropped the index argument after 6.6.
 * Hide the 6.6 index inside a unique loop variable.
 */
#undef drm_exec_for_each_locked_object
#define drm_exec_for_each_locked_object(exec, obj) \
	for (unsigned long pvr_exec_i__ = 0; \
	     ((obj) = drm_exec_obj((exec), pvr_exec_i__)); ++pvr_exec_i__)

/*
 * 6.15+ drm_sched_init() takes struct drm_sched_init_args. Reconstruct that
 * helper on top of the 6.6 argument list.
 */
struct drm_sched_init_args {
	const struct drm_sched_backend_ops *ops;
	struct workqueue_struct *submit_wq;
	u32 num_rqs;
	u32 credit_limit;
	unsigned int hang_limit;
	long timeout;
	struct workqueue_struct *timeout_wq;
	const char *name;
	struct device *dev;
};

static inline int pvr_drm_sched_init(struct drm_gpu_scheduler *sched,
				     const struct drm_sched_init_args *args)
{
	return (drm_sched_init)(sched, args->ops, args->credit_limit,
				args->hang_limit, args->timeout, args->timeout_wq,
				NULL, args->name, args->dev);
}

#define drm_sched_init(sched, args) pvr_drm_sched_init(sched, args)

/*
 * 6.6: drm_sched_job_init(job, entity, owner)
 * 6.15+: drm_sched_job_init(job, entity, credits, owner, drm_client_id)
 */
static inline int pvr_drm_sched_job_init(struct drm_sched_job *job,
					 struct drm_sched_entity *entity,
					 u32 credits, void *owner,
					 u64 drm_client_id)
{
	(void)credits;
	(void)drm_client_id;
	return (drm_sched_job_init)(job, entity, owner);
}

#define drm_sched_job_init(job, entity, credits, owner, client_id) \
	pvr_drm_sched_job_init(job, entity, credits, owner, client_id)

#ifndef drm_sched_job_has_dependency
static inline bool drm_sched_job_has_dependency(struct drm_sched_job *job,
						struct dma_fence *fence)
{
	struct dma_fence *f;
	unsigned long index;

	xa_for_each(&job->dependencies, index, f) {
		if (f == fence)
			return true;
	}

	return false;
}
#endif

#endif /* _PVR_BACKPORT_H_ */
