set(SUBSYS_NAME cuda_apps)
set(SUBSYS_DESC "Point cloud library CUDA apps")
set(SUBSYS_DEPS cuda_common cuda_io cuda_features cuda_segmentation cuda_sample_consensus common io visualization geometry)

# ---[ Point Cloud Library - pcl/cuda/apps
# Find VTK 
if(NOT VTK_FOUND)
    set(DEFAULT FALSE)
    set(REASON "VTK was not found.")
else(NOT VTK_FOUND)
    set(DEFAULT TRUE)
    set(REASON)
    message(STATUS "VTK found (include: ${VTK_INCLUDE_DIRS}, lib: ${VTK_LIBRARY_DIRS})")
    include("${VTK_USE_FILE}")
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
endif(NOT VTK_FOUND)
PCL_SUBSYS_OPTION(build "${SUBSYS_NAME}" "${SUBSYS_DESC}" ${DEFAULT} "${REASON}")
mark_as_advanced("BUILD_${SUBSYS_NAME}")
PCL_SUBSYS_DEPEND(build "${SUBSYS_NAME}" DEPS ${SUBSYS_DEPS})

if(build)
  if(VTK_FOUND AND WITH_OPENNI)

    PCL_CUDA_ADD_EXECUTABLE(kinect_viewer_cuda "${SUBSYS_NAME}" src/kinect_viewer_cuda.cpp)
    target_link_libraries (kinect_viewer_cuda pcl_io pcl_cuda_io pcl_visualization pcl_common pcl_kdtree)

    PCL_CUDA_ADD_EXECUTABLE(kinect_ransac "${SUBSYS_NAME}" src/kinect_ransac.cpp)
    target_link_libraries (kinect_ransac pcl_io pcl_cuda_io pcl_visualization pcl_common pcl_kdtree pcl_cuda_segmentation pcl_cuda_sample_consensus)


    #PCL_CUDA_ADD_EXECUTABLE(kinect_ransac_file "${SUBSYS_NAME}" src/kinect_ransac_file.cpp)
    #target_link_libraries (kinect_ransac_file pcl_io pcl_cuda_io pcl_visualization pcl_common pcl_kdtree pcl_cuda_segmentation pcl_cuda_sample_consensus)

    PCL_CUDA_ADD_EXECUTABLE(kinect_ransac_cylinder "${SUBSYS_NAME}" src/kinect_ransac_cylinder.cpp)
    target_link_libraries (kinect_ransac_cylinder pcl_io pcl_cuda_io pcl_visualization pcl_common pcl_kdtree pcl_cuda_segmentation pcl_cuda_sample_consensus)

    find_package(OpenCV REQUIRED)
    PCL_CUDA_ADD_EXECUTABLE(kinect_segmentation_cuda "${SUBSYS_NAME}" src/kinect_segmentation_cuda.cpp)
    target_link_libraries (kinect_segmentation_cuda pcl_io pcl_cuda_io pcl_visualization pcl_common pcl_kdtree pcl_cuda_features pcl_cuda_segmentation pcl_cuda_sample_consensus ${OpenCV_LIBS})

    #include_directories(${PCL_INCLUDE_DIRS})
    #link_directories(${PCL_LIBRARY_DIRS})
    #PCL_CUDA_ADD_EXECUTABLE(kinect_cylinder_detction "${SUBSYS_NAME}" src/kinect_cylinder_detection.cpp)
    #target_link_libraries (kinect_cylinder_detection ${PCL_LIBRARIES})

  endif()
endif(build)




