
#-------------------------------------------------------------------------------
# Copyright (c) 2013-2013, Lars Baehren <lbaehren@gmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
#  * Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#-------------------------------------------------------------------------------

# - Check for the presence of libmagic
#
# The following variables are set when libmagic is found:
#  libmagic_FOUND        = Set to true, if all components of libmagic have been
#                          found.
#  libmagic_INCLUDES     = Include path for the header files of libmagic
#  libmagic_LIBRARIES    = Link these to use libmagic
#  libmagic_LFLAGS       = Linker flags (optional)
#  libmagic_DEPENDENCIES = Link these external dependencies (for windows's cygwin version of libmagic).
#  libmagic_MAGIC_DB     = Path to the magic database.

if (NOT libmagic_FOUND)

  if (NOT libmagic_ROOT_DIR)
    set (libmagic_ROOT_DIR ${CMAKE_INSTALL_PREFIX})
  endif (NOT libmagic_ROOT_DIR)

  ##____________________________________________________________________________
  ## Check for the header files

  find_path (libmagic_FILE_H
    NAMES file/file.h
    HINTS ${libmagic_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES include
    )
  if (libmagic_FILE_H)
    list (APPEND libmagic_INCLUDES ${libmagic_FILE_H})
  endif (libmagic_FILE_H)

  find_path (libmagic_MAGIC_H
    NAMES magic.h
    HINTS ${libmagic_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES include include/linux
    )
  if (libmagic_MAGIC_H)
    list (APPEND libmagic_INCLUDES ${libmagic_MAGIC_H})
  endif (libmagic_MAGIC_H)

  if (libmagic_INCLUDES)
    list (REMOVE_DUPLICATES libmagic_INCLUDES)
  endif ()

  ##____________________________________________________________________________
  ## Check for the library

  find_library (libmagic_LIBRARIES magic magic-1
    HINTS ${libmagic_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES lib bin
    )

  find_library (libmagic_libgnurx_LIBRARY libgnurx-0
    HINTS ${libmagic_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES lib
    )
  if (libmagic_libgnurx_LIBRARY)
    list (APPEND libmagic_DEPENDENCIES ${libmagic_libgnurx_LIBRARY})
  endif (libmagic_libgnurx_LIBRARY)

  find_file (libmagic_MAGIC_DB
    NAMES magic.mgc
    HINTS ${libmagic_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES magic share/misc
    )

  ##____________________________________________________________________________
  ## Actions taken when all components have been found

  find_package_handle_standard_args (libmagic DEFAULT_MSG libmagic_LIBRARIES libmagic_INCLUDES)

  if (libmagic_FOUND)
    if (NOT libmagic_FIND_QUIETLY)
      message (STATUS "Found components for libmagic")
      message (STATUS "libmagic_ROOT_DIR     = ${libmagic_ROOT_DIR}")
      message (STATUS "libmagic_INCLUDES     = ${libmagic_INCLUDES}")
      message (STATUS "libmagic_LIBRARIES    = ${libmagic_LIBRARIES}")
      message (STATUS "libmagic_DEPENDENCIES = ${libmagic_DEPENDENCIES}")
      message (STATUS "libmagic_MAGIC_DB     = ${libmagic_MAGIC_DB}")
    endif ()
  else (libmagic_FOUND)
    if (libmagic_FIND_REQUIRED)
      message (FATAL_ERROR "Could not find libmagic!")
    endif (libmagic_FIND_REQUIRED)
  endif (libmagic_FOUND)

  ##____________________________________________________________________________
  ## Mark advanced variables

  mark_as_advanced (
    libmagic_ROOT_DIR
    libmagic_INCLUDES
    libmagic_LIBRARIES
    )

endif (NOT libmagic_FOUND)
