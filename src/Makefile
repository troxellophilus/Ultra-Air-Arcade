# Make file to compile on linux (all) or mac (osx)
CC=g++
CFLAGS= -ansi -pedantic -Wno-deprecated -g -std=c++11 
INC=-I$(EIGEN3_INCLUDE_DIR)
LIB=-DGL_GLEXT_PROTOTYPES -lGL -lGLEW -lglfw -lrt -lXrandr -lXinerama -lXi -lXcursor -lGL -lm -ldl -lXrender -ldrm -lXdamage -lX11-xcb -lxcb-glx -lxcb-dri2 -lxcb-dri3 -lxcb-present -lxcb-sync -lxshmfence -lXxf86vm -lXfixes -lXext -lX11 -lpthread -lxcb -lXau -lXdmcp
LIB_OSX= -framework OpenGL


all:
	$(CC) $(CFLAGS) $(INC) *.cpp ext/*.cpp ext/*.cc $(LIB)


osx:
	clang++ -std=c++11 -stdlib=libc++ *.cpp ext/*.cpp ext/*.cc -lglfw3 -framework OpenGL -lGLEW -framework Cocoa -DGL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED

fedora:
	g++ *.cpp ext/*.cpp ext/*.cc -lGL -lGLEW -lglfw -lSOIL -std=c++11

clean:
	rm -f *~ *.o lab1
