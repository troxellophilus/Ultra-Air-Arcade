INC=-I$(EIGEN3_INCLUDE_DIR)

all:
	g++ -Wno-deprecated $(INC) *.cpp *.cc -DGL_GLEXT_PROTOTYPES -lglut -lGL -lGLU

osx:
	clang++ -v -Wno-deprecated $(INC) *.cpp -framework GLUT -framework OpenGL

clean:
	rm -f *~ *.o a.out
