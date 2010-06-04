  /*=========================================================================

    Program:   Insight Segmentation & Registration Toolkit
    Module:    $RCSfile: itkVectorImageKmeansImageFilter.txx,v $
    Language:  C++
    Date:      $Date: 2010-01-31 19:29:05 $
    Version:   $Revision: 1.13 $

    Copyright (c) Insight Software Consortium. All rights reserved.
    See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

       This software is distributed WITHOUT ANY WARRANTY; without even 
       the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
       PURPOSE.  See the above copyright notices for more information.

  =========================================================================*/
#ifndef __itkVectorImageKmeansImageFilter_txx
#define __itkVectorImageKmeansImageFilter_txx

#include "itkVectorImageKmeansImageFilter.h"
#include "itkImageRegionExclusionIteratorWithIndex.h"

#ifdef ITK_USE_REVIEW_STATISTICS
#include "itkDistanceToCentroidMembershipFunction.h"
#endif

#include "itkProgressReporter.h"

namespace itk
{

template <class TInputImage, class TOutputImage>
VectorImageKmeansImageFilter<TInputImage, TOutputImage>
::VectorImageKmeansImageFilter()
{
  m_UseNonContiguousLabels = false;
  m_ImageRegionDefined = false;
}

template <class TInputImage, class TOutputImage>
void VectorImageKmeansImageFilter<TInputImage, TOutputImage>
::SetImageRegion( const ImageRegionType & region )
{
  m_ImageRegion = region;
  m_ImageRegionDefined = true;
}


  
template< class TInputImage, class TOutputImage >
void
VectorImageKmeansImageFilter< TInputImage, TOutputImage >
::GenerateData()
{
  typename AdaptorType::Pointer adaptor = AdaptorType::New();

  // Setup the regions here if a sub-region has been specified to restrict
  // classification on.. Since this is not ThreadedGenenerateData, we are
  // safe...
  if( m_ImageRegionDefined )
    {
    typename RegionOfInterestFilterType::Pointer regionOfInterestFilter 
      = RegionOfInterestFilterType::New();
    regionOfInterestFilter->SetRegionOfInterest( m_ImageRegion );
    regionOfInterestFilter->SetInput( this->GetInput() );
    regionOfInterestFilter->Update();
    adaptor->SetImage( regionOfInterestFilter->GetOutput() );
    }
  else
    { 
    adaptor->SetImage( this->GetInput() );
    }

  typename TreeGeneratorType::Pointer treeGenerator = TreeGeneratorType::New();

  treeGenerator->SetSample( adaptor );
  treeGenerator->SetBucketSize( 16 );
  treeGenerator->Update();

  typename EstimatorType::Pointer estimator = EstimatorType::New();

  const unsigned int numberOfClasses = this->m_InitialMeans.size();

  ParametersType  initialMeans( numberOfClasses );
  for(unsigned int cl=0; cl<numberOfClasses; cl++)
    {
    initialMeans[cl] = this->m_InitialMeans[cl];
    }

  estimator->SetParameters( initialMeans );
    
  estimator->SetKdTree( treeGenerator->GetOutput() );
  estimator->SetMaximumIteration( 200 );
  estimator->SetCentroidPositionChangesThreshold(0.0);
  estimator->StartOptimization();

  this->m_FinalMeans = estimator->GetParameters();

  typedef typename InputImageType::RegionType RegionType;
  typedef typename InputImageType::SizeType   SizeType;


  // Now classify the samples
  //

  DecisionRuleType::Pointer decisionRule = DecisionRuleType::New();
  typename ClassifierType::Pointer classifier = ClassifierType::New();

  classifier->SetDecisionRule( decisionRule.GetPointer() );
#ifdef ITK_USE_REVIEW_STATISTICS
  classifier->SetInput( adaptor );
#else
  classifier->SetSample( adaptor );
#endif

  classifier->SetNumberOfClasses( numberOfClasses  );


  ClassLabelVectorType classLabels;
  classLabels.resize( numberOfClasses );

  // Spread the labels over the intensity range 
  unsigned int labelInterval = 1;
  if( m_UseNonContiguousLabels )
    {
    labelInterval = ( NumericTraits<OutputPixelType>::max() / numberOfClasses ) - 1;
    }

  unsigned int label = 0;


#ifdef ITK_USE_REVIEW_STATISTICS
  MembershipFunctionVectorType membershipFunctions;
#endif

  for(unsigned int k=0; k<numberOfClasses; k++)
    {
    classLabels[k] = label;
    label += labelInterval;
    MembershipFunctionPointer membershipFunction = MembershipFunctionType::New();
    MembershipFunctionOriginType origin( adaptor->GetMeasurementVectorSize() );
    origin[0] = this->m_FinalMeans[k]; // A scalar image has a MeasurementVector of dimension 1
#ifdef ITK_USE_REVIEW_STATISTICS
    membershipFunction->SetCentroid( origin );
    const MembershipFunctionType *constMembershipFunction = membershipFunction;
    membershipFunctions.push_back( constMembershipFunction );
#else
    membershipFunction->SetOrigin( origin );
    classifier->AddMembershipFunction( membershipFunction.GetPointer() );
#endif
    }

#ifdef ITK_USE_REVIEW_STATISTICS
  typename ClassifierType::MembershipFunctionVectorObjectPointer membershipFunctionsObject =
    ClassifierType::MembershipFunctionVectorObjectType::New();
  membershipFunctionsObject->Set(membershipFunctions); 
  classifier->SetMembershipFunctions(membershipFunctionsObject);

  typedef typename ClassifierType::ClassLabelVectorObjectType ClassLabelVectorObjectType;
  typename ClassLabelVectorObjectType::Pointer classLabelsObject = ClassLabelVectorObjectType::New();
  classLabelsObject->Set(classLabels);
  classifier->SetClassLabels( classLabelsObject );
#else
  classifier->SetMembershipFunctionClassLabels( classLabels );
#endif

  // Execute the actual classification

  classifier->Update();

  // Now classify the pixels
  typename OutputImageType::Pointer outputPtr = this->GetOutput();

  typedef ImageRegionIterator< OutputImageType >  ImageIterator;

  outputPtr->SetBufferedRegion( outputPtr->GetRequestedRegion() );
  outputPtr->Allocate();

  RegionType region = outputPtr->GetBufferedRegion();

  // If we constrained the classification to a region, label only pixels within
  // the region. Label outside pixels as numberOfClasses + 1
  if( m_ImageRegionDefined )
    {
    region = m_ImageRegion;
    }
        
  ImageIterator pixel( outputPtr, region );
  pixel.GoToBegin();

#ifdef ITK_USE_REVIEW_STATISTICS
  typedef typename ClassifierType::MembershipSampleType ClassifierOutputType;
#else
  typedef typename ClassifierType::OutputType  ClassifierOutputType;
#endif
      
  const ClassifierOutputType  * membershipSample = classifier->GetOutput();

  typedef typename ClassifierOutputType::ConstIterator LabelIterator;
    
  LabelIterator iter = membershipSample->Begin();
  LabelIterator end  = membershipSample->End();

  while ( iter != end )
    {
    pixel.Set( iter.GetClassLabel() );
    ++iter;
    ++pixel;
    }

  if( m_ImageRegionDefined )
    {
    // If a region is defined to constrain classification to, we need to label
    // pixels outside with numberOfClasses + 1. 
    typedef ImageRegionExclusionIteratorWithIndex< OutputImageType > 
      ExclusionImageIteratorType;
    ExclusionImageIteratorType exIt( outputPtr, outputPtr->GetBufferedRegion() );
    exIt.SetExclusionRegion( region );
    exIt.GoToBegin();
    if( m_UseNonContiguousLabels )
      {
      OutputPixelType outsideLabel = labelInterval * numberOfClasses;
      while( !exIt.IsAtEnd() )
        {
        exIt.Set( outsideLabel );
        ++exIt;
        }
      }
    else
      {
      while( !exIt.IsAtEnd() )
        {
        exIt.Set( numberOfClasses );
        ++exIt;
        }
      }
    }

}

/**
 * Add a new class for the classifier. This requires to explicitly set the
 * initial mean value for that class.
 */
template <class TInputImage, class TOutputImage >
void
VectorImageKmeansImageFilter<TInputImage, TOutputImage >
::AddClassWithInitialMean( RealPixelType mean )
{
  this->m_InitialMeans.push_back( mean );
}


/**
 * Standard "PrintSelf" method
 */
template <class TInputImage, class TOutputImage >
void
VectorImageKmeansImageFilter<TInputImage, TOutputImage >
::PrintSelf(
  std::ostream& os, 
  Indent indent) const
{
  Superclass::PrintSelf( os, indent );
  os << indent << "Final Means " << m_FinalMeans << std::endl;
  os << indent << "Use Contiguous Labels " << m_UseNonContiguousLabels << std::endl;
  os << indent << "Image Region Defined: " << m_ImageRegionDefined << std::endl;
  os << indent << "Image Region: " << m_ImageRegion << std::endl;
}

} // end namespace itk

#endif
