# Makefile for BDT codes

# $< = name of first dependancy
# $@ = name of target ("main")
# -c indique au compileur de ne pas linker
# -o spÃ©cifie la cible
#

#Define variables
CC=g++ #Compiler used
DEBUG = -g
CFLAGS = -c  $(DEBUG) #Compilation options
#LFLAGS = $(DEBUG) -L/home/nico/root/lib -lTMVA -lTMVAGui
LFLAGS = $(DEBUG) -L/usr/local/root-6.05.02/lib -lTMVA -lTMVAGui
ROOTFLAGS = `root-config --glibs --cflags`
SRCS = $(wildcard *.cc) #Source files are all files with .cc extension
HDRS = $(wildcard *.h) #Header files are all files with .h extension
OBJS = $(SRCS:.cc=.o) #Object files are all files with .o extension, which have same names as source files
EXEC = main #Name of executable file


#Instructions
all: $(EXEC)

#Obtain executable from object files
$(EXEC): $(OBJS)
	@$(CC) $(OBJS) -o $@ $(ROOTFLAGS) $(LFLAGS)
	@echo "-- Linking --"
	@echo "-- Done --"

#Obtain objects from source and header files
%.o: %.cc $(HDRS)
	@$(CC) $(ROOTFLAGS) $(CFLAGS) $< -o $@
	@echo "-- Compiling --"

#Erase all objects and executable
clean:
	@rm -f $(OBJS) $(EXEC)
