#
# Copyright (C) 2020 ArrowOS
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Inherit framework first
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/languages_full.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/product_launched_with_p.mk)

# Inherit from ginkgo device
$(call inherit-product, device/xiaomi/ginkgo/device.mk)

# Inherit some common Afterlife stuff
$(call inherit-product, vendor/afterlife/config/common_full_phone.mk)

# Afterlife Flags
TARGET_FACE_UNLOCK_SUPPORTED := true
TARGET_SUPPORTS_QUICK_TAP := false
TARGET_BOOT_ANIMATION_RES := 1080
AFTERLIFE_GAPPS := true
AFTERLIFE_CORE := true
AFTERLIFE_MAINTAINER := AL
USE_PIXEL_CHARGING := true
TARGET_SUPPORTS_TOUCHGESTURES := true
AFTERLIFE_EXTRA := true
BUILD_GMAIL := true
BUILD_GALLERYGO := true

# Device identifier
PRODUCT_NAME := afterlife_ginkgo
PRODUCT_DEVICE := ginkgo
PRODUCT_BRAND := Xiaomi
PRODUCT_MODEL := Redmi Note 8/8T
PRODUCT_MANUFACTURER := Xiaomi
PRODUCT_GMS_CLIENTID_BASE := android-xiaomi