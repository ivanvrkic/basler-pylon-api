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
  \file   BatchAcquisitionProcessingKDTree.cpp
  \brief  Simple KD tree.

  This files contains implementation of a simple KD tree.
  Implementation mostly follows one described in the article
  Building a Balanced k-d Tree in O(kn log n) Time by Russell A. Brown
  (http://jcgt.org/published/0004/01/03/).

  \author Tomislav Petkovic
  \date   2017-06-06
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONKDTREE_CPP
#define __BATCHACQUISITIONKDTREE_CPP


#include "BatchAcquisitionProcessingKDTree.h"
#include "BatchAcquisition.h"


#pragma intrinsic(sqrt)



/****** INLINE HELPER FUNCTIONS ******/

//! Squared distance between two elements.
/*!
  Functions computes squared Euclidean distance between two vectors.

  \param a    Address of the first vector.
  \param b      Address of the second vector.
  \param dim   Number of dimensions.
  \return Returns squared Euclidean distance between two vectors.
*/
template <class T>
inline
T
SquaredDistance_inline(
                       T const * const a,
                       T const * const b,
                       int const dim
                       )
{
  assert( (NULL != a) && (NULL != b) );

  T dst2 = 0;
  for (int i = 0; i < dim; ++i)
    {
      T const diff = a[i] - b[i];
      dst2 += diff * diff;
    }
  /* for */

  return dst2;
}
/* SquaredDistance_inline */



/****** KD TREE ROOT ******/

//! Constructor.
/*!
  Creates KDTreeRoot structure.
  Note that KD tree itself is not created; call ConstructTree function to create the tree.
*/
KDTreeRoot_::KDTreeRoot_()
{
  this->Blank();
}
/* KDTreeRoot_::KDTreeRoot_ */



//! Destructor.
/*!
  Deletes KD tree structure.
*/
KDTreeRoot_::~KDTreeRoot_()
{
  this->DeleteTree();
  this->Blank();
}
/* KDTreeRoot_::~KDTreeRoot_*/



//! Blank class variables.
/*!
  Initializes all class variables.
*/
void
KDTreeRoot_::Blank(
                   void
                   )
{
  this->tree = NULL;

  this->num_nodes = 0;
  this->num_branches = 0;
  this->num_leaves = 0;
  this->max_depth = 0;

  this->min_half_dst2 = 0.0;
  this->min_half_dst = 0.0;

  this->n_dim = 0;
  this->n_pts = 0;
  this->data_stride = 0;

  this->data = NULL;
  this->sorted_indices = NULL;
}
/* KDTreeRoot_::Blank */



//! Delete KD tree.
/*!
  Deletes KD tree and associated data if one exists.
*/
void
KDTreeRoot_::DeleteTree(
                        void
                        )
{
  SAFE_DELETE(this->tree);
  SAFE_FREE(this->sorted_indices);
}
/* KDTreeRoot_::DeleteTree */



//! Compare vectors.
/*!
  Function compares two vectors from the storage this->data.
  As each vector has n_dim dimensions the comparison starts from the dimension indicated by the input value dim.

  \param A      Row index of the first vector.
  \param B      Row index of the second vector.
  \param axis    Starting axis for comparison.
  \return Returns 0 if vectors are equal, a negative value if A is before B, and a positive value if A is after B.
*/
double
KDTreeRoot_::CompareVectors(
                            int const A,
                            int const B,
                            int const axis
                            )
{
  assert( (0 <= A) && (A < this->n_pts) );
  assert( (0 <= B) && (B < this->n_pts) );

  double const * const row_A = (double const *)( (BYTE const *)(this->data) + A * this->data_stride );
  double const * const row_B = (double const *)( (BYTE const *)(this->data) + B * this->data_stride );

  double diff = 0.0;

  int const n_dim = this->n_dim;
  assert( (0 <= axis) && (axis < n_dim) );

  for (int i = 0; i < n_dim; ++i)
    {
      int idx = i + axis;
      if (idx >= n_dim) idx -= n_dim;
      assert( (0 <= idx) && (idx < n_dim) );

      diff = row_A[idx] - row_B[idx];

      if (0.0 != diff) return diff;
    }
  /* for */

  return diff;
}
/* KDTreeRoot_::CompareVectors */



//! Compare vectors.
/*!
  Function compares two vectors from the storage this->data.
  As each vector has n_dim dimensions the comparison starts from the dimension indicated by the input value dim.

  \param row_A      Pointer to the first vector.
  \param row_B      Pointer to the second vector.
  \param axis    Starting axis for comparison.
  \return Returns 0 if vectors are equal, a negative value if A is before B, and a positive value if A is after B.
*/
double
KDTreeRoot_::CompareVectors(
                            double const * const row_A,
                            double const * const row_B,
                            int const axis
                            )
{
  double diff = 0.0;

  int const n_dim = this->n_dim;
  assert( (0 <= axis) && (axis < n_dim) );

  for (int i = 0; i < n_dim; ++i)
    {
      int idx = i + axis;
      if (idx >= n_dim) idx -= n_dim;
      assert( (0 <= idx) && (idx < n_dim) );

      diff = row_A[idx] - row_B[idx];

      if (0.0 != diff) return diff;
    }
  /* for */

  return diff;
}
/* KDTreeRoot_::CompareVectors */



//! Merge sort.
/*!
  Performes merge sort on indices stored in this->sorted_indices along the selected dimension dim.
  Indices are ordered so they indicate values in this->data in ascending order.

  \param low    Starting index of the region to sort (element included).
  \param high   Ending index of the region to sort (element included).
  \param axis    Sorting axis.
*/
void
KDTreeRoot_::MergeSort(
                       int const low,
                       int const high,
                       int const axis
                       )
{
  assert( (0 <= low) && (low < this->n_pts) );
  assert( (0 <= high) && (high < this->n_pts) );
  assert( (0 <= axis) && (axis < this->n_dim) );

  // Return immediately if there is nothing to split and merge.
  if (low >= high) return;

  // Return immediately if there is no temporary storage.
  assert(NULL != this->sorted_indices);
  if (NULL == this->sorted_indices) return;

  // Temporary column is located after all axes.
  int const tmp = this->n_dim;

  // Select pivot in the middle.
  int const mid = low + (high - low) / 2;

  // Recursively subdivide and process lower and upper subrange.
  MergeSort(low, mid, axis);
  MergeSort(mid + 1, high, axis);

  int const step_indices = this->n_dim + 1;

  // Copy sorted data from the lower subrange into temporary column in normal order.
  for (int i = low; i <= mid; ++i)
    {
      int * const row_srcdst = this->sorted_indices + step_indices * i;
      row_srcdst[tmp] = row_srcdst[axis];
    }
  /* for */

  // Copy sorted data from the upper subrage into temporary column in reverse order.
  int const offset = mid + high + 1;
  for (int j = mid + 1; j <= high; ++j)
    {
      int const * const row_src = this->sorted_indices + step_indices * j;
      int       * const row_dst = this->sorted_indices + step_indices * (offset - j);
      row_dst[tmp] = row_src[axis];
    }
  /* for */

  // Merge results by copying from the temporary column back into the active column.
  int i_low = low;
  int i_high = high;
  for (int k = low; k <= high; ++k)
    {
      int * const row_dst = this->sorted_indices + step_indices * k;

      int const * const row_src_low = this->sorted_indices + step_indices * i_low;
      int const * const row_src_high = this->sorted_indices + step_indices * i_high;

      int const row_A = row_src_low[tmp];
      int const row_B = row_src_high[tmp];

      double const diff = CompareVectors(row_A, row_B, axis);
      if (diff < 0)
        {
          row_dst[axis] = row_A;
          i_low = i_low + 1;
        }
      else
        {
          row_dst[axis] = row_B;
          i_high = i_high - 1;
        }
      /* if */
    }
  /* for */
}
/* KDTreeRoot_::MergeSort */



//! Minimal distance.
/*!
  Function computes minimal squared half-distance between vectors in data.
  This value may be used to limit KD tree traversal if a node is closer than squared half-distance.

  \return Returns minimal squared half-distance or NaN if unsuccessfull.
*/
double
KDTreeRoot_::MinimalSquaredHalfDistance(
                                        void
                                        )
{
  double min_dst2 = std::numeric_limits<double>::quiet_NaN();

  assert(NULL != this->data);
  if (NULL == this->data) return min_dst2;

  min_dst2 = std::numeric_limits<double>::infinity();
  for (int i = 0; i < this->n_pts; ++i)
    {
      double const * const row_i = (double const *)( (BYTE const *)(this->data) + i * this->data_stride );
      for (int j = i + 1; j < this->n_pts; ++j)
        {
          double const * const row_j = (double const *)( (BYTE const *)(this->data) + j * this->data_stride );

          double dst2 = 0.0;
          for (int k = 0; k < this->n_dim; ++k)
            {
              double const diff = row_i[k] - row_j[k];
              dst2 += diff * diff;
            }
          /* for */

          if (dst2 < min_dst2) min_dst2 = dst2;
        }
      /* for */
    }
  /* for */

  // Half becomes quarter when squared.
  min_dst2 = min_dst2 * 0.25;

  return min_dst2;
}
/* KDTreeRoot_::MinimalSquaredHalfDistance */



//! Construct KD tree.
/*!
  Constructs KD tree for given data samples.

  \param data_in   Pointer to data.
  \param n_dim_in  Number of dimensions.
  \param n_pts_in  Number of data points.
  \param data_stride_in   Data stride in bytes (size of one row).
  \return Returns true if successfull, false otherwise.
*/
bool
KDTreeRoot_::ConstructTree(
                           double const * const data_in,
                           int const n_dim_in,
                           int const n_pts_in,
                           int const data_stride_in
                           )
{
  assert(NULL != data_in);
  if (NULL == data_in) return false;

  // Clear existing KD tree (if any).
  this->DeleteTree();

  // Copy data values.
  this->n_dim = n_dim_in;
  this->n_pts = n_pts_in;
  this->data_stride = data_stride_in;

  this->data = data_in;

  // Allocate temporary storage.
  assert(NULL == this->sorted_indices);
  this->sorted_indices = (int *)malloc( sizeof(int) * n_pts_in * (n_dim_in + 1) );
  assert(NULL != this->sorted_indices);

  // Return immediately if allocation fails of if there is no data.
  if ( (NULL == this->data) ||
       (NULL == this->sorted_indices)
       )
    {
      this->DeleteTree();
      return false;
    }
  /* if */

  // Create array of pre-sorted indices into data.
  int const step_indices = this->n_dim + 1;
  for (int i = 0; i < n_pts; ++i)
    {
      int * const row = this->sorted_indices + i * step_indices;
      for (int j = 0; j < this->n_dim; ++j) row[j] = i;
    }
  /* for */

  for (int axis = 0; axis < this->n_dim; ++axis)
    {
      MergeSort(0, this->n_pts - 1, axis);
    }
  /* for */

  // Compute minimal squared half-distance.
  this->min_half_dst2 = MinimalSquaredHalfDistance();
  this->min_half_dst = sqrt(this->min_half_dst2);

  // Recursively create KD tree.
  assert(NULL == this->tree);
  this->tree = KDTreeNode_::ConstructTree(this, 0, this->n_pts - 1, 0);
  assert(NULL != this->tree);

  // Verify KD tree.
  assert(true == this->tree->VerifyTree(0));

  // Free temporary memory.
  SAFE_FREE(this->sorted_indices);

  return (NULL != this->tree);
}
/* KDTreeRoot_::ConstructTree */



//! Find nearest neighbour.
/*!
  Function finds nearest neighbour

  \param nn Structure which holds query point and nearest neighbour.
  \return Returns true if successfull, false otherwise.
*/
bool
KDTreeRoot_::Find1NN(
                     KDTreeClosestPoint_ & nn
                     )
{
  assert(NULL != this->tree);
  if (NULL == this->tree) return false;

  assert(NULL != nn.query);
  if (NULL == nn.query) return false;

  // Invalidate closest point data.
  nn.ClearAllButIndex();

  // Check if previous solution is valid one.
  bool const is_best = this->Check1NN(nn);
  if (true == is_best) return true;

  // Recursively find closest point.
  bool const result = this->tree->Find1NN(nn, 0);
  assert(true == result);

  return result;
}
/* KDTreeRoot_::Find1NN */



//! Check if test point is closer than limit distance.
/*!
  Function checks if test point is closer than all other points.

  \param nn     Structure which holds query point and nearest neighbour.
  \return Returns true if successfull, false otherwise.
*/
bool
KDTreeRoot_::Check1NN(
                      KDTreeClosestPoint_ & nn
                      )
{
  assert(NULL != this->data);
  //if (NULL == this->data) return false;

  assert(false == nn.found_best);

  //assert( (0 <= nn.idx) && (nn.idx < this->n_pts) );
  if ( (nn.idx < 0) || (this->n_pts <= nn.idx) ) return false;

  // Get squared distace to the current node.
  int const n_dim = this->n_dim;
  double const * const row_cur = (double const *)( (BYTE const *)(this->data) + nn.idx * this->data_stride );
  double const dst2 = SquaredDistance_inline<double>(row_cur, nn.query, n_dim);

  // Check if the current node is the best possible match.
  if (dst2 < this->min_half_dst2)
    {
      // Mark the node as the best match.
      nn.value = row_cur;
      nn.dst2 = dst2;
      nn.found_best = true;

      return true;
    }
  else
    {
      // Invalidate closest point data.
      nn.Clear();

      return false;
    }
  /* if */

}
/* KDTreeRoot_::Check1NN */



/****** KD TREE NODE ******/

//! Constructor.
/*!
  Creates empty unlinked KD tree node.
*/
KDTreeNode_::KDTreeNode_()
{
  this->Blank();
}
/* KDTreeNode_::KDTreeNode_ */



//! Constructor.
/*!
  Creates node and initializes its data. Node must be linked manually.

  \param root_in        Pointer to root node.
  \param idx_in        Row index into temporary sorting array.
  \param axis_in       Splitting axis for this node.
  \param depth_in       Tree depth.
  \param type_in        Node type.
*/
KDTreeNode_::KDTreeNode_(
                         KDTreeRoot * const root_in,
                         int const idx_in,
                         int const axis_in,
                         int const depth_in,
                         signed char const type_in
                         )
{
  this->Blank();

  // Copy initialization data.
  assert( (0 <= axis_in) && (axis_in < SHORT_MAX) );
  this->axis = (short)axis_in;

  assert( (0 <= depth_in) && (depth_in < SHORT_MAX) );
  this->depth = (short)depth_in;

  this->type = type_in;

  // Set root node.
  assert(NULL != root_in);
  this->root = root_in;

  // Set remaining data.
  if (NULL != root_in)
    {
      // Set element row index.
      int * const sorted_indices = root_in->sorted_indices;
      assert(NULL != sorted_indices);

      if (NULL != sorted_indices)
        {
          int const step_indices = root_in->n_dim + 1;
          assert( (0 <= idx_in) && (idx_in < root_in->n_pts) );

          this->row_idx = sorted_indices[step_indices * idx_in];
          assert( (0 <= this->row_idx) && (this->row_idx < root_in->n_pts) );

          // Set pivot value.
          double const * const data = root_in->data;
          assert(NULL != data);
          if (NULL != data)
            {
              double const * const row = (double const *)( (BYTE const *)(data) + this->row_idx * root_in->data_stride);
              this->pivot = row[axis_in];
            }
          /* if */
        }
      /* if */

      // Update statistics.
      root_in->num_nodes += 1;
      if (type_in == KDTREE_NODE_LEAF) root_in->num_leaves += 1;
      if (type_in == KDTREE_NODE_BRANCH) root_in->num_branches += 1;
      if (root_in->max_depth < depth) root_in->max_depth = depth;
    }
  /* if */
}
/* KDTreeNode_::KDTreeNode_ */



//! Destructor.
/*!
  Destroys KDTreeNode_ structure.
*/
KDTreeNode_::~KDTreeNode_()
{
  this->DeleteTree();
  this->Blank();
}
/* KDTreeNode_::~KDTreeNode_ */



//! Blank class variables.
/*!
  Initializes all class variables.
*/
void
KDTreeNode_::Blank(
                   void
                   )
{
  this->root = NULL;
  this->less_than_pivot = NULL;
  this->equal_to_or_greater_than_pivot = NULL;
  this->pivot = std::numeric_limits<double>::quiet_NaN();
  this->row_idx = -1;
  this->axis = -1;
  this->depth = -1;
  this->type = KDTREE_NODE_UNDEFINED;
}
/* KDTreeNode_::Blank */



//! Recursively delete tree.
/*!
  Calls destructors of two leaf nodes.
*/
void
KDTreeNode_::DeleteTree(
                        void
                        )
{
  SAFE_DELETE( this->less_than_pivot );
  SAFE_DELETE( this->equal_to_or_greater_than_pivot );
}
/* KDTreeNode */



//! Recursively create KD tree.
/*!
  Function recursively creates KD tree.
  Function operates on a continuous range of rows of the root->data and root->sorted_indices arrays.

  \param root   Pointer to placeholder root node which holds all shared data for the KD tree.
  \param start  Starting row index into datapoints.
  \param end    Ending row index into datapoints.
  \param depth  Depth at the current call. Note that active axis is determined from depth as depth % root->n_dim.
  \return Returns root node of the constructed KD tree.
*/
KDTreeNode_ *
KDTreeNode_::ConstructTree(
                           KDTreeRoot * const root,
                           int const start,
                           int const end,
                           int const depth
                           )
{
  assert(NULL != root);
  if (NULL == root) return NULL;

  int const n_dim = root->n_dim;
  int const n_pts = root->n_pts;

  assert( (0 <= start) && (start < n_pts) );
  assert( (0 <= end) && (end < n_pts) );
  assert( start <= end );
  assert( 0 <= depth );

  // Get current axis index.
  int const axis = depth % n_dim;

  if (start == end)
    {
      // Only one element remains so create leaf node and return its pointer.
      KDTreeNode_ * const leaf = new KDTreeNode_(root, start, axis, depth, KDTREE_NODE_LEAF);
      assert(NULL != leaf);
      return leaf;
    }
  else if (start + 1 == end)
    {
      // Two elemnents remain so create one branch node and its leaf node.
      // Data is presorted so start is always greater than or equal to end.
      KDTreeNode_ * const branch = new KDTreeNode_(root, start, axis, depth, KDTREE_NODE_BRANCH);
      assert(NULL != branch);
      if (NULL != branch)
        {
          int const axis_next = (axis + 1) % n_dim;

          KDTreeNode_ * const leaf = new KDTreeNode_(root, end, axis_next, depth + 1, KDTREE_NODE_LEAF);
          assert(NULL != leaf);
          branch->equal_to_or_greater_than_pivot = leaf;
        }
      /* if */
      return branch;
    }
  else if (start + 2 == end)
    {
      // Three elements remain so create one branch node and its two leaf nodes.
      // Data is presorted so middle element may be selected as pivot.
      KDTreeNode_ * const branch = new KDTreeNode_(root, start + 1, axis, depth, KDTREE_NODE_BRANCH);
      assert(NULL != branch);
      if (NULL != branch)
        {
          int const axis_next = (axis + 1) % n_dim;

          KDTreeNode_ * const leaf_lt = new KDTreeNode_(root, start, axis_next, depth + 1, KDTREE_NODE_LEAF);
          assert(NULL != leaf_lt);
          branch->less_than_pivot = leaf_lt;

          KDTreeNode_ * const leaf_eqgt = new KDTreeNode_(root, end, axis_next, depth + 1, KDTREE_NODE_LEAF);
          assert(NULL != leaf_eqgt);
          branch->equal_to_or_greater_than_pivot = leaf_eqgt;
        }
      /* if */
      return branch;
    }
  else if (start + 2 < end)
    {
      // More than tree elements remain so we have to pick a pivot and split points into two sets.

      // Fetch temporary storage data.
      int * const sorted_indices = root->sorted_indices;
      assert(NULL != sorted_indices);

      // Fetch data pointer and dimensions.
      double const * const data = root->data;
      assert(NULL != data);

      int const step_indices = n_dim + 1;

      // Pivot is selected as the middle datapoint which is median due to points beeing pre-sorted.
      int pivot = start + (end - start) / 2;

      // If needed move the pivot so no elements whose value is equal to pivot appear before the pivot.
      if (start < pivot)
        {
          int pivot_idx = sorted_indices[step_indices * pivot];
          int prev_idx = sorted_indices[step_indices * (pivot - 1)];

          double const * pivot_row = (double const *)( (BYTE const *)(data) + pivot_idx * root->data_stride);
          double const * prev_row = (double const *)( (BYTE const *)(data) + prev_idx * root->data_stride);

          while (pivot_row[axis] == prev_row[axis])
            {
              --pivot;
              if (start >= pivot) break;

              pivot_idx = sorted_indices[step_indices * pivot];
              prev_idx = sorted_indices[step_indices * (pivot - 1)];

              pivot_row = (double const *)( (BYTE const *)(data) + pivot_idx * root->data_stride);
              prev_row = (double const *)( (BYTE const *)(data) + prev_idx * root->data_stride);
            }
          /* while */
        }
      /* if */

      assert( (start <= pivot) && (pivot <= end) );
      assert( (0 <= pivot) && (pivot < n_pts) );

      // Create branch node.
      KDTreeNode_ * const branch = new KDTreeNode_(root, pivot, axis, depth, KDTREE_NODE_BRANCH);
      assert(NULL != branch);

      // Repartition indices into array root->data stored in root->sorted_indices array.
      // Repartititioning is done for all axes except active one according to the pivot element of the active axis.
      // Note that repartitioning result for column i is stored in column i-1, i.e. columns are
      // permuted so the first column with index 0 will always contains sorted indices for the active
      // axis which is used to split the data.
      int lower = pivot - 1;
      int upper = end;
      if (1 < n_dim)
        {
          // Copy active axis from the first column to a temporary column before partitioning.
          // The temporary column is located after all columns, i.e. it is the last column.
          for (int i = start; i <= end; i++)
            {
              int * const row_srcdst = sorted_indices + step_indices * i;
              row_srcdst[n_dim] = row_srcdst[0];
            }
          /* for */

          for (int i = 1; i < n_dim; ++i)
            {
              // Re-sort according to the current pivot element.
              // As elements are already presorted they remain sorted after split.
              // Note that sorted elements are placed in the previous column.
              lower = start - 1;
              upper = pivot;
              int const prev = i - 1;
              for (int j = start; j <= end; ++j)
                {
                  int const * const row_src = sorted_indices + step_indices * j;
                  int const row_A = row_src[i];
                  int const row_B = branch->row_idx;
                  double const diff = root->CompareVectors(row_A, row_B, axis);
                  if (diff < 0)
                    {
                      lower = lower + 1;
                      int * const row_dst = sorted_indices + step_indices * lower;
                      row_dst[prev] = row_src[i];
                    }
                  else if (diff > 0)
                    {
                      upper = upper + 1;
                      int * const row_dst = sorted_indices + step_indices * upper;
                      row_dst[prev] = row_src[i];
                    }
                  /* if */
                }
              /* for */

#ifdef _DEBUG
              {
                int * const row_dst = sorted_indices + step_indices * pivot;
                row_dst[prev] = -1; // Invalidate pivot.
              }
#endif /* _DEBUG */

              assert( (start <= lower) && (lower < pivot) );
              assert( (pivot < upper) && (upper <= end) );
            }
          /* for */

          // Restore active axis data by copying it from the temporary column.
          int const col = n_dim - 1;
          for (int i = start; i <= end; i++)
            {
              int * const row_srcdst = sorted_indices + step_indices * i;
              row_srcdst[col] = row_srcdst[n_dim];
            }
          /* for */
        }
      /* if */

      // Create subtree for elements less than pivot.
      assert(NULL == branch->less_than_pivot);
      branch->less_than_pivot = KDTreeNode_::ConstructTree(root, start, lower, depth + 1);

      // Create subtree for elements equal to or greater than pivot.
      assert(NULL == branch->equal_to_or_greater_than_pivot);
      branch->equal_to_or_greater_than_pivot = KDTreeNode_::ConstructTree(root, pivot + 1, upper, depth + 1);

      // Return root branch.
      return branch;
    }
  /* if */

  return NULL;
}
/* KDTreeNode_::ConstructTree */



//! Recursively verify KD tree.
/*!
  Function recursively verifies KD tree.

  \param depth  Current verification depth.
*/
bool
KDTreeNode_::VerifyTree(
                        int const depth
                        )
{
  assert(NULL != this->root);
  if (NULL == this->root) return false;

  int const n_dim = this->root->n_dim;
  int const n_pts = this->root->n_pts;

  BYTE const * data = (BYTE const *)(this->root->data);
  int const data_stride = this->root->data_stride;

  // Get current axis index.
  int const axis = depth % n_dim;

  // Assume tree is valid.
  bool is_valid = true;

  // Leaf node cannot have children.
  if (KDTREE_NODE_LEAF == this->type)
    {
      is_valid = is_valid && (NULL == this->less_than_pivot);
      assert(true == is_valid);

      is_valid = is_valid && (NULL == this->equal_to_or_greater_than_pivot);
      assert(true == is_valid);
    }
  /* if */

  // Two subtrees may match in the active axis but cannot match in all axes.
  // This is why two test are done: first test checks only active axis coordinate
  // while the second test checks all axes. The second test is applicable only
  // if input vectors do not contain duplicates which is true for point cloud
  // scanner data and for constellation used in phase unwrapping.
  if (NULL != this->less_than_pivot)
    {
      double const * const row_pvt = (double const *)( data + this->row_idx * data_stride );
      double const * const row_lt = (double const *)( data + this->less_than_pivot->row_idx * data_stride );
      is_valid = is_valid && (row_lt[axis] <= row_pvt[axis]);
      assert(true == is_valid);

      double const diff = root->CompareVectors(row_lt, row_pvt, axis);
      is_valid = is_valid && (diff < 0);
      assert(true == is_valid);

      is_valid = is_valid && this->less_than_pivot->VerifyTree(depth + 1);
      assert(true == is_valid);
    }
  /* if */

  if (NULL != this->equal_to_or_greater_than_pivot)
    {
      double const * const row_pvt = (double const *)( data + this->row_idx * data_stride );
      double const * const row_eqgt = (double const *)( data + this->equal_to_or_greater_than_pivot->row_idx * data_stride );
      is_valid = is_valid && (row_pvt[axis] <= row_eqgt[axis]);
      assert(true == is_valid);

      double const diff = root->CompareVectors(row_eqgt, row_pvt, axis);
      is_valid = is_valid && (diff > 0);
      assert(true == is_valid);

      is_valid = is_valid && this->equal_to_or_greater_than_pivot->VerifyTree(depth + 1);
      assert(true == is_valid);
    }
  /* if */

  return is_valid;
}
/* KDTreeNode_::VerifyTree */



//! Find nearest neighbour.
/*!
  Recursively finds closest point in the tree.

  \param nn     Reference to structure holding both query and the closest point.
  \param depth  Current search depth.
*/
bool
KDTreeNode_::Find1NN(
                     KDTreeClosestPoint_ & nn,
                     int const depth
                     )
{
  assert(NULL != this->root);
  if (NULL == this->root) return false;

  assert(NULL != this->root->data);
  //if (NULL == this->root->data) return false;

  // Assume success.
  bool result = true;

  // If we already found the best match return immediately.
  if (true == nn.found_best) return true;

  // Get squared distace to the current node.
  int const n_dim = this->root->n_dim;
  double const * const row_cur = (double const *)( (BYTE const *)(this->root->data) + this->row_idx * this->root->data_stride );
  double const dst2 = SquaredDistance_inline<double>(row_cur, nn.query, n_dim);

  // Check if the current node is the best possible match and terminate early if it is.
  if (dst2 < this->root->min_half_dst2)
    {
      nn.value = row_cur;
      nn.dst2 = dst2;
      nn.idx = this->row_idx;
      nn.found_best = true; // Mark the node as the best match.

      return result;
    }
  /* if */

  // Get current axis index.
  int const axis = depth % n_dim;

  // Get current coordinate of the query point.
  double const query = nn.query[axis];

  // Test if we have both children.
  bool const have_lt = (NULL != this->less_than_pivot);
  bool const have_gteq = (NULL != this->equal_to_or_greater_than_pivot);

  if ( (true == have_lt) && (true == have_gteq) )
    {
      // Select subtree based on pivot value.
      double const dst_to_hyperplane = query - this->pivot;
      bool const goto_lt = (0.0 > dst_to_hyperplane);
      if (true == goto_lt)
        {
          // Check the LT subtree.
          result = result & this->less_than_pivot->Find1NN(nn, depth + 1);
          assert(true == result);
        }
      else
        {
          // Check the GTEQ subtree.
          result = result & this->equal_to_or_greater_than_pivot->Find1NN(nn, depth + 1);
          assert(true == result);
        }
      /* if */

      // If the current node is a better match then store it.
      if (dst2 < nn.dst2)
        {
          nn.value = row_cur;
          nn.dst2 = dst2;
          nn.idx = this->row_idx;
        }
      /* if */

      // If slicing hyperplane is closer than the current best match check the other side of the hyperplane.
      // As this test is done on recursion return perform it only if the best point is not found.
      if (false == nn.found_best)
        {
          int const dst2_to_hyperplane = dst_to_hyperplane * dst_to_hyperplane;
          if ( dst2_to_hyperplane < nn.dst2 )
            {
              if (true == goto_lt)
                {
                  result = result & this->equal_to_or_greater_than_pivot->Find1NN(nn, depth + 1);
                  assert(true == result);
                }
              else
                {
                  result = result & this->less_than_pivot->Find1NN(nn, depth + 1);
                  assert(true == result);
                }
              /* if */
            }
          /* if */
        }
      /* if */
    }
  else if (true == have_lt)
    {
      // Check the LT subtree.
      result = result & this->less_than_pivot->Find1NN(nn, depth + 1);
      assert(true == result);

      // If the current node is a better match then store it.
      if (dst2 < nn.dst2)
        {
          nn.value = row_cur;
          nn.dst2 = dst2;
          nn.idx = this->row_idx;
        }
      /* if */
    }
  else if (true == have_gteq)
    {
      // Check the GTEQ subtree.
      result = result & this->equal_to_or_greater_than_pivot->Find1NN(nn, depth + 1);
      assert(true == result);

      // If the current node is a better match then store it.
      if (dst2 < nn.dst2)
        {
          nn.value = row_cur;
          nn.dst2 = dst2;
          nn.idx = this->row_idx;
        }
      /* if */
    }
  else
    {
      // TODO: Once support for bucket storage is added to the leaf node put code
      // to sequentially test all points in the node here.
      assert(KDTREE_NODE_LEAF == this->type);
    }
  /* if */

  return result;
}
/* KDTreeNode_::Find1NN */



/****** KD TREE CLOSEST POINT ******/

//! Constructor.
KDTreeClosestPoint_::KDTreeClosestPoint_()
{
  this->query = NULL;
  this->Clear();
}
/* KDTreeClosestPoint_ */



//! Clear closest point.
/*!
  Clears data which identifies the closest point to the query point.
*/
void
KDTreeClosestPoint_::Clear(
                           void
                           )
{
  this->idx = -1;
  this->ClearAllButIndex();
}
/* KDTreeClosestPoint_::Clear */



//! Clear closest point.
/*!
  Clears data which identifies the closest point to the query point.
*/
void
KDTreeClosestPoint_::ClearAllButIndex(
                                      void
                                      )
{
  this->value = NULL;
  this->dst2 = std::numeric_limits<double>::infinity();
  this->found_best = false;
}
/* KDTreeClosestPoint_::ClearAllButIndex */



#endif /* !__BATCHACQUISITIONKDTREE_CPP */
