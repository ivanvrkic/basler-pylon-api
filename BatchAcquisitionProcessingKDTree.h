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
  \file   BatchAcquisitionProcessingKDTree.h
  \brief  Simple KD tree.

  A simple KD tree for 1NN search.

  \author Tomislav Petkovic
  \date   2017-06-06
*/



#ifndef __BATCHACQUISITIONKDTREE_H
#define __BATCHACQUISITIONKDTREE_H


/* Predeclare typedefs. */
struct KDTreeRoot_;
struct KDTreeNode_;
struct KDTreeClosestPoint_;


/* Define node types. */
#define KDTREE_NODE_BRANCH 0
#define KDTREE_NODE_LEAF 1
#define KDTREE_NODE_UNDEFINED -1



//! Class to store KD tree.
/*!
  This class stores KD tree data and the pointer KD tree itself.

  The KD tree is implemented using different class KDTreeNode.
  Note that each KD tree node holds the pointer to KDTreeRoot class so
  variables that are the same for all nodes may be stored in KDTreeRoot;
  these include statistics about the tree, number and dimensionality of data points,
  pointer to memory where data points are stored, temporary storage used during tree construction etc.
*/
typedef
struct KDTreeRoot_
{
  KDTreeNode_ * tree; //!< Root of the KD tree.

  int num_nodes; //!< Total number of nodes in the KD tree.
  int num_branches; //!< Total number of branches in the KD tree.
  int num_leaves; //!< Total number of leaf nodes in the KD tree.
  int max_depth; //!< Maximal depth of the KD tree.

  double min_half_dst2; //!< Minimal squared half-distance between any two data elements.
  double min_half_dst; //!< Minimal half-distance between any two data elements.

  int n_dim; //!< Number of dimensions.
  int n_pts; //!< Number of elements in the data matrix.
  int data_stride; //!< Size of one element in bytes.

  double const * data; //!< Pointer to data matrix which stores all elements. Storage is externally allocated.

  int * sorted_indices; //!< Lists of temporary indices used in tree construction. Size of storage must fit row * (col + 1) elements.

  //! Constructor.
  KDTreeRoot_();

  //! Destructor.
  ~KDTreeRoot_();

  //! Blank class variables.
  void Blank(void);

  //! Delete KD tree.
  void DeleteTree(void);

  //! Compare vectors.
  double CompareVectors(int const, int const, int const);

  //! Compare vectors.
  double CompareVectors(double const * const, double const * const, int const);

  //! Merge sort.
  void MergeSort(int const, int const, int const);

  //! Minimal distance.
  double MinimalSquaredHalfDistance(void);

  //! Construct KD tree.
  bool ConstructTree(double const * const, int const, int const, int const);

  //! Find nearest neighbour.
  bool Find1NN(KDTreeClosestPoint_ &);

  //! Check if test point is closer than limit distance.
  bool Check1NN(KDTreeClosestPoint_ &);

} KDTreeRoot;



//! Structure to store one node of KD tree.
/*!
  This structure stores one node of a KD tree.
*/
typedef
struct KDTreeNode_
{
  KDTreeRoot_ * root; //!< Pointer to root node.
  KDTreeNode_ * less_than_pivot; //!< Pointer to subtree indicating elements lower than pivot.
  KDTreeNode_ * equal_to_or_greater_than_pivot; //!< Pointer to subtree indicating elements equal to or greater than pivot.

  double pivot; //!< Pivot value.

  int row_idx; //!< Element row index. May be -1 if there is no element assocaited with this node.

  short axis; //!< Axis index of this node.
  short depth; //!< Node depth.

  signed char type; //!< Variable which indicates if the node type; e.g. pivot or leaf node.

  //! Constructor.
  KDTreeNode_();

  //! Constructor.
  KDTreeNode_(KDTreeRoot * const, int const, int const, int const, signed char const);

  //! Destructor.
  ~KDTreeNode_();

  //! Blank class variables.
  void Blank(void);

  //! Recursively delete KD tree.
  void DeleteTree(void);

  //! Recursively create KD tree.
  static KDTreeNode_ * ConstructTree(KDTreeRoot * const, int const, int const, int const);

  //! Recursively verify KD tree.
  bool VerifyTree(int const);

  //! Find nearest neighbour.
  bool Find1NN(KDTreeClosestPoint_ &, int const);

} KDTreeNode;



//! Structure to store closest neighbour.
/*!
  Structure to store closest neighbour from a KDTree to a query point.
  Structure holds the pointer to query point which cannot be NULL.
  Structure also stores a point from the KDTree which is a closest match
  to the query point; note that NULL pointer, index -1, and distance Inf indicate
  uninitialized closest point.
*/
typedef
struct KDTreeClosestPoint_
{
  double const * query; //!< Pointer to the query point.
  double const * value; //!< Pointer to the closest point.
  double dst2; //!< Squared distance from the query point to the closest point.
  int idx;  //!< Index of closest point in the dataset.
  bool found_best; //!< Flag which indicates best match was found.

  //! Constructor.
  KDTreeClosestPoint_();

  //! Clear closest point.
  void Clear(void);

  //! Clear closest point.
  void ClearAllButIndex(void);

} KDTreeClosestPoint;



#endif /* !__BATCHACQUISITIONKDTREE_H */
