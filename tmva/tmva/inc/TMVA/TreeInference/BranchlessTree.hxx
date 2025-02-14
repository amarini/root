/**********************************************************************************
 * Project: ROOT - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *                                                                                *
 * Authors:                                                                       *
 *      Stefan Wunsch (stefan.wunsch@cern.ch)                                     *
 *      Luca Zampieri (luca.zampieri@alumni.epfl.ch)                              *
 *                                                                                *
 * Copyright (c) 2019:                                                            *
 *      CERN, Switzerland                                                         *
 *                                                                                *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted according to the terms listed in LICENSE           *
 * (http://tmva.sourceforge.net/LICENSE)                                          *
 **********************************************************************************/

#ifndef TMVA_TREEINFERENCE_BRANCHLESSTREE
#define TMVA_TREEINFERENCE_BRANCHLESSTREE

#include <vector>
#include <algorithm>

namespace TMVA {
namespace Experimental {

namespace Internal {

/// Fill the empty nodes of a sparse tree recursively
template <typename T>
void RecursiveFill(int thisIndex, int lastIndex, int treeDepth, int maxTreeDepth, std::vector<T> &thresholds,
                   std::vector<int> &inputs)
{
   // If we are upstream of a leaf in a sparse branch, copy the last threshold value
   // and mark this node as a leaf again
   if (inputs[lastIndex] == -1) {
      thresholds.at(thisIndex) = thresholds.at(lastIndex);
      // Don't access the feature vector in the last layer of the tree since we
      // don't store these values in the inputs vector
      if (treeDepth < maxTreeDepth)
         inputs.at(thisIndex) = -1;
   }

   // Fill the children of this node if we are not in the final layer of the tree
   if (treeDepth < maxTreeDepth) {
      Internal::RecursiveFill<T>(2 * thisIndex + 1, thisIndex, treeDepth + 1, maxTreeDepth, thresholds, inputs);
      Internal::RecursiveFill<T>(2 * thisIndex + 2, thisIndex, treeDepth + 1, maxTreeDepth, thresholds, inputs);
   }
}

} // namespace Internal

/// \class Tree
/// \brief Branchless representation of a decision tree using topological ordering
///
/// \tparam T Value type for the computation (usually floating point type)
template <typename T>
struct BranchlessTree {
   int fTreeDepth;             ///< Depth of the tree
   std::vector<T> fThresholds; ///< Cut thresholds or scores if corresponding node is a leaf
   std::vector<int> fInputs;   ///< Cut variables / inputs

   inline T Inference(const T *input, const int stride);
   inline void FillSparse();
};

/// Perform inference on a single input vector
/// \param[in] input Pointer to data containing the input values
/// \param[in] stride Stride to go from one input variable to the next one
/// \param[out] Tree score, result of the inference
template <typename T>
inline T BranchlessTree<T>::Inference(const T *input, const int stride)
{
   int index = 0;
   for (int level = 0; level < fTreeDepth; ++level) {
      index = 2 * index + 1 + (input[fInputs[index] * stride] > fThresholds[index]);
   }
   return fThresholds[index];
}

/// Fill nodes of a sparse tree forming a full tree
///
/// Sparse parts of the tree are marked with -1 values in the feature vector. The
/// algorithm fills these parts up with the last threshold value so that the result
/// of the inference stays the same but the computation always traverses the full tree,
/// which is needed to avoid branching logic.
template <typename T>
inline void BranchlessTree<T>::FillSparse()
{
   // Fill threshold / leaf values recursively
   Internal::RecursiveFill<T>(1, 0, 1, fTreeDepth, fThresholds, fInputs);
   Internal::RecursiveFill<T>(2, 0, 1, fTreeDepth, fThresholds, fInputs);

   // Replace feature indices of -1 with 0
   std::replace(fInputs.begin(), fInputs.end(), -1.0, 0.0);
}

} // namespace Experimental
} // namespace TMVA

#endif // TMVA_TREEINFERENCE_BRANCHLESSTREE
