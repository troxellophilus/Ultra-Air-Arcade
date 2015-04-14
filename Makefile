# Make file to compile on linux (all) or mac (osx)

all:
	g++ -o lab1 -ansi -pedantic -Wno-deprecated *.cpp *.cc -DGL_GLEXT_PROTOTYPES -lglfw -lGL -lGLU

osx:
	g++ -o lab1 -ansi -pedantic -Werror *.cpp *.cc -lGLEW -lglfw3 -framework OpenGL

clean:
	rm -f *~ *.o lab1
