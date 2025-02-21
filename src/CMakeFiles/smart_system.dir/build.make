# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/raspberrypi/Projects/Smart-Monitoring-and-Safety-System/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/raspberrypi/Projects/Smart-Monitoring-and-Safety-System/src

# Include any dependencies generated for this target.
include CMakeFiles/smart_system.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/smart_system.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/smart_system.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/smart_system.dir/flags.make

CMakeFiles/smart_system.dir/gpioevent.cpp.o: CMakeFiles/smart_system.dir/flags.make
CMakeFiles/smart_system.dir/gpioevent.cpp.o: gpioevent.cpp
CMakeFiles/smart_system.dir/gpioevent.cpp.o: CMakeFiles/smart_system.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/raspberrypi/Projects/Smart-Monitoring-and-Safety-System/src/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/smart_system.dir/gpioevent.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/smart_system.dir/gpioevent.cpp.o -MF CMakeFiles/smart_system.dir/gpioevent.cpp.o.d -o CMakeFiles/smart_system.dir/gpioevent.cpp.o -c /home/raspberrypi/Projects/Smart-Monitoring-and-Safety-System/src/gpioevent.cpp

CMakeFiles/smart_system.dir/gpioevent.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/smart_system.dir/gpioevent.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/raspberrypi/Projects/Smart-Monitoring-and-Safety-System/src/gpioevent.cpp > CMakeFiles/smart_system.dir/gpioevent.cpp.i

CMakeFiles/smart_system.dir/gpioevent.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/smart_system.dir/gpioevent.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/raspberrypi/Projects/Smart-Monitoring-and-Safety-System/src/gpioevent.cpp -o CMakeFiles/smart_system.dir/gpioevent.cpp.s

CMakeFiles/smart_system.dir/SmartMonitoringAndSafetySystem.cpp.o: CMakeFiles/smart_system.dir/flags.make
CMakeFiles/smart_system.dir/SmartMonitoringAndSafetySystem.cpp.o: SmartMonitoringAndSafetySystem.cpp
CMakeFiles/smart_system.dir/SmartMonitoringAndSafetySystem.cpp.o: CMakeFiles/smart_system.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/raspberrypi/Projects/Smart-Monitoring-and-Safety-System/src/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/smart_system.dir/SmartMonitoringAndSafetySystem.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/smart_system.dir/SmartMonitoringAndSafetySystem.cpp.o -MF CMakeFiles/smart_system.dir/SmartMonitoringAndSafetySystem.cpp.o.d -o CMakeFiles/smart_system.dir/SmartMonitoringAndSafetySystem.cpp.o -c /home/raspberrypi/Projects/Smart-Monitoring-and-Safety-System/src/SmartMonitoringAndSafetySystem.cpp

CMakeFiles/smart_system.dir/SmartMonitoringAndSafetySystem.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/smart_system.dir/SmartMonitoringAndSafetySystem.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/raspberrypi/Projects/Smart-Monitoring-and-Safety-System/src/SmartMonitoringAndSafetySystem.cpp > CMakeFiles/smart_system.dir/SmartMonitoringAndSafetySystem.cpp.i

CMakeFiles/smart_system.dir/SmartMonitoringAndSafetySystem.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/smart_system.dir/SmartMonitoringAndSafetySystem.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/raspberrypi/Projects/Smart-Monitoring-and-Safety-System/src/SmartMonitoringAndSafetySystem.cpp -o CMakeFiles/smart_system.dir/SmartMonitoringAndSafetySystem.cpp.s

# Object files for target smart_system
smart_system_OBJECTS = \
"CMakeFiles/smart_system.dir/gpioevent.cpp.o" \
"CMakeFiles/smart_system.dir/SmartMonitoringAndSafetySystem.cpp.o"

# External object files for target smart_system
smart_system_EXTERNAL_OBJECTS =

smart_system: CMakeFiles/smart_system.dir/gpioevent.cpp.o
smart_system: CMakeFiles/smart_system.dir/SmartMonitoringAndSafetySystem.cpp.o
smart_system: CMakeFiles/smart_system.dir/build.make
smart_system: CMakeFiles/smart_system.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/raspberrypi/Projects/Smart-Monitoring-and-Safety-System/src/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable smart_system"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/smart_system.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/smart_system.dir/build: smart_system
.PHONY : CMakeFiles/smart_system.dir/build

CMakeFiles/smart_system.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/smart_system.dir/cmake_clean.cmake
.PHONY : CMakeFiles/smart_system.dir/clean

CMakeFiles/smart_system.dir/depend:
	cd /home/raspberrypi/Projects/Smart-Monitoring-and-Safety-System/src && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/raspberrypi/Projects/Smart-Monitoring-and-Safety-System/src /home/raspberrypi/Projects/Smart-Monitoring-and-Safety-System/src /home/raspberrypi/Projects/Smart-Monitoring-and-Safety-System/src /home/raspberrypi/Projects/Smart-Monitoring-and-Safety-System/src /home/raspberrypi/Projects/Smart-Monitoring-and-Safety-System/src/CMakeFiles/smart_system.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/smart_system.dir/depend

