DESCRIPTION = "Tests application"
SECTION = "bt tools"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
  file://BTPMSRVR.c \
  file://BTPMSRVR.h \
  file://SS1BTPM.h \
  file://SS1BTPS.h \
  file://BTAPITyp.h \
  file://BTTypes.h \
  file://BaseTypes.h \
  file://BTBTypes.h \
  file://BSCAPI.h \
  file://BTPSKRNL.h \
  file://BKRNLAPI.h \
  file://GAPAPI.h \
  file://HCITypes.h \
  file://HCICommT.h \
  file://L2CAPAPI.h \
  file://L2CAPTyp.h \
  file://SCOAPI.h \
  file://RFCOMAPI.h \
  file://RFCOMMT.h \
  file://GOEPAPI.h \
  file://OTPAPI.h \
  file://SDPAPI.h \
  file://SDPTypes.h \
  file://SMTypes.h \
  file://SPPAPI.h \
  file://OBXTypes.h \
  file://HCIUSBT.h \
  file://BTPSCFG.h \
  file://BTPMCFG.h \
  file://BTPMERR.h \
  file://BTPMVER.h \
  file://DBGAPI.h \
  file://BTPMDBGZ.h \
  file://MSGAPI.h \
  file://BTPMMSGT.h \
  file://DEVMMSG.h \
  file://DEVMType.h \
  file://SS1BTGAT.h \
  file://ATTTypes.h \
  file://GATTAPI.h \
  file://GATTType.h \
  file://SETAPI.h \
  file://SPPMMSG.h \
  file://SPPMType.h \
  file://GATMMSG.h \
  file://GATMType.h \
  file://BTPMVSAPI.h \
  file://SCOMMSG.h \
  file://server/BTPMDEVM.h \
  file://server/BTPMGATM.h \
  file://server/BTPMMAPI.h \
  file://server/BTPMSCOM.h \
  file://server/BTPMSPPM.h \
  file://server/DEVMAPI.h \
  file://server/DEVMDISC.h \
  file://server/DEVMGR.h \
  file://server/DEVMGRI.h \
  file://server/GATMAPI.h \
  file://server/GATMGR.h \
  file://server/SCOMAPI.h \
  file://server/SCOMGR.h \
  file://server/SPPMAPI.h \
  file://server/SPPMGR.h \
  file://TMRAPI.h \
  file://HCIAPI.h \
  file://BTErrors.h \
  file://BTPSVER.h \
  file://LinuxHDSM_HS.c \
  file://LinuxHDSM_HS.h \
  file://LinuxHDSM_AG.c \
  file://LinuxHDSM_AG.h \
  file://LinuxHFRM_HF.c \
  file://LinuxHFRM_HF.h \
  file://SS1BTHDSM.h \
  file://client/BTPMDEVM.h \
  file://client/BTPMGATM.h \
  file://client/BTPMMAPI.h \
  file://client/BTPMSCOM.h \
  file://client/BTPMSPPM.h \
  file://client/DEVMAPI.h \
  file://client/DEVMGR.h \
  file://client/GATMAPI.h \
  file://client/GATMGR.h \
  file://client/SCOMAPI.h \
  file://client/SCOMGR.h \
  file://client/SPPMAPI.h \
  file://client/SPPMGR.h \
  file://client/HDSMAPI.h \
  file://HDSMMSG.h \
  file://HDSMType.h \
  file://SS1BTHDS.h \
  file://HDSETAPI.h \
  file://SS1BTHFRM.h \
  file://HFRMMSG.h \
  file://SS1BTHFR.h \
  file://HFREAPI.h \
  file://HFRMType.h \
  file://client/HFRMAPI.h \
  file://libBTPM_S.a \
  file://libBTPM_HAL_S.a \
  file://libSS1BTAUD.a \
  file://libSS1BTGAV.a \
  file://libSS1BTAVC.a \
  file://libSS1BTAVR.a \
  file://libSS1SBC.a \
  file://libSS1BTFTP.a \
  file://libSS1BTHDP.a \
  file://libSS1BTHDS.a \
  file://libSS1BTHIDH.a \
  file://libSS1BTHID.a \
  file://libSS1BTPAN.a \
  file://libBTPS_VNET.a \
  file://libSS1BTPBA.a \
  file://libSS1BTHFR.a \
  file://libSS1BTMAP.a \
  file://libSS1BTOPP.a \
  file://libSS1BTCTS.a \
  file://libSS1BTNDCS.a \
  file://libSS1BTRTUS.a \
  file://libSS1BTDIS.a \
  file://libSS1BTGAT.a \
  file://libSS1BTGAPS.a \
  file://libSS1BTCSCS.a \
  file://libSS1BTHIDS.a \
  file://libSS1BTNDC.a \
  file://libSS1BTRTU.a \
  file://libSS1BTIAS.a \
  file://libSS1BTHRS.a \
  file://libSS1BTSCP.a \
  file://libSS1BTLLS.a \
  file://libSS1BTBAS.a \
  file://libSS1BTTPS.a \
  file://libBTPS.a \
  file://libBTPS_PLA.a \
  file://libBTPS_OSA.a \
  file://libBTPM_HAL_C.a \
  file://libBTPM_C.a \
"

S = "${WORKDIR}"

do_compile() {

	${CC} BTPMSRVR.c -I./ -Iserver -lpthread -lm \
	                 libBTPM_S.a libBTPM_HAL_S.a libSS1BTAUD.a libSS1BTGAV.a \
	                 libSS1BTAVC.a libSS1BTAVR.a libSS1SBC.a libSS1BTFTP.a \
	                 libSS1BTHDP.a libSS1BTHDS.a libSS1BTHIDH.a libSS1BTHID.a \
	                 libSS1BTPAN.a libBTPS_VNET.a libSS1BTPBA.a libSS1BTHFR.a \
	                 libSS1BTMAP.a libSS1BTOPP.a libSS1BTCTS.a libSS1BTNDCS.a \
	                 libSS1BTRTUS.a libSS1BTDIS.a libSS1BTGAT.a libSS1BTGAPS.a \
	                 libSS1BTCSCS.a libSS1BTHIDS.a libSS1BTNDC.a libSS1BTRTU.a \
	                 libSS1BTIAS.a libSS1BTHRS.a libSS1BTSCP.a libSS1BTLLS.a \
	                 libSS1BTBAS.a libSS1BTTPS.a libBTPS.a libBTPS_PLA.a \
	                 libBTPS_OSA.a \
	      -o SS1BTPM

	${CC} LinuxHDSM_AG.c -I./ -Iclient -lpthread -lm \
	                     libBTPM_C.a libBTPM_HAL_C.a \
	      -o LinuxHDSM_AG

	${CC} LinuxHDSM_HS.c -I./ -Iclient -lpthread -lm \
	                     libBTPM_C.a libBTPM_HAL_C.a \
	      -o LinuxHDSM_HS

	${CC} LinuxHFRM_HF.c -I./ -Iclient -lpthread -lm \
	                     libBTPM_C.a libBTPM_HAL_C.a \
	      -o LinuxHFRM_HF


}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 SS1BTPM ${D}${bindir}
	install -m 0755 LinuxHDSM_AG ${D}${bindir}
	install -m 0755 LinuxHDSM_HS ${D}${bindir}
	install -m 0755 LinuxHFRM_HF ${D}${bindir}
}
