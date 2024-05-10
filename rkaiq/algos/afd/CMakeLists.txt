set(AFD_SRC
    rk_aiq_algo_afd_itf.c
    rk_aiq_uapi_afd_int.c
    )
if (RKAIQ_HAVE_AFD_V1)
    list(APPEND AFD_SRC rk_aiq_afd_algo.c)
endif()

if (RKAIQ_HAVE_AFD_V2)
    list(APPEND AFD_SRC rk_aiq_afd_algo.c)
endif()

dec_algo(afd rk_aiq_algo_afd_itf.c ${AFD_SRC})
