// SPDX-License-Identifier: GPL-2.0-only
/*
 * Optional Allwinner A733 GPU/NPU clock helper.
 *
 * Uses clk + OPP APIs only. Default module parameters apply no change until
 * npu_mhz / gpu_mhz are written (or the init script loads the module with
 * values). Rates above the compiled-in caps are rejected.
 */

#include <linux/clk.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/pm_opp.h>

#ifndef SUNXI_OVERCLOCK_NPU_MAX_KHZ
#define SUNXI_OVERCLOCK_NPU_MAX_KHZ 1120000
#endif

#ifndef SUNXI_OVERCLOCK_GPU_MAX_KHZ
#define SUNXI_OVERCLOCK_GPU_MAX_KHZ 1008000
#endif

#ifdef SUNXI_OVERCLOCK_ALLOW_EXTREME
#define NPU_CAP_KHZ 2520000
#define GPU_CAP_KHZ 1488000
#else
#define NPU_CAP_KHZ SUNXI_OVERCLOCK_NPU_MAX_KHZ
#define GPU_CAP_KHZ SUNXI_OVERCLOCK_GPU_MAX_KHZ
#endif

static unsigned int npu_mhz;
static unsigned int gpu_mhz;
static char *profile = "stock";

module_param(npu_mhz, uint, 0644);
MODULE_PARM_DESC(npu_mhz, "Target NPU frequency in MHz (0 = leave unchanged)");
module_param(gpu_mhz, uint, 0644);
MODULE_PARM_DESC(gpu_mhz, "Target GPU frequency in MHz (0 = leave unchanged)");
module_param(profile, charp, 0644);
MODULE_PARM_DESC(profile, "Named profile: stock, npu1120, or custom");

static int apply_rate(struct device *dev, const char *clk_name,
		      unsigned long hz, unsigned long cap_hz)
{
	struct clk *clk;
	int err;

	if (!hz)
		return 0;
	if (hz > cap_hz) {
		dev_err(dev, "refusing %s rate %lu Hz (cap %lu Hz)\n",
			clk_name, hz, cap_hz);
		return -EINVAL;
	}

	clk = clk_get(dev, clk_name);
	if (IS_ERR(clk))
		return PTR_ERR(clk);

	err = dev_pm_opp_set_rate(dev, hz);
	if (err && err != -ENODEV && err != -ENOTSUPP) {
		dev_warn(dev, "OPP set_rate(%lu) failed (%d), trying clk_set_rate\n",
			 hz, err);
		err = clk_set_rate(clk, hz);
	} else if (err) {
		err = clk_set_rate(clk, hz);
	}

	if (!err)
		dev_info(dev, "%s now %lu Hz (requested %lu)\n",
			 clk_name, clk_get_rate(clk), hz);

	clk_put(clk);
	return err;
}

static struct device *dev_from_compat(const char *compat)
{
	struct device_node *np;
	struct platform_device *pdev;

	np = of_find_compatible_node(NULL, NULL, compat);
	if (!np)
		return NULL;
	pdev = of_find_device_by_node(np);
	of_node_put(np);
	return pdev ? &pdev->dev : NULL;
}

static int apply_profile(void)
{
	if (!strcmp(profile, "stock")) {
		npu_mhz = 0;
		gpu_mhz = 0;
	} else if (!strcmp(profile, "npu1120")) {
		npu_mhz = 1120;
	} else if (strcmp(profile, "custom") && profile[0]) {
		pr_err("sunxi-overclock: unknown profile '%s'\n", profile);
		return -EINVAL;
	}
	return 0;
}

static int __init sunxi_overclock_init(void)
{
	struct device *npu, *gpu;
	int err;

	err = apply_profile();
	if (err)
		return err;

	if (!npu_mhz && !gpu_mhz) {
		pr_info("sunxi-overclock: loaded, no rate changes (profile=%s)\n",
			profile);
		return 0;
	}

	if (npu_mhz) {
		npu = dev_from_compat("allwinner,npu");
		if (!npu) {
			pr_err("sunxi-overclock: NPU device not found\n");
			return -ENODEV;
		}
		err = apply_rate(npu, "clk_npu",
				 (unsigned long)npu_mhz * 1000000UL,
				 (unsigned long)NPU_CAP_KHZ * 1000UL);
		put_device(npu);
		if (err)
			return err;
	}

	if (gpu_mhz) {
		gpu = dev_from_compat("img,img-rogue");
		if (!gpu)
			gpu = dev_from_compat("img,gpu");
		if (!gpu) {
			pr_err("sunxi-overclock: GPU device not found\n");
			return -ENODEV;
		}
		err = apply_rate(gpu, "core",
				 (unsigned long)gpu_mhz * 1000000UL,
				 (unsigned long)GPU_CAP_KHZ * 1000UL);
		put_device(gpu);
		if (err)
			return err;
	}

	return 0;
}

static void __exit sunxi_overclock_exit(void)
{
}

module_init(sunxi_overclock_init);
module_exit(sunxi_overclock_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OpenWrt sunxi AIoT");
MODULE_DESCRIPTION("Optional Allwinner A733 GPU/NPU clock helper");
