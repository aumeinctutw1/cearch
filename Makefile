CXX=clang++
CXXFLAGS=-Wall -Wextra -std=c++20 -O3
CXXLIBS=-lpugixml -lboost_system -lpoppler-cpp

APP_NAME=cearch
SOURCE_DIR=source
BUILD_DIR=build

OS:=$(shell uname)

ifeq ($(OS), Darwin)
	TARGET=build_mac
else
	TARGET=build
endif

all: $(TARGET)

dirs: 
	mkdir -p $(BUILD_DIR)

# find all cpp files in the source dir
SOURCES=$(wildcard $(SOURCE_DIR)/*.cpp)
# rename all cpp files to object files in the build dir
OBJS=$(patsubst $(SOURCE_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))

build: $(OBJS)
	$(CXX) $(FLAGS) $^ -o $(APP_NAME) $(CXXLIBS)

# needed building locally on Mac 
MAC_INCLUDES =  -I/opt/homebrew/Cellar/boost/1.86.0/include \
				-I/opt/homebrew/Cellar/poppler/24.04.0_1/include 
MAC_LIBS=-lpugixml -lpoppler-cpp -L/opt/homebrew/Cellar/poppler/24.04.0_1/lib/ 

# link object files in build dir to final executable
build_mac: $(OBJS)
	$(CXX) $(MAC_INCLUDES) $(MAC_LIBS) -o $(APP_NAME) $(FLAGS) $^

# rule to build the object files
# build the objects the "|" is used to tell make that dirs must exist
# befor the target can be executed
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp | dirs 
	$(CXX) $(CXXFLAGS) $(MAC_INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(APP_NAME)




