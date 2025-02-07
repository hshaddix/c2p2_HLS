# name of .xlc file
HLS_NAME := StripClustering
# xilinx part to be used
PART := xilinx_u250_gen3x16_xdma_4_1_202210_1
# emulation mode (sw_emu, hw_emu, hw)
EMU_MODE := sw

# Define SW_EMU macro when compiling for software emulation
CFLAGS := 
ifeq ($(EMU_MODE), sw_emu)
    CFLAGS += -DSW_EMU
endif

########
main: ${HLS_NAME}.xclbin host export.sh

.PHONY: host compile link clean all

all: host compile link

# Makefile to build the Vitis kernel and host application
CXX := g++
CXXFLAGS := -Wall -O0 -g -std=c++14 -I${XILINX_XRT}/include/ -I${XILINX_HLS}/include/ -I../kernel/ -I.
LDFLAGS := -L/opt/xilinx/xrt/lib -lOpenCL -lpthread -lrt -lstdc++ -L${XILINX_XRT}/lib -lxilinxopencl -lxrt_coreutil

# # Directories
KERNEL_DIR := $(realpath ../kernel)
SRC_DIR := $(realpath ./)

# # Files
HOST_SRC := $(SRC_DIR)/host.cpp $(SRC_DIR)/xcl2.cpp
CONFIG_FILE := $(SRC_DIR)/config.cfg

KERNEL_SRC := ${KERNEL_DIR}/processHits.cpp
KERNEL_INCLUDE := -I${KERNEL_DIR}
KERNEL_NAME := processHits

########
## Host
host: $(HOST_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) -lpthread

#########
## Compile the top level kernel
${KERNEL_NAME}.xo: ${KERNEL_SRC}
	mkdir -p ${SRC_DIR}/kernels
	cd ${SRC_DIR}/kernels; \
	v++ -c -t ${EMU_MODE} --platform ${PART} -k ${KERNEL_NAME} ${CFLAGS} ${KERNEL_INCLUDE} ${KERNEL_SRC} -o ${KERNEL_NAME}.xo && \
	mv ${SRC_DIR}/kernels/${KERNEL_NAME}.xo ${SRC_DIR}/

###############
## Compile the kernel into xclbin
${HLS_NAME}.xclbin: $(KERNEL_NAME).xo emconfig.json ${CONFIG_FILE}
	mkdir -p ${SRC_DIR}/xclbin
	cd ${SRC_DIR}/xclbin; \
	v++ -l -t $(EMU_MODE) --platform $(PART) --config ${CONFIG_FILE} ${SRC_DIR}/$(KERNEL_NAME).xo -o $(HLS_NAME).xclbin && \
	mv ${SRC_DIR}/xclbin/${HLS_NAME}.xclbin ${SRC_DIR}/

###############
## Simple export script
export.sh: ${HLS_NAME}.xclbin
	@echo "export XCL_EMULATION_MODE=$(EMU_MODE)\n" > export.sh
	echo "do 'source export.sh' if EMU_MODE was changed"

###############
## config for the part
emconfig.json:
	emconfigutil --platform $(PART)

link:
	rm -f ${HLS_NAME}.xclbin
	${MAKE}

compile:
	rm -f ${KERNEL_NAME}.xo
	${MAKE}

clean:
	rm -rf ./.Xil/
	rm -rf ./.run/
	rm -rf ./kernels
	rm -rf ./xclbin
	rm -f *.xo
	rm -f *.xclbin
	rm -f *.exe
	rm -f emconfig.json
	rm -f export.sh
	rm -f host.exe
	rm -f xilinx_u200_gen*
	rm -f xrt.run_summary
	rm -f emulation_debug.log
	rm -f *.csv
	rm -rf logs
	rm -rf host
	rm -rf _x
	rm -rf *log
	rm -rf build
