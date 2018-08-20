EXE = scfw_cli

SRC_DIR = src
OBJ_DIR = obj

SCFW_DIR = /home/manuel/Documents/Devices/imx8/Software/imx8_yocto_beta2/bld-x11-8qm/tmp/work/imx8qmlpddr4arm2-poky-linux/linux-imx/4.9.51-r0/git/include/

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

#CPPFLAGS += -I$(SCFW_DIR)
#CFLAGS += -Wall
# Linker flags
#LDFLAGS += -Llib
#LDLIBS += -lglfw3 -lX11 -ldl -lpthread

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ) $(EXE)
