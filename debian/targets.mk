# Target SoCs supported by camera-engine-rkaiq
# debian/rules includes this Makefile snippet.

rkaiq_platforms += rk3588
rk3588_config := -DRKAIQ_TARGET_SOC=rk3588 \
		 -DISP_HW_VERSION=-DISP_HW_V30 \
		 -DRKAIQ_ENABLE_CAMGROUP=TRUE

rkaiq_platforms += rk3568
rk3568_config := -DRKAIQ_TARGET_SOC=rk356x \
		 -DISP_HW_VERSION=-DISP_HW_V21
