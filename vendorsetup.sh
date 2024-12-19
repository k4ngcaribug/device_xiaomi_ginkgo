git clone --depth=1 https://github.com/ArrowOS-Devices/android_vendor_miuicamera -b arrow-12.0-a3 vendor/miuicamera
rm -rf frameworks/base
rm -rf frameworks/av
rm -rf vendor/derp
git clone --depth=1 https://github.com/k4ngcaribug/derp_frameworks_base frameworks/base
git clone --depth=1 https://github.com/k4ngcaribug/derp_frameworks_av frameworks/av
git clone --depth=1 https://github.com/k4ngcaribug/vendor_derp vendor/derp
