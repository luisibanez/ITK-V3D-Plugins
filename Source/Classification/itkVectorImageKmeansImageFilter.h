/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkVectorImageKmeansImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2010-01-31 19:29:05 $
  Version:   $Revision: 1.7 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkVectorImageKmeansImageFilter_h
#define __itkVectorImageKmeansImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkImage.h"
#include "itkNumericTraits.h"

#include "itkKdTree.h"
#include "itkKdTreeBasedKmeansEstimator.h"
#include "itkWeightedCentroidKdTreeGenerator.h"

#ifdef ITK_USE_REVIEW_STATISTICS
#include "itkEuclideanDistanceMetric.h"
#include "itkSampleClassifierFilter.h"
#include "itkImageToListSampleAdaptor.h"
#include "itkMinimumDecisionRule2.h"
#else
#include "itkMinimumDecisionRule.h"
#include "itkEuclideanDistance.h"
#include "itkSampleClassifier.h"
#include "itkScalarImageToListAdaptor.h"
#endif

#include "itkImageRegion.h"
#include "itkRegionOfInterestImageFilter.h"

#include <vector>

namespace itk
{
/** \class VectorImageKmeansImageFilter
 * \brief Classifies the intensity values of a scalar image using the K-Means algorithm.
 *
 * Given an input image with scalar values, it uses the K-Means statistical
 * classifier in order to define labels for every pixel in the image. The
 * filter is templated over the type of the input image. The output image is
 * predefined as having the same dimension of the input image and pixel type
 * unsigned char, under the assumption that the classifier will generate less
 * than 256 classes. 
 *
 * You may want to look also at the RelabelImageFilter that may be used as a
 * postprocessing stage, in particular if you are interested in ordering the
 * labels by their relative size in number of pixels.
 *
 * \sa Image
 * \sa ImageKmeansModelEstimator
 * \sa KdTreeBasedKmeansEstimator, WeightedCentroidKdTreeGenerator, KdTree
 * \sa RelabelImageFilter
 * 
 * \ingroup ClassificationFilters 
 */
template <class TInputImage,
          class TOutputImage=Image<unsigned char, ::itk::GetImageDimension<TInputImage>::ImageDimension> >
class ITK_EXPORT VectorImageKmeansImageFilter :
    public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Extract dimension from input and output image. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                      TInputImage::ImageDimension);

  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage  InputImageType;
  typedef TOutputImage OutputImageType;

  /** Standard class typedefs. */
  typedef VectorImageKmeansImageFilter                         Self;
  typedef ImageToImageFilter< InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self>                                   Pointer;
  typedef SmartPointer<const Self>                             ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(VectorImageKmeansImageFilter, ImageToImageFilter);
  
  /** Image typedef support. */
  typedef typename InputImageType::PixelType  InputPixelType;
  typedef typename OutputImageType::PixelType OutputPixelType;

  /** Type used for representing the Mean values */
  typedef typename NumericTraits< InputPixelType >::RealType RealPixelType;
  
  /** Create a List from the scalar image */
#ifdef ITK_USE_REVIEW_STATISTICS
  typedef itk::Statistics::ImageToListSampleAdaptor< InputImageType > AdaptorType;
#else
  typedef itk::Statistics::ScalarImageToListAdaptor< 
                                   InputImageType > AdaptorType;
#endif  

  /** Define the Measurement vector type from the AdaptorType */
  typedef typename AdaptorType::MeasurementVectorType  MeasurementVectorType;

#ifdef ITK_USE_REVIEW_STATISTICS
  typedef itk::Statistics::DistanceToCentroidMembershipFunction< MeasurementVectorType > MembershipFunctionType;
  typedef itk::Statistics::SampleClassifierFilter< AdaptorType > ClassifierType;
  typedef itk::Statistics::MinimumDecisionRule2                  DecisionRuleType;
#else
  typedef itk::Statistics::EuclideanDistance< MeasurementVectorType > MembershipFunctionType;
  typedef itk::Statistics::SampleClassifier< AdaptorType > ClassifierType;
  typedef itk::MinimumDecisionRule                         DecisionRuleType;
#endif

#ifdef ITK_USE_REVIEW_STATISTICS
  typedef typename ClassifierType::ClassLabelVectorType ClassLabelVectorType;
#else
  typedef std::vector< unsigned int > ClassLabelVectorType;
#endif
  
#ifdef ITK_USE_REVIEW_STATISTICS
  typedef typename ClassifierType::MembershipFunctionVectorType MembershipFunctionVectorType;
  typedef typename MembershipFunctionType::CentroidType  MembershipFunctionOriginType;
#else
  typedef typename MembershipFunctionType::OriginType  MembershipFunctionOriginType;
#endif
  
  typedef typename MembershipFunctionType::Pointer     MembershipFunctionPointer;
  
  /** Create the K-d tree structure */
  typedef itk::Statistics::WeightedCentroidKdTreeGenerator< 
                                                      AdaptorType > 
                                                                TreeGeneratorType;
  typedef typename TreeGeneratorType::KdTreeType                TreeType;
  typedef itk::Statistics::KdTreeBasedKmeansEstimator<TreeType> EstimatorType;

  typedef typename EstimatorType::ParametersType ParametersType;

  typedef typename InputImageType::RegionType        ImageRegionType;
  
  typedef RegionOfInterestImageFilter< 
                                 InputImageType,
                                 InputImageType  > RegionOfInterestFilterType;


  /** Add a new class to the classification by specifying its initial mean. */
  void AddClassWithInitialMean( RealPixelType mean );

  /** Return the array of Means found after the classification */
  itkGetConstReferenceMacro( FinalMeans, ParametersType );

  /** Set/Get the UseNonContiguousLabels flag. When this is set to false the
   * labels are numbered contiguously, like in {0,1,3..N}. When the flag is set
   * to true, the labels are selected in order to span the dynamic range of the
   * output image. This last option is useful when the output image is intended
   * only for display. The default value is false. */
  itkSetMacro( UseNonContiguousLabels, bool );
  itkGetConstReferenceMacro( UseNonContiguousLabels, bool );
  itkBooleanMacro( UseNonContiguousLabels );

  /** Set Region method to constrain classfication to a certain region */
  void SetImageRegion( const ImageRegionType & region );
  
  /** Get the region over which the statistics will be computed */
  itkGetConstReferenceMacro( ImageRegion, ImageRegionType );
  
#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro(InputHasNumericTraitsCheck,
                  (Concept::HasNumericTraits<InputPixelType>));
  /** End concept checking */
#endif

protected:
  VectorImageKmeansImageFilter();
  virtual ~VectorImageKmeansImageFilter() {}
  void PrintSelf(std::ostream& os, Indent indent) const;

  /** This method runs the statistical methods that identify the means of the
   * classes and the use the distances to those means in order to label the
   * image pixels.
   * \sa ImageToImageFilter::GenerateData() 
   */
  void GenerateData();

private:
  VectorImageKmeansImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  typedef std::vector< RealPixelType > MeansContainer;

  MeansContainer  m_InitialMeans;

  ParametersType  m_FinalMeans;

  bool m_UseNonContiguousLabels;

  ImageRegionType m_ImageRegion;

  bool m_ImageRegionDefined;
};
  
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkVectorImageKmeansImageFilter.txx"
#endif

#endif
