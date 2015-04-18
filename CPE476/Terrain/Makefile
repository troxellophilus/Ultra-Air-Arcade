INC=-I$(EIGEN3_INCLUDE_DIR)

all:
	g++ -Wno-deprecated $(INC) *.cpp *.cc -DGL_GLEXT_PROTOTYPES -lglut -lGL -lGLU

osx:
	g++ -Wno-deprecated $(INC) *.cpp *.cc -framework GLUT -framework OpenGL

clean:
	rm -f *~ *.o a.out
