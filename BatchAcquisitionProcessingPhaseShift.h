/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2015-2021 UniZG, Zagreb. All rights reserved.
 * (c) 2015-2021 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionProcessingPhaseShift.h
  \brief  Phase estimation methods.

  Functions for phase estimation including multiple phase-shifting
  and a combination of Gray code and phase shifting.

  \author Tomislav Petkovic, Tomislav Pribanic
  \date   2021-04-20
*/


#ifndef __BATCHACQUISITIONPROCESSINGPHASESHIFT_H
#define __BATCHACQUISITIONPROCESSINGPHASESHIFT_H


#include "BatchAcquisitionProcessing.h"
#include "BatchAcquisitionProcessingKDTree.h"


/****** RELATIVE PHASE ESTIMATION ******/

//! Relative phase estimation (single precision).
cv::Mat * EstimateRelativePhaseSingle(ImageSet * const, int const, int const);

//! Relative phase estimation (double precision).
cv::Mat * EstimateRelativePhase(ImageSet * const, int const, int const);


/****** GRAY CODE DECODING ******/

//! Computes Gray code weights (double precision).
double * CreateGrayCodeWeights(int const);

//! Deletes Gray code weights (double precision).
void DeleteGrayCodeWeights(double * const);

//! Decodes Gray code (double precision).
cv::Mat * DecodeGrayCode(ImageSet * const, int const, int const);


/****** ABSOLUTE PHASE ESTIMATION USING GC+PS ******/

//! Unwraps phase using Gray code.
cv::Mat * UnwrapPhasePSAndGC(
                             ImageSet * const,
                             int const,
                             int const,
                             int const,
                             int const,
                             int const,
                             int const,
                             cv::Mat * const,
                             cv::Mat * * const,
                             cv::Mat * * const
                             );



/****** ABSOLUTE PHASE ESTIMATION USING MPS ******/

//! Compute greatest common divisor of two numbers.
double mps_gcd(double const, double const);

//! Compute greatest common divisor of many numbers.
double mps_gcd(std::vector<double> const &);

//! Compute least common multiple of two numbers.
double mps_lcm(double const, double const);

//! Compute least common multiple of many numbers.
double mps_lcm(std::vector<double> const &);

//! Return periods from fringe counts.
bool
mps_periods_from_fringe_counts(
                               std::vector<double> const &,
                               double const,
                               std::vector<double> * * const,
                               double * const
                               );

//! Computes all valid period tuples.
bool
mps_get_period_tuples(
                      std::vector<double> const &,
                      double const,
                      cv::Mat * * const,
                      double * const,
                      cv::Mat * * const,
                      cv::Mat * * const
                      );

//! Return line equations for chosen set of wavelengths.
bool
mps_get_lines(
              std::vector<double> const &,
              double const,
              cv::Mat * * const,
              cv::Mat * * const,
              cv::Mat * * const,
              double * const,
              cv::Mat * * const,
              cv::Mat * * const
              );

//! Return wrapped tupples.
bool
mps_get_wrapped_tuples(
                       std::vector<double> const &,
                       double const,
                       cv::Mat * * const,
                       double * const,
                       cv::Mat * * const
                       );

//! Returns ortographic projection matrix and projected tuple center points.
bool
mps_get_projection_matrix_and_centers(
                                      std::vector<double> const &,
                                      double const,
                                      cv::Mat * * const,
                                      cv::Mat * * const,
                                      cv::Mat * * const,
                                      cv::Mat * * const,
                                      cv::Mat * * const,
                                      cv::Mat * * const,
                                      double * const
                                      );

//! Construct KD tree.
bool
mps_get_kd_tree(
                cv::Mat * const,
                cv::Mat * const,
                cv::Mat * const,
                cv::Mat * const,
                cv::Mat * * const,
                cv::Mat * * const,
                std::vector<int> * * const,
                KDTreeRoot_ * * const
                );

//! Return standard weights.
bool
mps_get_weights(
                std::vector<double> const &,
                std::vector<double> * * const
                );

//! Unwraps phase using orthographic projection.
bool
mps_unwrap_phase(
                 std::vector<cv::Mat *> const &,
                 cv::Mat * const,
                 cv::Mat * const,
                 cv::Mat * const,
                 KDTreeRoot * const,
                 std::vector<double> const &,
                 std::vector<double> const &,
                 cv::Mat * * const,
                 cv::Mat * * const,
                 cv::Mat * * const
                 );


/****** PHASE STATISTICS ON SLIDING WINDOW ******/

//! Compute statistics.
bool
GetAbsolutePhaseOrderAndDeviation(
                                  cv::Mat * const,
                                  int const,
                                  int const,
                                  cv::Mat * * const,
                                  cv::Mat * * const
                                  );

//! Combine phase deviation or distance (single precision).
cv::Mat *
CombinePhaseDeviationOrDistance(
                                cv::Mat const * const,
                                cv::Mat const * const
                                );



#endif /* !__BATCHACQUISITIONPROCESSINGPHASESHIFT_H */
