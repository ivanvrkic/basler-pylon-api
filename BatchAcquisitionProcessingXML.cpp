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
  \file   BatchAcquisitionProcessingXML.cpp
  \brief  Functions to parse XML configuration data.

  This file contains implementations of functions required to load XML configuration data.

  \author Tomislav Petkovic
  \date   2017-05-05
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONPROCSSINGXML_CPP
#define __BATCHACQUISITIONPROCSSINGXML_CPP


#include "BatchAcquisitionProcessingXML.h"



//! Process leaf node which contains a matrix.
/*!
  Processes leaf node which contains a matrix.

  \param pReader        Pointer to XMLLite reader object.
  \param tag_name       Name of the XML node to process.
  \param tag_depth      Starting depth of the XML node to process.
  \param matrix   Reference to matrix structure which needs to be extracted.
  \return Returns S_OK if successfull and error code otherwise.
*/
HRESULT
ProcessingXMLParseMatrix(
                         IXmlReader * pReader,
                         wchar_t const * const tag_name,
                         UINT const tag_depth,
                         XMLMatrix & matrix
                         )
{
  assert(NULL != pReader);
  if (NULL == pReader) return E_POINTER;

  assert(NULL != tag_name);
  if (NULL == tag_name) return E_POINTER;

  HRESULT hr = S_OK;

  // Test if starting node name matches.
  {
    WCHAR const * pName = NULL;
    hr = pReader->GetLocalName(&pName, NULL);
    assert(S_OK == hr);
    if (S_OK != hr) return E_ABORT;

    bool const is_match = (0 == _wcsicmp(tag_name, pName));
    assert(true == is_match);
    if (false == is_match) return E_ABORT;
  }

  size_t const n = matrix.ids.size();
  assert(matrix.rows.size() == n);
  if (matrix.rows.size() != n) return E_INVALIDARG;

  // Process nodes until end node is encountered or there are no more nodes to process.
  HRESULT have_node = S_OK;
  XmlNodeType nodeType = XmlNodeType_None;
  while (S_OK == have_node)
    {
      have_node = pReader->Read(&nodeType);
      if (S_OK != have_node) break;

      switch (nodeType)
        {
        case XmlNodeType_Element:
          {
            WCHAR const * pStartName = NULL;
            hr = pReader->GetLocalName(&pStartName, NULL);
            assert(S_OK == hr);

            UINT start_depth = 0;
            hr = pReader->GetDepth(&start_depth);
            assert(S_OK == hr);

            bool const is_name_match = ( 0 == _wcsicmp(matrix.name.c_str(), pStartName) );
            if (false == is_name_match) break;

            HRESULT const have_attribute = pReader->MoveToFirstAttribute();
            if ( !SUCCEEDED(have_attribute ) ) break;

            WCHAR const * pAttribute = NULL;
            hr = pReader->GetLocalName(&pAttribute, NULL);
            assert(S_OK == hr);

            WCHAR const * pValue = NULL;
            hr = pReader->GetValue(&pValue, NULL);
            assert( SUCCEEDED(hr) );

            bool const is_id_match = (0 == _wcsicmp(L"id", pAttribute));
            if (false == is_id_match) break;

            int const id = _wtof(pValue);
            assert( (1 <= id) && (id <= (int)n) );

            int const i_max = (int)n;
            for (int i = 0; i < i_max; ++i)
              {
                bool const is_match = (id == matrix.ids[i]);
                if (false == is_match) continue;

                bool break_while_loop = false;
                XmlNodeType inside_node_type = XmlNodeType_None;
                while ( S_OK == (have_node = pReader->Read(&inside_node_type)) )
                  {
                    switch (inside_node_type)
                      {
                      case XmlNodeType_Text:
                        {
                          WCHAR const * pValue = NULL;
                          hr = pReader->GetValue(&pValue, NULL);
                          assert(S_OK == hr);

                          int k = 0;
                          int const k_max = 1024;
                          wchar_t buffer[k_max + 1];
                          buffer[k_max] = 0;

                          for (int j = 0; 0 != pValue[j]; ++j)
                            {
                              if (k < k_max) buffer[k] = pValue[j];

                              if (wchar_t(',') == pValue[j])
                                {
                                  buffer[k] = 0;
                                  double const value = _wtof(buffer);
                                  matrix.rows[i].push_back(value);
                                  k = 0;
                                }
                              else
                                {
                                  ++k;
                                }
                              /* if */
                            }
                          /* for */

                          buffer[k] = 0;
                          double const value = _wtof(buffer);
                          matrix.rows[i].push_back(value);
                        }
                        break;

                      case XmlNodeType_EndElement:
                        {
                          WCHAR const * pEndName = NULL;
                          hr = pReader->GetLocalName(&pEndName, NULL);
                          assert(S_OK == hr);

                          UINT end_depth = 0;
                          hr = pReader->GetDepth(&end_depth);
                          assert(S_OK == hr);

                          break_while_loop = (start_depth + 1 == end_depth) && (0 == _wcsicmp(pStartName, pEndName));
                        }
                        break;
                      }
                    /* switch */

                    if (true == break_while_loop) break;
                  }
                /* while */

                break;

              }
            /* for */
          }
          break;

        case XmlNodeType_EndElement:
          {
            WCHAR const * pEndName = NULL;
            hr = pReader->GetLocalName(&pEndName, NULL);
            assert(S_OK == hr);

            UINT end_depth = 0;
            hr = pReader->GetDepth(&end_depth);
            assert(S_OK == hr);

            if ( (tag_depth + 1 == end_depth) && (0 == _wcsicmp(tag_name, pEndName)) )
              {
                // Return when end node is encountered.
                have_node = S_FALSE;
              }
            /* if */
          }
          break;
        }
      /* switch (nodeType) */
    }
  /* while (S_OK == have_node) */

  return hr;
}
/* ProcessingXMLParseMatrix */



//! Process leaf node.
/*!
  Processes all specified tags inside leaf nodes and extracts theirs numerical value.

  \param pReader        Pointer to XMLLite reader object.
  \param tag_name       Name of the XML node to process.
  \param tag_depth      Starting depth of the XML node to process.
  \param leaf     Reference to leaf structure which holds names of the variables to be extracted.
  \return Returns S_OK if successfull and error code otherwise.
*/
HRESULT
ProcessingXMLParseLeaf(
                       IXmlReader * const pReader,
                       wchar_t const * const tag_name,
                       UINT const tag_depth,
                       XMLLeaf & leaf
                       )
{
  assert(NULL != pReader);
  if (NULL == pReader) return E_POINTER;

  assert(NULL != tag_name);
  if (NULL == tag_name) return E_POINTER;

  HRESULT hr = S_OK;

  // Test if starting node name matches.
  {
    WCHAR const * pName = NULL;
    hr = pReader->GetLocalName(&pName, NULL);
    assert(S_OK == hr);
    if (S_OK != hr) return E_ABORT;

    bool const is_match = (0 == _wcsicmp(tag_name, pName));
    assert(true == is_match);
    if (false == is_match) return E_ABORT;
  }

  size_t const n = leaf.names.size();
  assert(leaf.values.size() == n);
  if (leaf.values.size() != n) return E_INVALIDARG;

  // Process nodes until end node is encountered or there are no more nodes to process.
  HRESULT have_node = S_OK;
  XmlNodeType nodeType = XmlNodeType_None;
  while (S_OK == have_node)
    {
      have_node = pReader->Read(&nodeType);
      if (S_OK != have_node) break;

      switch (nodeType)
        {
        case XmlNodeType_Element:
          {
            WCHAR const * pStartName = NULL;
            hr = pReader->GetLocalName(&pStartName, NULL);
            assert(S_OK == hr);

            UINT start_depth = 0;
            hr = pReader->GetDepth(&start_depth);
            assert(S_OK == hr);

            for (size_t i = 0; i < n; ++i)
              {
                bool const is_match = ( 0 == _wcsicmp(leaf.names[i].c_str(), pStartName) );
                if (false == is_match) continue;

                bool break_while_loop = false;
                XmlNodeType inside_node_type = XmlNodeType_None;
                while ( S_OK == (have_node = pReader->Read(&inside_node_type)) )
                  {
                    switch (inside_node_type)
                      {
                      case XmlNodeType_Text:
                        {
                          WCHAR const * pValue = NULL;
                          hr = pReader->GetValue(&pValue, NULL);
                          assert(S_OK == hr);

                          double const value = _wtof(pValue);
                          assert( isnan_inline(leaf.values[i]) );
                          leaf.values[i] = value;
                        }
                        break;

                      case XmlNodeType_EndElement:
                        {
                          WCHAR const * pEndName = NULL;
                          hr = pReader->GetLocalName(&pEndName, NULL);
                          assert(S_OK == hr);

                          UINT end_depth = 0;
                          hr = pReader->GetDepth(&end_depth);
                          assert(S_OK == hr);

                          break_while_loop = (start_depth + 1 == end_depth) && (0 == _wcsicmp(pStartName, pEndName));
                        }
                        break;
                      }
                    /* switch */

                    if (true == break_while_loop) break;
                  }
                /* while */

                break;
              }
            /* for */
          }
          break;

        case XmlNodeType_EndElement:
          {
            WCHAR const * pEndName = NULL;
            hr = pReader->GetLocalName(&pEndName, NULL);
            assert(S_OK == hr);

            UINT end_depth = 0;
            hr = pReader->GetDepth(&end_depth);
            assert(S_OK == hr);

            if ( (tag_depth + 1 == end_depth) && (0 == _wcsicmp(tag_name, pEndName)) )
              {
                // Return when end node is encountered.
                have_node = S_FALSE;
              }
            /* if */
          }
          break;
        }
      /* switch (nodeType) */
    }
  /* while (S_OK == have_node) */

  return hr;
}
/* ProcessingXMLParseLeaf */



//! Process branch node.
/*!
  Processes all leafs inside a branch node.

  \param pReader        Pointer to XMLLite reader object.
  \param tag_name       Name of the XML node to process.
  \param tag_depth      Starting depth of the XML node to process.
  \param branch     Reference to branch structure which holds all leaves to be processed.
  \return Returns S_OK if successfull and error code otherwise.
*/
HRESULT
ProcessingXMLParseBranch(
                         IXmlReader * pReader,
                         wchar_t const * const tag_name,
                         UINT const tag_depth,
                         XMLBranch & branch
                         )
{
  assert(NULL != pReader);
  if (NULL == pReader) return E_POINTER;

  assert(NULL != tag_name);
  if (NULL == tag_name) return E_POINTER;

  HRESULT hr = S_OK;

  // Test if starting node name matches.
  {
    WCHAR const * pName = NULL;
    hr = pReader->GetLocalName(&pName, NULL);
    assert(S_OK == hr);
    if (S_OK != hr) return E_ABORT;

    bool const is_match = (0 == _wcsicmp(tag_name, pName));
    assert(true == is_match);
    if (false == is_match) return E_ABORT;
  }

  // Process nodes until end node is encountered or there are no more nodes to process.
  HRESULT have_node = S_OK;
  XmlNodeType nodeType = XmlNodeType_None;
  while (S_OK == have_node)
    {
      have_node = pReader->Read(&nodeType);
      if (S_OK != have_node) break;

      switch (nodeType)
        {
        case XmlNodeType_Element:
          {
            WCHAR const * pLeafName = NULL;
            hr = pReader->GetLocalName(&pLeafName, NULL);
            assert(S_OK == hr);

            UINT leaf_depth = 0;
            hr = pReader->GetDepth(&leaf_depth);
            assert(S_OK == hr);

            for(std::vector<XMLLeaf>::iterator it = branch.leaves.begin(); it != branch.leaves.end(); ++it)
              {
                bool const is_match = (0 == _wcsicmp(it->name.c_str(), pLeafName) );
                if (false == is_match) continue;

                hr = ProcessingXMLParseLeaf(pReader, it->name.c_str(), leaf_depth, *it);
                assert( SUCCEEDED(hr) );

                break;
              }
            /* for */
          }
          break;

        case XmlNodeType_EndElement:
          {
            WCHAR const * pEndName = NULL;
            hr = pReader->GetLocalName(&pEndName, NULL);
            assert(S_OK == hr);

            UINT end_depth = 0;
            hr = pReader->GetDepth(&end_depth);
            assert(S_OK == hr);

            if ( (tag_depth + 1 == end_depth) && (0 == _wcsicmp(tag_name, pEndName)) )
              {
                // Return when end node is encountered.
                have_node = S_FALSE;
              }
            /* if */
          }
          break;
        }
      /* switch (nodeType) */
    }
  /* while (S_OK == have_node) */

  return hr;
}
/* ProcessingXMLParseBranch */



//! Process resolution.
/*!
  Processes resolution.

  \param pReader        Pointer to XMLLite reader object.
  \param tag_depth      Starting depth of the XML node to process.
  \param resParam      Address of memory space which is sufficient to store three double precision values.
  First two values are width and height and third value is refresh rate.
  \return Returns S_OK if successfull and error code otherwise.
*/
HRESULT
ProcessingXMLParseResolution(
                             IXmlReader * pReader,
                             UINT const tag_depth,
                             double * const resParam
                             )
{
  assert(NULL != pReader);
  if (NULL == pReader) return E_POINTER;

  assert(NULL != resParam);
  if (NULL == resParam) return E_POINTER;

  HRESULT hr = S_OK;

  // Test if starting node name matches.
  wchar_t const tag_name[] = L"resolution";
  {
    WCHAR const * pName = NULL;
    hr = pReader->GetLocalName(&pName, NULL);
    assert(S_OK == hr);
    if (S_OK != hr) return E_ABORT;

    bool const is_match = (0 == _wcsicmp(tag_name, pName));
    assert(true == is_match);
    if (false == is_match) return E_ABORT;
  }

  // Declare tag names.
  XMLLeaf resolution;
  resolution.name = tag_name;
  resolution.names.push_back(L"width");  resolution.values.push_back(std::numeric_limits<double>::quiet_NaN());
  resolution.names.push_back(L"height");  resolution.values.push_back(std::numeric_limits<double>::quiet_NaN());
  resolution.names.push_back(L"frequency");  resolution.values.push_back(std::numeric_limits<double>::quiet_NaN());

  hr = ProcessingXMLParseLeaf(pReader, tag_name, tag_depth, resolution);
  assert( SUCCEEDED(hr) );

  if ( SUCCEEDED(hr) )
    {
      resParam[0] = resolution.values[0];
      resParam[1] = resolution.values[1];
      resParam[2] = resolution.values[2];
    }
  else
    {
      resParam[0] = std::numeric_limits<double>::quiet_NaN();
      resParam[1] = std::numeric_limits<double>::quiet_NaN();
      resParam[2] = std::numeric_limits<double>::quiet_NaN();
    }
  /* if */

  return hr;
}
/* ProcessingXMLParseResolution */



//! Process geometry intrinsics.
/*!
  Processes geometry intrinsics.

  \param pReader        Pointer to XMLLite reader object.
  \param tag_depth      Starting depth of the XML node to process.
  \param intParam      Address of memory space which is sufficient to store six double precision values.
  First two values are focal length, next two values are camera principle point, and last two values are radial distortion parameters.
  \return Returns S_OK if successfull and error code otherwise.
*/
HRESULT
ProcessingXMLParseIntrinsics(
                             IXmlReader * pReader,
                             UINT const tag_depth,
                             double * const intParam
                             )
{
  assert(NULL != pReader);
  if (NULL == pReader) return E_POINTER;

  assert(NULL != intParam);
  if (NULL == intParam) return E_POINTER;

  HRESULT hr = S_OK;

  // Test if starting node name matches.
  wchar_t const tag_name[] = L"intrinsics";
  {
    WCHAR const * pName = NULL;
    hr = pReader->GetLocalName(&pName, NULL);
    assert(S_OK == hr);
    if (S_OK != hr) return E_ABORT;

    bool const is_match = (0 == _wcsicmp(tag_name, pName));
    assert(true == is_match);
    if (false == is_match) return E_ABORT;
  }

  // Declare tag names.
  XMLLeaf focus;
  focus.name = L"focus";
  focus.names.push_back(L"x");  focus.values.push_back(std::numeric_limits<double>::quiet_NaN());
  focus.names.push_back(L"y");  focus.values.push_back(std::numeric_limits<double>::quiet_NaN());

  XMLLeaf center;
  center.name = L"center";
  center.names.push_back(L"x");  center.values.push_back(std::numeric_limits<double>::quiet_NaN());
  center.names.push_back(L"y");  center.values.push_back(std::numeric_limits<double>::quiet_NaN());

  XMLLeaf skew;
  skew.name = L"skew";
  skew.names.push_back(L"s");  skew.values.push_back(std::numeric_limits<double>::quiet_NaN());

  XMLLeaf distortion;
  distortion.name = L"distortion";
  distortion.names.push_back(L"k0");  distortion.values.push_back(std::numeric_limits<double>::quiet_NaN());
  distortion.names.push_back(L"k1");  distortion.values.push_back(std::numeric_limits<double>::quiet_NaN());

  XMLBranch intrinsics;
  intrinsics.leaves.push_back(focus);
  intrinsics.leaves.push_back(center);
  intrinsics.leaves.push_back(skew);
  intrinsics.leaves.push_back(distortion);

  hr = ProcessingXMLParseBranch(pReader, tag_name, tag_depth, intrinsics);
  assert( SUCCEEDED(hr) );

  if ( SUCCEEDED(hr) )
    {
      intParam[0] = intrinsics.leaves[0].values[0];
      intParam[1] = intrinsics.leaves[0].values[1];
      intParam[2] = intrinsics.leaves[1].values[0];
      intParam[3] = intrinsics.leaves[1].values[1];
      intParam[4] = intrinsics.leaves[3].values[0];
      intParam[5] = intrinsics.leaves[3].values[1];
    }
  else
    {
      intParam[0] = std::numeric_limits<double>::quiet_NaN();
      intParam[1] = std::numeric_limits<double>::quiet_NaN();
      intParam[2] = std::numeric_limits<double>::quiet_NaN();
      intParam[3] = std::numeric_limits<double>::quiet_NaN();
      intParam[4] = std::numeric_limits<double>::quiet_NaN();
      intParam[5] = std::numeric_limits<double>::quiet_NaN();
    }
  /* if */

  return hr;
}
/* ProcessingXMLParseIntrinsics */



//! Process geometry extrinsics.
/*!
  Processes geometry extrinsics.

  \param pReader        Pointer to XMLLite reader object.
  \param tag_depth      Starting depth of the XML node to process.
  \param extParam      Address of memory space which is sufficient to store six double precision values.
  First three values are rotation angles and next three values are camera center.
  \return Returns S_OK if successfull and error code otherwise.
*/
HRESULT
ProcessingXMLParseExtrinsics(
                             IXmlReader * pReader,
                             UINT const tag_depth,
                             double * const extParam
                             )
{
  assert(NULL != pReader);
  if (NULL == pReader) return E_POINTER;

  assert(NULL != extParam);
  if (NULL == extParam) return E_POINTER;

  HRESULT hr = S_OK;

  // Test if starting node name matches.
  wchar_t const tag_name[] = L"extrinsics";
  {
    WCHAR const * pName = NULL;
    hr = pReader->GetLocalName(&pName, NULL);
    assert(S_OK == hr);
    if (S_OK != hr) return E_ABORT;

    bool const is_match = (0 == _wcsicmp(tag_name, pName));
    assert(true == is_match);
    if (false == is_match) return E_ABORT;
  }

  // Declare tag names.
  XMLLeaf rotation;
  rotation.name = L"rotation";

  XMLLeaf center;
  center.name = L"center";
  center.names.push_back(L"x");  center.values.push_back(std::numeric_limits<double>::quiet_NaN());
  center.names.push_back(L"y");  center.values.push_back(std::numeric_limits<double>::quiet_NaN());
  center.names.push_back(L"z");  center.values.push_back(std::numeric_limits<double>::quiet_NaN());

  XMLBranch extrinsics;
  extrinsics.leaves.push_back(rotation);
  extrinsics.leaves.push_back(center);

  hr = ProcessingXMLParseBranch(pReader, tag_name, tag_depth, extrinsics);
  assert( SUCCEEDED(hr) );

  if ( SUCCEEDED(hr) )
    {
      extParam[0] = std::numeric_limits<double>::quiet_NaN();
      extParam[1] = std::numeric_limits<double>::quiet_NaN();
      extParam[2] = std::numeric_limits<double>::quiet_NaN();
      extParam[3] = extrinsics.leaves[1].values[0];
      extParam[4] = extrinsics.leaves[1].values[1];
      extParam[5] = extrinsics.leaves[1].values[2];
    }
  else
    {
      extParam[0] = std::numeric_limits<double>::quiet_NaN();
      extParam[1] = std::numeric_limits<double>::quiet_NaN();
      extParam[2] = std::numeric_limits<double>::quiet_NaN();
      extParam[3] = std::numeric_limits<double>::quiet_NaN();
      extParam[4] = std::numeric_limits<double>::quiet_NaN();
      extParam[5] = std::numeric_limits<double>::quiet_NaN();
    }
  /* if */

  return hr;
}
/* ProcessingXMLParseExtrinsics */



//! Process projection matrix.
/*!
  Processes geometry extrinsics.

  \param pReader        Pointer to XMLLite reader object.
  \param tag_depth      Starting depth of the XML node to process.
  \param proMatrix      Address of memory space which is sufficient to store twelve double precision values.
  Twelve values are 3x4 projection matrix stored row-wise.
  \return Returns S_OK if successfull and error code otherwise.
*/
HRESULT
ProcessingXMLParseProjectionMatrix(
                                   IXmlReader * pReader,
                                   UINT const tag_depth,
                                   double * const proMatrix
                                   )
{
  assert(NULL != pReader);
  if (NULL == pReader) return E_POINTER;

  assert(NULL != proMatrix);
  if (NULL == proMatrix) return E_POINTER;

  HRESULT hr = S_OK;

  // Test if starting node name matches.
  wchar_t const tag_name[] = L"projection_matrix";
  {
    WCHAR const * pName = NULL;
    hr = pReader->GetLocalName(&pName, NULL);
    assert(S_OK == hr);
    if (S_OK != hr) return E_ABORT;

    bool const is_match = (0 == _wcsicmp(tag_name, pName));
    assert(true == is_match);
    if (false == is_match) return E_ABORT;
  }

  // Declare tag names.
  std::vector<double> row;
  row.reserve(4);

  XMLMatrix matrix;
  matrix.name = L"row";
  matrix.ids.push_back(1);  matrix.rows.push_back(row);
  matrix.ids.push_back(2);  matrix.rows.push_back(row);
  matrix.ids.push_back(3);  matrix.rows.push_back(row);

  hr = ProcessingXMLParseMatrix(pReader, tag_name, tag_depth, matrix);
  assert( SUCCEEDED(hr) );

  if ( SUCCEEDED(hr) )
    {
      for (int j = 0; j < 3; ++j)
        {
          int const n = (int)( matrix.rows[j].size() );
          for (int i = 0; (i < 4) && (i < n); ++i) proMatrix[i + 4*j] = matrix.rows[j][i];
        }
      /* for */
    }
  else
    {
      proMatrix[ 0] = std::numeric_limits<double>::quiet_NaN();
      proMatrix[ 1] = std::numeric_limits<double>::quiet_NaN();
      proMatrix[ 2] = std::numeric_limits<double>::quiet_NaN();
      proMatrix[ 3] = std::numeric_limits<double>::quiet_NaN();
      proMatrix[ 4] = std::numeric_limits<double>::quiet_NaN();
      proMatrix[ 5] = std::numeric_limits<double>::quiet_NaN();
      proMatrix[ 6] = std::numeric_limits<double>::quiet_NaN();
      proMatrix[ 7] = std::numeric_limits<double>::quiet_NaN();
      proMatrix[ 8] = std::numeric_limits<double>::quiet_NaN();
      proMatrix[ 9] = std::numeric_limits<double>::quiet_NaN();
      proMatrix[10] = std::numeric_limits<double>::quiet_NaN();
      proMatrix[11] = std::numeric_limits<double>::quiet_NaN();
    }
  /* if */

  return hr;
}
/* ProcessingXMLParseProjectionMatrix */



#endif /* !__BATCHACQUISITIONPROCSSINGXML_CPP */
