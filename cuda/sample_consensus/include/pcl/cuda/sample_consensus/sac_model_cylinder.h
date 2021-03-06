/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2009, Willow Garage, Inc.
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
 * Author: Nico Blodow (blodow@cs.tum.edu)
 *
 * $Id$
 *
 */

#ifndef PCL_CUDA_SAMPLE_CONSENSUS_MODEL_CYLINDER_H_
#define PCL_CUDA_SAMPLE_CONSENSUS_MODEL_CYLINDER_H_

#include <pcl/cuda/sample_consensus/sac_model.h>
#include <thrust/random.h>

#include <pcl/point_types.h>

namespace pcl
{
  namespace cuda
  {
    /** \brief Check if a certain tuple is a point inlier. */
    struct CountPlanarInlier
    {
      float4 coefficients;
      float threshold;

      CountPlanarInlier (float4 coeff, float thresh) : 
        coefficients(coeff), threshold(thresh) 
      {}

      template <typename Tuple> __inline__ __host__ __device__ bool
      operator () (const Tuple &t);
    };

    /** \brief Check if a certain tuple is a point inlier. */
    template <template <typename> class Storage>
    struct NewCheckPlanarInlier
    {
      float4 coefficients;
      float threshold;
      const typename Storage<PointXYZRGB>::type &input_;

      NewCheckPlanarInlier (float4 coeff, float thresh, const typename Storage<PointXYZRGB>::type &input) : 
        coefficients(coeff), threshold(thresh), input_(input)
      {}

      __inline__ __host__ __device__ int
      operator () (const int &idx);
    };

    /** \brief Check if a certain tuple is a point inlier. */
    struct CheckPlanarInlier
    {
      float4 coefficients;
      float threshold;

      CheckPlanarInlier (float4 coeff, float thresh) : 
        coefficients(coeff), threshold(thresh) 
      {}

      template <typename Tuple> __inline__ __host__ __device__ int
      operator () (const Tuple &t);
    };

    /** \brief Check if a certain tuple is a point inlier. */
    struct CheckPlanarInlierIndices
    {
      float4 coefficients;
      float threshold;

      CheckPlanarInlierIndices (float4 coeff, float thresh) : 
        coefficients(coeff), threshold(thresh) 
      {}

      __inline__ __host__ __device__ int
      operator () (const PointXYZRGB &pt, const int &idx);
    };

    /** \brief Check if a certain tuple is a point inlier. */
    struct CheckPlanarInlierKinectNormalIndices
    {
      float4 coefficients;
      float threshold;
      float angle_threshold;

      CheckPlanarInlierKinectNormalIndices (float4 coeff, float thresh, float angle_thresh) : 
        coefficients(coeff), threshold(thresh), angle_threshold (angle_thresh)
      {}

      template <typename Tuple> __inline__ __host__ __device__ int
      operator () (const Tuple &t, const int &idx);
    };

    /** \brief Check if a certain tuple is a point inlier. */
    struct CheckCylinderInlier
    {
      float3 line_pt;
      float3 line_dir;
      float radius;
      float ptdotdir;
      float dirdotdir;
      float threshold;

      CheckCylinderInlier (float3 _line_pt, float3 _line_dir, float _radius, float _ptdotdir, float _dirdotdir, float thresh) : 
        line_pt(_line_pt), line_dir(_line_dir), radius(_radius), ptdotdir(_ptdotdir), dirdotdir(_dirdotdir), threshold(thresh)
      {}

      template <typename Tuple> __inline__ __host__ __device__ int
      operator () (const Tuple &t, const int &idx);
    };

    /** \brief Check if a certain tuple is a point inlier. */
    struct CheckPlanarInlierKinectIndices
    {
      float4 coefficients;
      float threshold;
      float angle_threshold;

      CheckPlanarInlierKinectIndices (float4 coeff, float thresh, float angle_thresh) : 
        coefficients(coeff), threshold(thresh), angle_threshold (angle_thresh)
      {}

      __inline__ __host__ __device__ int
      operator () (const PointXYZRGB &pt, const int &idx);
    };

    /** \brief Check if a certain tuple is a point inlier. */
    struct CheckPlanarInlierNormalIndices
    {
      float4 coefficients;
      float threshold;
      float angle_threshold;

      CheckPlanarInlierNormalIndices (float4 coeff, float thresh, float angle_thresh) : 
        coefficients(coeff), threshold(thresh), angle_threshold (angle_thresh)
      {}

      template <typename Tuple>
      __inline__ __host__ __device__ int
      operator () (const Tuple &pt, const int &idx);
    };

    struct ConvertCloudToNormal
    {
      __host__ __device__ float4
      operator () (const PointXYZRGB &pt)
      {
        return make_float4 (pt.x, pt.y, pt.z, 0.0f);
      }
    };
    ////////////////////////////////////////////////////////////////////////////////////////////
    /** \brief @b SampleConsensusModelCylinder defines a model for 3D plane segmentation.
      */
    template <template <typename> class Storage>
    class SampleConsensusModelCylinder : public SampleConsensusModel<Storage>
    {
      public:
        using SampleConsensusModel<Storage>::input_;
        using SampleConsensusModel<Storage>::normals_;
        using SampleConsensusModel<Storage>::indices_;
        using SampleConsensusModel<Storage>::indices_stencil_;
        using SampleConsensusModel<Storage>::rngl_;

        typedef typename SampleConsensusModel<Storage>::PointCloud PointCloud;
        typedef typename PointCloud::Ptr PointCloudPtr;
        typedef typename PointCloud::ConstPtr PointCloudConstPtr;

        typedef typename SampleConsensusModel<Storage>::Indices Indices;
        typedef typename SampleConsensusModel<Storage>::IndicesPtr IndicesPtr;
        typedef typename SampleConsensusModel<Storage>::IndicesConstPtr IndicesConstPtr;

        typedef typename SampleConsensusModel<Storage>::Coefficients Coefficients;
        typedef typename SampleConsensusModel<Storage>::Hypotheses Hypotheses;
        typedef typename SampleConsensusModel<Storage>::HypothesesVector HypothesesVector;
        typedef typename SampleConsensusModel<Storage>::Samples Samples;


        typedef boost::shared_ptr<SampleConsensusModelCylinder> Ptr;

        /** \brief Constructor for base SampleConsensusModelCylinder.
          * \param cloud the input point cloud dataset
          */
        SampleConsensusModelCylinder (const PointCloudConstPtr &cloud);

        /** \brief Get 3 random non-collinear points as data samples and return them as point indices.
          * \param iterations the internal number of iterations used by SAC methods
          * \param samples the resultant model samples
          * \note assumes unique points!
          */
        void 
        getSamples (int &iterations, Indices &samples);

        void
        setNormalsVector (const PointCloudConstPtr &cloud_normal);
        /** \brief Check whether the given index samples can form a valid plane model, compute the model coefficients from
          * these samples and store them in model_coefficients. The plane coefficients are:
          * a, b, c, d (ax+by+cz+d=0)
          * \param samples the point indices found as possible good candidates for creating a valid model
          * \param model_coefficients the resultant model coefficients
          */
        bool 
        computeModelCoefficients (const Indices &samples, Coefficients &model_coefficients);

        bool 
        generateModelHypotheses (Hypotheses &h, int max_iterations);

        bool 
        generateModelHypotheses (Hypotheses &h, Samples &s, int max_iterations);

        bool 
        generateModelHypothesesVector (HypothesesVector &hv, int max_iterations);

        /** \brief Select all the points which respect the given model coefficients as inliers.
          * \param model_coefficients the coefficients of a plane model that we need to 
          * compute distances to
          * \param threshold a maximum admissible distance threshold for determining the 
          * inliers from the outliers
          * \param inliers the resultant model inliers
          * \param inliers_stencil
          */
        int
        selectWithinDistance (const Coefficients &model_coefficients, 
                              float threshold, IndicesPtr &inliers, IndicesPtr &inliers_stencil);

        int
        selectWithinDistance (const Hypotheses &h, int idx,
                              float threshold,
                              IndicesPtr &inliers, IndicesPtr &inliers_stencil);
        int
        selectWithinDistance (Hypotheses &h, int idx,
                              float threshold,
                              IndicesPtr &inliers_stencil,
                              float3 &centroid);

        int
        selectWithinDistance (const HypothesesVector &hv, int idx,
                              float threshold,
                              IndicesPtr &inliers_stencil);
        int
        countWithinDistance (const Coefficients &model_coefficients, float threshold);

        int
        countWithinDistance (const HypothesesVector &hv, int idx, float threshold);

        int
        countWithinDistance (const Hypotheses &h, int idx, float threshold);

  //    private:
  //      /** \brief Define the maximum number of iterations for collinearity checks */
        const static int MAX_ITERATIONS_COLLINEAR = 1000;
    };

    /** \brief Check if a certain tuple is a point inlier. */
    template <template <typename> class Storage>
    struct CreateCylinderHypothesis
    {
      typedef typename SampleConsensusModel<Storage>::PointCloud PointCloud;
      typedef typename PointCloud::ConstPtr PointCloudConstPtr;
      typedef typename SampleConsensusModel<Storage>::Indices Indices;
      typedef typename SampleConsensusModel<Storage>::IndicesConstPtr IndicesConstPtr;

      const PointXYZRGB *input;
      const float4 *normals_;
      const int *indices;
      int nr_indices;
      float bad_value;

      CreateCylinderHypothesis (const PointXYZRGB *_input, const float4* normals, const int *_indices, int _nr_indices, float bad) : 
        input(_input), normals_(normals), indices(_indices), nr_indices(_nr_indices), bad_value(bad)
      {}

      //template <typename Tuple> 
      __inline__ __host__ __device__ CylinderHypotheses
      //operator () (const Tuple &t);
      operator () (int t);
    };

    /** \brief Check if a certain tuple is a point inlier. */
    
    template <template <typename> class Storage>
    struct CreateCylinderSampleHypothesis
    {
      typedef typename SampleConsensusModel<Storage>::PointCloud PointCloud;
      typedef typename PointCloud::ConstPtr PointCloudConstPtr;
      typedef typename SampleConsensusModel<Storage>::Indices Indices;
      typedef typename SampleConsensusModel<Storage>::IndicesConstPtr IndicesConstPtr;

      const PointXYZRGB *input;
      const float4 *normals_;
      const int *indices;
      int width_;
      int height_;
      int nr_indices;
      float bad_value;
      thrust::default_random_engine rng;

      CreateCylinderSampleHypothesis (const PointXYZRGB *_input, const float4* normals, const int *_indices, int width, int height, int _nr_indices, float bad) : 
        input(_input), normals_(normals), indices(_indices), width_(width), height_(height), nr_indices(_nr_indices), bad_value(bad)
      {
      }

      //template <typename Tuple> 
      __inline__ __host__ __device__ thrust::tuple<int,float4>
      //operator () (const Tuple &t);
      operator () (int t);
    };
    

    struct parallel_random_generator 
    { 
      
      __inline__ __host__ __device__ 
      parallel_random_generator(unsigned int seed) 
      { 
        m_seed = seed; 
      } 

      __inline__ __host__ __device__ 
      unsigned int operator()(const unsigned int n) const 
      { 
        thrust::default_random_engine rng(m_seed); 
        // discard n numbers to avoid correlation 
        rng.discard(n); 
        // return a random number 
        return rng(); 
      } 
      unsigned int m_seed; 
    }; 

  } // namespace
} // namespace

#endif  //#ifndef PCL_CUDA_SAMPLE_CONSENSUS_MODEL_CYLINDER_H_
