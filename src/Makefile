# Make file to compile on linux (all) or mac (osx)
CC=g++
CFLAGS= -std=c++11 -ansi -pedantic -Wno-deprecated -g
INC=-I$(EIGEN3_INCLUDE_DIR)
LIB=-DGL_GLEXT_PROTOTYPES -lglfw3 -lrt -lXrandr -lXinerama -lXi -lXcursor -lGL -lm -ldl -lXrender -ldrm -lXdamage -lX11-xcb -lxcb-glx -lxcb-dri2 -lxcb-dri3 -lxcb-present -lxcb-sync -lxshmfence -lXxf86vm -lXfixes -lXext -lX11 -lpthread -lxcb -lXau -lXdmcp
LIB_OSX= -framework OpenGL


all:
	$(CC) $(CFLAGS) $(INC) *.cpp *.cc $(LIB)


osx:
	g++ -o lab1 -ansi -pedantic -Werror *.cpp *.cc -lGLEW -lglfw3 -framework OpenGL

clean:
	rm -f *~ *.o lab1
