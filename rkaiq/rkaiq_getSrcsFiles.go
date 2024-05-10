package rkaiqdefaults

import (
    // "fmt"
)

func rkaiq_getSrcsFiles(soc string, macros_map map[string]bool) []string {

    var flag0 bool = false;

    var srcs []string;
    var hdl_srcs []string;

    flag0 = macros_map["ISP_HW_V39"]
    if flag0 {
        srcs = append(srcs, "hwi/isp39/CamHwIsp39.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_FAKECAM"]
    if (flag0) {
        srcs = append(srcs, "hwi/fakecamera/FakeCamHwIsp20.cpp")
        srcs = append(srcs, "hwi/fakecamera/FakeSensorHw.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_SPSTREAM"]
    if (flag0) {
        srcs = append(srcs, "hwi/isp20/SPStreamProcUnit.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_MULTIISP"]
    if (flag0) {
        srcs = append(srcs, "hwi/IspParamsSplitter.cpp")
    }

    flag0 = macros_map["RKAIQ_ENABLE_CAMGROUP"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_camgroup_handlers/*.cpp")
        srcs = append(srcs, "RkAiqCamGroupManager.cpp")
        srcs = append(srcs, "aiq_core/RkAiqCamgroupHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_ENABLE_PARSER_V1"]
    if flag0 {
       //IQ_PARSER_SRC
        srcs = append(srcs, "iq_parser/RkAiqCalibParser.cpp")
        srcs = append(srcs, "iq_parser/RkAiqCalibDb.cpp")
        srcs = append(srcs, "iq_parser/RkAiqCalibTag.cpp")
        srcs = append(srcs, "iq_parser/tinyxml2.cpp")
        srcs = append(srcs, "iq_parser/xmltags.cpp")
        srcs = append(srcs, "iq_parser/RkAiqCalibApi.cpp")
       //IQ_PARSER_V2_SRC
        srcs = append(srcs, "iq_parser_v2/awb_xml2json.cpp")
        srcs = append(srcs, "iq_parser_v2/bayernr_xml2json_v1.cpp")
        srcs = append(srcs, "iq_parser_v2/mfnr_xml2json_v1.cpp")
        srcs = append(srcs, "iq_parser_v2/ynr_xml2json_v1.cpp")
        srcs = append(srcs, "iq_parser_v2/uvnr_xml2json_v1.cpp")
        srcs = append(srcs, "iq_parser_v2/sharp_xml2json_v1.cpp")
        srcs = append(srcs, "iq_parser_v2/edgefilter_xml2json_v1.cpp")
    }
    if soc == "rv1109" {
        srcs = append(srcs, "hwi/isp20/NrStatsStream.cpp")
        srcs = append(srcs, "hwi/isp20/TnrStatsStream.cpp")
        srcs = append(srcs, "hwi/isp20/NrStreamProcUnit.cpp")
        srcs = append(srcs, "hwi/isp20/FecParamStream.cpp")
        srcs = append(srcs, "hwi/isp20/TnrStreamProcUnit.cpp")
    }

    flag0 = macros_map["RKAIQ_NEWSTRUCT_TEST"]
    if flag0 {
        srcs = append(srcs, "modules/rk_aiq_isp32_modules.c")
        flag1 = macros_map["ISP_HW_V39"]
        if flag1 {
            srcs = append(srcs, "modules/rk_aiq_isp39_modules.c")
        }
        flag2 = macros_map["ISP_HW_V33"]
        if flag2 {
            srcs = append(srcs, "modules/rk_aiq_isp33_modules.c")
        }
        srcs = append(srcs, "algos/newStruct/algo_common.c")
    }

    hdl_srcs = rkaiq_getAlgoHandleSrcs(soc, macros_map);

    srcs = append(srcs, hdl_srcs...);

    // for i, v := range srcs {
    //     fmt.Printf("%d %s\n", i, v)
    // }
    // fmt.Printf("macros_map size: %d\n", len(srcs))
    return srcs;
}

func rkaiq_getAlgoHandleSrcs(soc string, macros_map map[string]bool) []string {

    var flag0 bool = false;
    var flag1 bool = false;
    var flag2 bool = false;
    var flag3 bool = false;
    var flag4 bool = false;
    var flag5 bool = false;
    var flag6 bool = false;
    var flag7 bool = false;
    var flag8 bool = false;
    var flag9 bool = false;

    var srcs []string

    useNewstruct := macros_map["RKAIQ_NEWSTRUCT_TEST"]

    srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAwbHandle.cpp")

    flag0 = macros_map["RKAIQ_HAVE_AWB_V21"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAwbV21Handle.cpp")
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqCustomAwbHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_AWB_V32"]
    flag1 = macros_map["RKAIQ_HAVE_AWB_V32LT"]
    if (flag0 || flag1) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAwbV32Handle.cpp")
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqCustomAwbHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_AWB_V39"]
    if flag0 {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAwbV39Handle.cpp")
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqCustomAwbHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_GIC_V1"]
    flag1 = macros_map["RKAIQ_HAVE_GIC_V2"]
    if (flag0 || flag1) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqGicHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAgicHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_ORB_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAorbHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_EIS_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAeisHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_CAC_V03"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcacV3Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_CAC_V10"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcacHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_CAC_V11"]
    flag1 = macros_map["RKAIQ_HAVE_CAC_V12"]
    flag2 = macros_map["RKAIQ_HAVE_CAC_V30"]
    if (flag0 || flag1 || flag2) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqCacHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcacV11Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_YNR_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAynrHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_YNR_V2"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAynrV2Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_YNR_V3"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAynrV3Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_YNR_V22"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqYnrHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAynrV22Handle.cpp")
        }
    }
    
    flag0 = macros_map["RKAIQ_HAVE_YNR_V24"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqYnrHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAynrV24Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_UVNR_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcnrHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_CNR_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcnrV1Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_CNR_V2"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcnrV2Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_CNR_V30"]
    flag1 = macros_map["RKAIQ_HAVE_CNR_V30_LITE"]
    if (flag0 || flag1) {
        if flag0 && useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqCnrHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcnrV30Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_CNR_V31"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqCnrHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcnrV31Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_SHARP_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAsharpHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_SHARP_V3"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAsharpV3Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_SHARP_V4"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAsharpV4Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_SHARP_V33"]
    flag1 = macros_map["RKAIQ_HAVE_SHARP_V33_LITE"]
    if (flag0 || flag1) {
        if flag0 && useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqSharpHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAsharpV33Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_SHARP_V34"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqSharpHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAsharpV34Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_MFNR_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAmfnrHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_BAYERNR_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqArawnrHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_BAYERNR_V2"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqArawnrV2Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_BAYER2DNR_V2"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAbayer2dnrV2Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_BAYER2DNR_V23"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAbayer2dnrV23Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_BAYERTNR_V2"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAbayertnrV2Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_BAYERTNR_V23"]
    flag1 = macros_map["RKAIQ_HAVE_BAYERTNR_V23_LITE"]
    if (flag0 || flag1) {
        if flag0 && useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqBtnrHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAbayertnrV23Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_BAYERTNR_V30"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqBtnrHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAbayertnrV30Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_GAIN_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAgainHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_GAIN_V2"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqGainHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAgainV2Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_BLC_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAblcHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_BLC_V32"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqBlcHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAblcV32Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_ANR_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAnrHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_AMD_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAmdHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_3DLUT_V1"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiq3dlutHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqA3dlutHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_CCM_V1"]
    flag1 = macros_map["RKAIQ_HAVE_CCM_V2"]
    flag2 = macros_map["RKAIQ_HAVE_CCM_V3"]
    if (flag0 || flag1 || flag2) {
        if flag2 && useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqCcmHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAccmHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_GAMMA_V11"]
    flag1 = macros_map["RKAIQ_HAVE_GAMMA_V10"]
    if (flag0 || flag1) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqGammaHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAgammaHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_MERGE_V10"]
    flag1 = macros_map["RKAIQ_HAVE_MERGE_V11"]
    flag2 = macros_map["RKAIQ_HAVE_MERGE_V12"]
    if (flag0 || flag1 || flag2) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqMergeHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAmergeHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_DRC_V10"]
    flag1 = macros_map["RKAIQ_HAVE_DRC_V11"]
    flag2 = macros_map["RKAIQ_HAVE_DRC_V12"]
    flag3 = macros_map["RKAIQ_HAVE_DRC_V12_LITE"]
    flag4 = macros_map["RKAIQ_HAVE_DRC_V20"]
    if (flag0 || flag1 || flag2 || flag3 || flag4) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqDrcHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAdrcHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_DEHAZE_V10"]
    flag1 = macros_map["RKAIQ_HAVE_DEHAZE_V11"]
    flag2 = macros_map["RKAIQ_HAVE_DEHAZE_V11_DUO"]
    flag3 = macros_map["RKAIQ_HAVE_DEHAZE_V12"]
    flag4 = macros_map["RKAIQ_HAVE_DEHAZE_V14"]
    if (flag0 || flag1 || flag2 || flag3 || flag4) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqDehazeHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAdehazeHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_RGBIR_REMOSAIC_V10"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqRgbirHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqArgbirHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_TMO_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAtmoHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_WDR_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAwdrHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_LDCH_V21"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqLdchHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAldchHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_LDCH_V10"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAldchHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_FEC_V10"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAfecHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_CSM_V1"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqCsmHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcsmHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_CGC_V1"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqCgcHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcgcHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_AE_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAeHandle.cpp")
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqCustomAeHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_AFD_V1"]
    flag1 = macros_map["RKAIQ_HAVE_AFD_V2"]
    if (flag0 || flag1) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAfdHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_DEBAYER_V1"]
    flag1 = macros_map["RKAIQ_HAVE_DEBAYER_V2"]
    flag2 = macros_map["RKAIQ_HAVE_DEBAYER_V2_LITE"]
    flag3 = macros_map["RKAIQ_HAVE_DEBAYER_V3"]
    flag4 = macros_map["RKAIQ_HAVE_DEBAYER_V4"]
    if (flag0 || flag1 || flag2 || flag3 || flag4) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqDmHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAdebayerHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_LSC_V1"]
    flag1 = macros_map["RKAIQ_HAVE_LSC_V2"]
    flag2 = macros_map["RKAIQ_HAVE_LSC_V3"]
    if (flag0 || flag1 || flag2) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqLscHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAlscHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_DPCC_V1"]
    flag1 = macros_map["RKAIQ_HAVE_DPCC_V2"]
    if (flag0 || flag1) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqDpccHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAdpccHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_ACP_V10"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqCpHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcpHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_AIE_V10"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqIeHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAieHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_ASD_V10"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAsdHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_YUVME_V1"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqYmeHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAyuvmeV1Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_DEGAMMA_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAdegammaHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_AF_V20"]
    flag1 = macros_map["RKAIQ_HAVE_AF_V30"]
    flag2 = macros_map["RKAIQ_HAVE_AF_V31"]
    flag3 = macros_map["RKAIQ_HAVE_AF_V32_LITE"]
    flag4 = macros_map["RKAIQ_ONLY_AF_STATS_V20"]
    flag5 = macros_map["RKAIQ_ONLY_AF_STATS_V30"]
    flag6 = macros_map["RKAIQ_ONLY_AF_STATS_V31"]
    flag7 = macros_map["RKAIQ_ONLY_AF_STATS_V32_LITE"]
    flag8 = macros_map["RKAIQ_HAVE_AF_V33"]
    flag9 = macros_map["RKAIQ_ONLY_AF_STATS_V33"]
    if (flag0 || flag1 || flag2 || flag3 || flag4 || flag5 || flag6 || flag7 || flag8 || flag9) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAfHandle.cpp")
    }
    return srcs
}
