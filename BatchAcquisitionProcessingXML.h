/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2017 UniZG, Zagreb. All rights reserved.
 * (c) 2017 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionProcessingXML.h
  \brief  Functions to parse XML configuration data.

  This file contains headers of functions required to load XML configuration data.

  \author Tomislav Petkovic
  \date   2017-05-05
*/


#ifndef __BATCHACQUISITIONPROCSSINGXML_H
#define __BATCHACQUISITIONPROCSSINGXML_H


#include "BatchAcquisition.h"


//! Leaf geometry node.
/*!
  Leaf geometry node contains XML tags with number only. Each XML tag has its name and units.
  This structure holds the name of the leaf node and names of all elements in that leaf.
*/
typedef
struct XMLLeaf_
{
  std::wstring name; /*!< Leaf name. */
  std::vector<std::wstring> names; /*!< Names of elements in a leaf. */
  std::vector<double> values; /*!< Values of elements in a leaf*/
} XMLLeaf;



//! Matrix geometry node.
/*!
  We store matrices in XML rowwise so each row has the same tag and different row id.
*/
typedef
struct XMLMatrix_
{
  std::wstring name; /*!< Row name. */
  std::vector<unsigned int> ids; /*!< Id attributes to read. */
  std::vector<std::vector<double>> rows; /*!< Vector of matrix rows. */
} XMLMatrix;



//! Branch geometry node.
/*!
  Branch geometry node contains multiple leaf nodes. This structure holds all leaf nodes in one branch.
*/
typedef
struct XMLBranch_
{
  std::vector<XMLLeaf> leaves; /*!< Leaves in a branch.*/
  std::vector<XMLMatrix> matrices; /*!< Matrices in a branch. */
} XMLBranch;



//! Process leaf node which contains a matrix.
HRESULT
ProcessingXMLParseMatrix(
                         IXmlReader *,
                         wchar_t const * const,
                         UINT const,
                         XMLMatrix &
                         );

//! Process leaf node.
HRESULT
ProcessingXMLParseLeaf(
                       IXmlReader * const,
                       wchar_t const * const,
                       UINT const,
                       XMLLeaf &
                       );

//! Process branch node.
HRESULT
ProcessingXMLParseBranch(
                         IXmlReader * const,
                         wchar_t const * const,
                         UINT const,
                         XMLBranch &
                         );

//! Process resolution.
HRESULT
ProcessingXMLParseResolution(
                             IXmlReader *,
                             UINT const,
                             double * const
                             );

//! Process geometry intrinsics.
HRESULT
ProcessingXMLParseIntrinsics(
                             IXmlReader *,
                             UINT const,
                             double * const
                             );

//! Process geometry extrinsics.
HRESULT
ProcessingXMLParseExtrinsics(
                             IXmlReader *,
                             UINT const,
                             double * const
                             );

//! Process projection matrix.
HRESULT
ProcessingXMLParseProjectionMatrix(
                                   IXmlReader *,
                                   UINT const,
                                   double * const
                                   );


#endif /* !__BATCHACQUISITIONPROCSSINGXML_H */
