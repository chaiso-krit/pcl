/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2010, Willow Garage, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id$
 *
 */

#include <pcl/cuda/time_cpu.h>
#include <pcl/cuda/time_gpu.h>
#include <pcl/cuda/io/cloud_to_pcl.h>
#include <pcl/cuda/io/extract_indices.h>
#include <pcl/cuda/io/disparity_to_cloud.h>
#include <pcl/cuda/io/host_device.h>

#include <pcl/io/openni_grabber.h>
#include <pcl/io/pcd_grabber.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <boost/shared_ptr.hpp>
#include <pcl/visualization/cloud_viewer.h>
#include <iostream>
#include <pcl/cuda/sample_consensus/sac_model_cylinder.h>
#include <pcl/cuda/sample_consensus/ransac_cylinder.h>

#include <pcl/cuda/features/normal_3d.h>

using namespace pcl::cuda;

//typedef pcl::PointXYZRGB PointT;
typedef pcl::PointNormal PointT;

class SimpleKinectTool
{
  public:
    SimpleKinectTool () : viewer ("KinectGrabber"), go_on(true) {}

    template <template <typename> class Storage>
      boost::shared_ptr<typename Storage<float4>::type> createNormals (size_t size_data)
    {
      boost::shared_ptr<typename Storage<float4>::type> normals (new typename Storage<float4>::type);
      normals->resize (size_data);
      return normals;
    }


    template <template <typename> class Storage> void 
    file_cloud_cb (const pcl::PointCloud<PointT>::ConstPtr& cloud) 
    {
      ScopeTimeCPU ttt ("all");
      pcl::PointCloud<pcl::PointXYZRGB>::Ptr output (new pcl::PointCloud<pcl::PointXYZRGB>);
      PointCloudAOS<Host> data_host;
     // boost::shared_ptr<typename Storage<float4>::type> normals (new typename Storage<float4>::type);

      data_host.points.resize (cloud->points.size());
      //boost::shared_ptr<typename Storage<float4>::type> normals (new typename Storage<float4>::type);

      //normals->resize (50000);

      for (size_t i = 0; i < cloud->points.size (); ++i)
      {
        PointXYZRGB pt;
        pt.x = cloud->points[i].x;
        pt.y = cloud->points[i].y;
        pt.z = cloud->points[i].z;
        // Pack RGB into a float
        //pt.rgb = *(float*)(&cloud->points[i].rgb);
        data_host.points[i] = pt;
      }
      data_host.width = cloud->width;
      data_host.height = cloud->height;
      data_host.is_dense = cloud->is_dense;
      typename PointCloudAOS<Storage>::Ptr data = toStorage<Host, Storage> (data_host);

      //boost::shared_ptr<typename Storage<float4>::type> normals;
      //typename Storage<float4>::type normals (new typename Storage<float4>::type);
      //normals = extractNormals<Storage> (cloud);

      typename SampleConsensusModelCylinder<Storage>::Ptr sac_model (new SampleConsensusModelCylinder<Storage> (data));
      sac_model->setNormalsVector (data);
      RandomSampleConsensusCylinder<Storage> sac (sac_model);
      sac.setMaxIterations (2);
      sac.setDistanceThreshold (1.5);

      {
        for(int i=0; i<1; ++i)
        {
          ScopeTimeCPU timer ("computeModel: ");
          if (!sac.computeModel (0))
          {
            std::cerr << "Failed to compute model" << std::endl;
          }
          else
          {
            typename SampleConsensusModelCylinder<Storage>::IndicesPtr inliers_stencil;
            inliers_stencil = sac.getInliersStencil ();

            OpenNIRGB color;
            color.r = 255; color.g = 255; color.b = 0;
            printf("Before Color\n");
            //std::cerr << data->points.size() << " =?= " << inliers_stencil->size () << std::endl;
            //colorIndices<Storage> (data, inliers_stencil, color);
            printf("After Color\n");
          }
        }
      }
     
      go_on = false;
      //std::cerr << "got here" << std::endl;
      toPCL (*data, *output);
      //std::cerr << "not here" << std::endl;
      viewer.showCloud (output);
    }

    template <template <typename> class Storage> void 
    cloud_cb (const boost::shared_ptr<openni_wrapper::Image>& image,
              const boost::shared_ptr<openni_wrapper::DepthImage>& depth_image, 
              float constant)
    {
      pcl::PointCloud<PointT>::Ptr output (new pcl::PointCloud<PointT>);
      typename PointCloudAOS<Storage>::Ptr data;
      {
      ScopeTimeCPU timer ("All: ");
      // Compute the PointCloud on the device
      d2c.compute<Storage> (depth_image, image, constant, data);

      typename SampleConsensusModelCylinder<Storage>::Ptr sac_model (new SampleConsensusModelCylinder<Storage> (data));
      RandomSampleConsensusCylinder<Storage> sac (sac_model);
      sac.setMaxIterations (10000);
      sac.setDistanceThreshold (0.05);

      {
        ScopeTimeCPU timer ("computeModel: ");
        if (!sac.computeModel (0))
        {
          std::cerr << "Failed to compute model" << std::endl;
        }
        else
        {
          typename SampleConsensusModelCylinder<Storage>::IndicesPtr inliers_stencil;
          inliers_stencil = sac.getInliersStencil ();

          OpenNIRGB color;
          color.r = 253; color.g = 0; color.b = 0;
          colorIndices<Storage> (data, inliers_stencil, color);
        }
      }

    }
      toPCL (*data, *output);
      //viewer.showCloud (output);
    }
    
    void 
    run (bool use_device, char *pcd_filename)
    {
#if 1
      pcl::Grabber* filegrabber = 0;

      float frames_per_second = 1;
      bool repeat = false;

      std::string path = pcd_filename;
      filegrabber = new pcl::PCDGrabber<PointT> (path, frames_per_second, repeat);
      
      if (use_device)
      {
        std::cerr << "[RANSAC] Using GPU..." << std::endl;
        boost::function<void (const pcl::PointCloud<PointT>::ConstPtr&)> f = boost::bind (&SimpleKinectTool::file_cloud_cb<Device>, this, _1);
        filegrabber->registerCallback (f);
      }
      else
      {
        //std::cerr << "[RANSAC] Using CPU..." << std::endl;
        //boost::function<void (const pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr&)> f = boost::bind (&SimpleKinectTool::file_cloud_cb<pcl_cuda::Host>, this, _1);
        //filegrabber->registerCallback (f);
      }

      filegrabber->start ();
      while (!viewer.wasStopped () || go_on)//!viewer.wasStopped () && go_on)
      {
        pcl_sleep (1);
      }
      filegrabber->stop ();

      
      //------- END --------- load pcl logo file
#else

      pcl::Grabber* interface = new pcl::OpenNIGrabber();


      boost::signals2::connection c;
      if (use_device)
      {
        std::cerr << "[RANSAC] Using GPU..." << std::endl;
        boost::function<void (const boost::shared_ptr<openni_wrapper::Image>& image, const boost::shared_ptr<openni_wrapper::DepthImage>& depth_image, float)> f = boost::bind (&SimpleKinectTool::cloud_cb<pcl_cuda::Device>, this, _1, _2, _3);
        c = interface->registerCallback (f);
      }
      else
      {
        std::cerr << "[RANSAC] Using CPU..." << std::endl;
        boost::function<void (const boost::shared_ptr<openni_wrapper::Image>& image, const boost::shared_ptr<openni_wrapper::DepthImage>& depth_image, float)> f = boost::bind (&SimpleKinectTool::cloud_cb<pcl_cuda::Host>, this, _1, _2, _3);
        c = interface->registerCallback (f);
      }

      //viewer.runOnVisualizationThread (fn, "viz_cb");
      interface->start ();
      while (!viewer.wasStopped ())
      {
        sleep (1);
      }

      interface->stop ();
#endif 
    }

    DisparityToCloud d2c;
    pcl::visualization::CloudViewer viewer;
    boost::mutex mutex_;
    bool go_on;
};

int 
main (int argc, char **argv)
{
  bool use_device = false;
  if (argc >= 2)
    use_device = true;
  SimpleKinectTool v;
  v.run (use_device, argv[1]);
  return 0;
}
