CFLAGS = -std=c++20
LDFLAGSLINUX = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lassimp
LDFLAGSWINDOWS = -lglfw3 -lopengl32 -lgdi32 -limm32 -lz -lassimp

buildlinux:
	g++ $(CFLAGS) -I ../include -L ../lib -o opengl ../src/*.cpp ../src/*.c ../include/imGui/*.cpp $(LDFLAGSLINUX)

buildwindows:
	g++ $(CFLAGS) -I ../include -L ../lib -o opengl ../src/*.cpp ../src/*.c ../include/imGui/*.cpp $(LDFLAGSWINDOWS)

clean:
	rm opengl opengl.exe