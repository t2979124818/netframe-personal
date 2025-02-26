CXX = g++
CXXFLAGS := -std=c++20 -Wall -Wextra -g
#动态链接所需要的参数如-lring
LFLAGS :=
OUTPUT := output
SRC := src
INCLUDE := include 
#-L动态链接库
LIB := lib 
SOURCEDIRS := $(shell find $(SRC) -type d)
INCLUDEDIRS := $(shell find $(INCLUDE) -type d)
LIBDIRS := $(shell find $(LIB) -type d)

FIXPATH = $1
RM := rm -f 
MD := mkdir -p

#把文件路径后的/去掉
LIBS := $(patsubst %,-L%,$(SOURCEDIRS:%/=%))
INCLUDES := $(patsubst %,-I%,$(INCLUDEDIRS:%/=%))

#启用函数中的通配符，然后把路径换成文件夹下的cpp源文件
SOURCES := $(wildcard $(patsubst %,%/*.cpp,$(SOURCEDIRS)))

SOURCESFO := $(filter-out src/tcpclient.cpp src/tcpserver.cpp src/main.cpp,$(SOURCES))
#.cpp换成.o
OBJECTS := $(SOURCES:.cpp=.o)
OBJECTSFO := $(SOURCESFO:.cpp=.o)
#.o换成.d用于输出依赖关系，只要依赖项目变动就再次编译，
#用于防止套娃式编程导致内部包含的代码改动了但makefile没有发现(1(23)(45))->(1(23)(456))
DEPS := $(OBJECTS:.o=.d)
DEPSFO := $(OBJECTSFO:.o=.d)

#给$1赋上.o输出文件的值
OUTPUTMAIN := $(call FIXPATH,$(OUTPUT)/$(MAIN))

CLIENT := tcpclient
SERVER := tcpserver
MAIN := main
#用来多个文件的make
FILE := 

ifneq ($(wildcard $(SOURCEDIRS)/$(CLIENT).cpp ),)
	FILE += $(CLIENT)
endif
ifneq ($(wildcard $(SOURCEDIRS)/$(SERVER).cpp ),)
	FILE += $(SERVER)
endif
ifneq ($(wildcard $(SOURCEDIRS)/$(MAIN).cpp ),)
	FILE += $(MAIN)
endif

#当OUTPUT和MAIN执行完成是输出
#all: $(OUTPUT) $(MAIN)
all:$(OUTPUT) $(FILE)
	@echo Exec all done!

$(OUTPUT):
	$(MD) $(OUTPUT)
$(MAIN): $(OBJECTSFO) $(SOURCEDIRS)/$(MAIN).cpp
	$(CXX) -o $(OUTPUT)/$(MAIN) $(SOURCEDIRS)/$(MAIN).cpp $(OBJECTSFO) $(CXXFLAGS) $(LFLAGS) $(INCLUDES) $(LIBS)  
$(CLIENT):$(OBJECTSFO) $(SOURCEDIRS)/$(CLIENT).cpp
	$(CXX) -o $(OUTPUT)/$(CLIENT) $(SOURCEDIRS)/$(CLIENT).cpp $(OBJECTSFO) $(CXXFLAGS) $(LFLAGS) $(INCLUDES) $(LIBS)
$(SERVER):$(OBJECTSFO) $(SOURCEDIRS)/$(SERVER).cpp
	$(CXX) -o $(OUTPUT)/$(SERVER) $(SOURCEDIRS)/$(SERVER).cpp $(OBJECTSFO) $(CXXFLAGS) $(LFLAGS) $(INCLUDES) $(LIBS)
#-include $(DEPS)
-include $(DEPSFO)

# $@ 输出全部参数列表
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -MMD $< -o $@

#执行clean(make clean)时无论是否有同名文件(clean)都会被执行
.PHONY:clean
clean:
	$(RM) $(SOURCEDIRS)/*.o $(SOURCEDIRS)/*.d
#$(RM) $(OUTPUTMAIN)
#$(RM) $(call FIXPATH,$(OBJECTS))
#$(RM) $(call FIXPATH,$(DEPS))
	@echo Clean done!
run:all
	./$(OUTPUT)
	@echo Exec run:all done!