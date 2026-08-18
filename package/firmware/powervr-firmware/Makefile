#
# Copyright (C) 2026 OpenWrt
#
# This is free software, licensed under the GNU General Public License v2.

include $(TOPDIR)/rules.mk

PKG_NAME:=powervr-firmware
PKG_VERSION:=1.1.OS@6976702
PKG_RELEASE:=1

PKG_LICENSE:=proprietary
PKG_LICENSE_FILES:=LICENSE.powervr

PKG_MAINTAINER:=OpenWrt sunxi AIoT

include $(INCLUDE_DIR)/package.mk

define Package/powervr-firmware
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=Imagination PowerVR BXM-4-64 firmware (A733)
  DEPENDS:=@TARGET_allwinner
endef

define Package/powervr-firmware/description
  Firmware for the Allwinner A733 PowerVR BXM-4-64 GPU
  (BVNC 36.56.104.183). Required by kmod-drm-powervr.

  Source: Imagination linux-firmware (gitlab.freedesktop.org/imagination/linux-firmware,
  branch powervr, rogue_36.56.104.183_v1.fw). Redistributable; see LICENSE.powervr.
endef

define Build/Compile
	true
endef

define Package/powervr-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/powervr
	$(INSTALL_DATA) ./firmware/rogue_36.56.104.183_v1.fw \
		$(1)/lib/firmware/powervr/
	$(INSTALL_DIR) $(1)/usr/share/licenses/powervr-firmware
	$(INSTALL_DATA) ./LICENSE.powervr $(1)/usr/share/licenses/powervr-firmware/
endef

$(eval $(call BuildPackage,powervr-firmware))
