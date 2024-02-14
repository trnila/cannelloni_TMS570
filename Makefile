TI_CGT_ROOT?=/opt/ccstudio/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/
BUILD_DIR=./build/
TARGET=CANnode.out
CC=$(TI_CGT_ROOT)/bin/armcl

CFLAGS= \
	-mv7R5 \
	--code_state=32 \
	--float_support=VFPv3D16 \
	-g \
	--c99 \
	--enum_type=packed \
	--abi=eabi \
	-Isrc \
	-I./TMS570LC435/ \
	-I./lwip/src/include \
	-I./lwip/src/include/lwip \
	-I./lwip/ports/hdk/include/ \
	-I$(TI_CGT_ROOT)/include/

LDFLAGS=\
	-z \
	--heap_size=0x800 \
	--stack_size=0x800 \
	--reread_libs \
	--warn_sections \
	--rom_model \
	--be32 \
	./TMS570LC435.cmd \
	--reread_libs \
	-i$(TI_CGT_ROOT)/lib/ \
	-lrtsv7R4_T_be_v3D16_eabi.lib \
	-llibc.a

OBJS = \
	src/cannelloni.obj \
	src/drivers/can.obj \
	src/drivers/gio.obj \
	src/drivers/spi.obj \
	src/drivers/mdio.obj \
	src/drivers/timer.obj \
	src/drivers/vim.obj \
	src/HL_system.obj \
	src/main.obj \
	src/pinmux.obj \
	src/DP8386.obj \
	src/SJA1105.obj \
	src/startup.obj \
	lwip/ports/hdk/netif/hdkif.obj \
	lwip/ports/hdk/sys_arch.obj \
	lwip/src/core/def.obj \
	lwip/src/core/inet_chksum.obj \
	lwip/src/core/init.obj \
	lwip/src/core/ip.obj \
	lwip/src/core/ipv4/etharp.obj \
	lwip/src/core/ipv4/icmp.obj \
	lwip/src/core/ipv4/ip4_addr.obj \
	lwip/src/core/ipv4/ip4.obj \
	lwip/src/core/mem.obj \
	lwip/src/core/memp.obj \
	lwip/src/core/netif.obj \
	lwip/src/core/pbuf.obj \
	lwip/src/core/sys.obj \
	lwip/src/core/timeouts.obj \
	lwip/src/core/udp.obj \
	lwip/src/netif/ethernet.obj


all: $(BUILD_DIR)/$(TARGET)
$(BUILD_DIR)/$(TARGET): $(addprefix $(BUILD_DIR)/,$(OBJS))
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(BUILD_DIR)/%.obj: %.c
	@mkdir -p $(BUILD_DIR)/$(dir $<)
	$(CC) $(CFLAGS) -fr $(BUILD_DIR)/$(dir $<) $^

$(BUILD_DIR)/%.obj: %.asm
	@mkdir -p $(BUILD_DIR)/$(dir $<)
	$(CC) $(CFLAGS) -fr $(BUILD_DIR)/$(dir $<) $^

flash_%: $(BUILD_DIR)/$(TARGET)
	DSLite flash \
		--config XDS110_chain.ccxml \
		-s FlashEraseSelection=1 \
		--core $* \
		--verbose \
		--flash \
		--verify \
		--run \
		$(BUILD_DIR)/$(TARGET)

flash: flash_0 flash_1 flash_2

clean:
	rm -rf $(BUILD_DIR)
