CXXFLAGS += --std=c++11

.PHONY: all clean
all: blte_decode.exe minimal_archive_set.exe minimize_encoding.exe minimize_index.exe

clean:
	$(RM) blte.o blte_decode.exe minimal_archive_set.exe minimize_encoding.exe minimize_index.exe

blte.o: blte.cpp blte.hpp bigendian.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.exe: %.cpp blte.o
	$(CXX) $(CXXFLAGS) -lz $^ -o $@