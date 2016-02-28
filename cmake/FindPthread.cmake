# - Try to find Pthread
# Once done this will define
#  
#  PTHREAD_FOUND		- system has Pthread
#  PTHREAD_INCLUDE_DIR  - the Pthread include directory
#  PTHREAD_LIBRARIES	- link these to use Pthread
#

find_path(PTHREAD_INCLUDE_DIR pthread.h
  PATH_SUFFIXES include
)
#MESSAGE("PTHREAD_INCLUDE_DIR is ${PTHREAD_INCLUDE_DIR}")

find_library(PTHREAD_LIBRARIES
  NAMES pthread.lib pthreadVCE2.lib
  HINTS
  PATH_SUFFIXES lib
)
#MESSAGE("PTHREAD_LIBRARIES is ${PTHREAD_LIBRARIES}")

find_package(PackageHandleStandardArgs)
find_package_handle_standard_args(PTHREAD
  REQUIRED_VARS PTHREAD_INCLUDE_DIR PTHREAD_LIBRARIES)