rm -rf frameworks/base
rm -rf frameworks/av
rm -rf vendor/derp
rm -rf packages/apps/Settings
git clone --depth=1 https://github.com/k4ngcaribug/derp_frameworks_base frameworks/base
git clone --depth=1 https://github.com/k4ngcaribug/derp_frameworks_av frameworks/av
git clone --depth=1 https://github.com/k4ngcaribug/vendor_derp vendor/derp
git clone --depth=1 https://github.com/k4ngcaribug/derp_packages_apps_Settings packages/apps/Settings
