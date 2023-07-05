#/*============================================================================
#
#  NifTK: A software platform for medical image computing.
#
#  Copyright (c) University College London (UCL). All rights reserved.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.
#
#  See LICENSE.txt in the top level directory for details.
#
#============================================================================*/

# Construct consistent error messages for use below.
set(NIFTYREG_DIR_DESCRIPTION "directory containing the file 'include/_reg_tools.h'. This is either the root of the build tree, or PREFIX for an installation.")
set(NIFTYREG_DIR_MESSAGE "NIFTYREG not found.  Set the NIFTYREG_DIR cmake cache entry to the ${NIFTYREG_DIR_DESCRIPTION}")


if(NOT NIFTYREG_FOUND)

  # Look for reg_tools.h in build trees or under <prefix>/include.
  find_path(NIFTYREG_DIR
    NAMES include/_reg_f3d.h
    HINTS ENV NIFTYREG_DIR
    PATHS

    # Help the user find it if we cannot.
    DOC "The ${NIFTYREG_DIR_DESCRIPTION}"
    )

  if(NIFTYREG_DIR)
    if(EXISTS ${NIFTYREG_DIR}/include/_reg_f3d.h)
      set(NIFTYREG_FOUND 1)
    else()
      set(NIFTYREG_DIR "NIFTYREG_DIR-NOTFOUND" CACHE PATH "The ${NIFTYREG_DIR_DESCRIPTION}" FORCE)
    endif()
  endif()

endif()


if (NIFTYREG_FOUND)

  set(NIFTYREG_LIBRARIES
    _reg_aladin
    _reg_f3d
    _reg_measure
    _reg_blockMatching
    _reg_femTrans
    _reg_globalTrans
    _reg_localTrans
    _reg_resampling
    _reg_ReadWriteImage
    _reg_tools
    _reg_maths
    reg_png
    reg_nifti
    z
    )

  set(NIFTYREG_INCLUDE_DIR
    ${NIFTYREG_DIR}/include
    )
  set(NIFTYREG_LIBRARY_DIR
    ${NIFTYREG_DIR}/lib
    )

else()
  # Eigen not found, explain to the user how to specify its location.
  if(NIFTYREG_FIND_REQUIRED)
    message(FATAL_ERROR ${NIFTYREG_DIR_MESSAGE})
  else()
    if(NOT NIFTYREG_FIND_QUIETLY)
      message(STATUS ${NIFTYREG_DIR_MESSAGE})
    endif()
  endif()

endif()
