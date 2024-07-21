CFLAGS = -std=c++17 -O3
LDFLAGSLINUX = -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl
LDFLAGSWINDOWS = -lglfw3 -lopengl32

buildlinux:
	g++ $(CFLAGS) -I ../include -L ../lib -o opengl ../src/*.cpp ../src/*.c $(LDFLAGSLINUX)

buildwindows:
	g++ $(CFLAGS) -I ../include -L ../lib -o opengl ../src/*.cpp ../src/*.c $(LDFLAGSWINDOWS)

clean:
	rm vulkan vulkan.exe