CXX = g++
CXXFLAGS = -std=c++17 -O2
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

%: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(basename $(wildcard *.cpp))