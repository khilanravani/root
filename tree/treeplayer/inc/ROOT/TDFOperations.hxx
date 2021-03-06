// Author: Enrico Guiraud, Danilo Piparo CERN  12/2016

/*************************************************************************
 * Copyright (C) 1995-2016, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TDFOPERATIONS
#define ROOT_TDFOPERATIONS

#include "ROOT/TDFTraitsUtils.hxx"
#include "ROOT/TThreadedObject.hxx"

#include <algorithm>
#include <limits>
#include <memory>
#include <vector>

#include "TH1F.h"

/// \cond HIDDEN_SYMBOLS

namespace ROOT {

namespace Internal {

namespace Operations {

using namespace Internal::TDFTraitsUtils;
using Count_t = unsigned long;
using Hist_t = ::TH1F;

class CountOperation {
   unsigned int *fResultCount;
   std::vector<Count_t> fCounts;

public:
   CountOperation(unsigned int *resultCount, unsigned int nSlots);
   void Exec(unsigned int slot);
   void Finalize();
   ~CountOperation();
};

class FillOperation {
   // this sets a total initial size of 16 MB for the buffers (can increase)
   static constexpr unsigned int fgTotalBufSize = 2097152;
   using BufEl_t = double;
   using Buf_t = std::vector<BufEl_t>;

   std::vector<Buf_t> fBuffers;
   std::vector<Buf_t> fWBuffers;
   std::shared_ptr<Hist_t> fResultHist;
   unsigned int fNSlots;
   unsigned int fBufSize;
   Buf_t fMin;
   Buf_t fMax;

   void UpdateMinMax(unsigned int slot, double v);

public:
   FillOperation(std::shared_ptr<Hist_t> h, unsigned int nSlots);
   void Exec(double v, unsigned int slot);
   void Exec(double v, double w, unsigned int slot);

   template <typename T, typename std::enable_if<TIsContainer<T>::fgValue, int>::type = 0>
   void Exec(const T &vs, unsigned int slot)
   {
      auto& thisBuf = fBuffers[slot];
      for (auto& v : vs) {
         UpdateMinMax(slot, v);
         thisBuf.emplace_back(v); // TODO: Can be optimised in case T == BufEl_t
      }
   }

   template <typename T, typename W, typename std::enable_if<TIsContainer<T>::fgValue && TIsContainer<W>::fgValue, int>::type = 0>
   void Exec(const T &vs, const W &ws, unsigned int slot)
   {
      auto& thisBuf = fBuffers[slot];
      for (auto& v : vs) {
         UpdateMinMax(slot, v);
         thisBuf.emplace_back(v); // TODO: Can be optimised in case T == BufEl_t
      }

      auto& thisWBuf = fWBuffers[slot];
      for (auto& w : ws) {
         thisWBuf.emplace_back(w); // TODO: Can be optimised in case T == BufEl_t
      }
   }

   void Finalize();
   ~FillOperation();
};

extern template void FillOperation::Exec(const std::vector<float>&, unsigned int);
extern template void FillOperation::Exec(const std::vector<double>&, unsigned int);
extern template void FillOperation::Exec(const std::vector<char>&, unsigned int);
extern template void FillOperation::Exec(const std::vector<int>&, unsigned int);
extern template void FillOperation::Exec(const std::vector<unsigned int>&, unsigned int);
extern template void FillOperation::Exec(const std::vector<float>&, const std::vector<float>&, unsigned int);
extern template void FillOperation::Exec(const std::vector<double>&, const std::vector<double>&, unsigned int);
extern template void FillOperation::Exec(const std::vector<char>&, const std::vector<char>&, unsigned int);
extern template void FillOperation::Exec(const std::vector<int>&, const std::vector<int>&, unsigned int);
extern template void FillOperation::Exec(const std::vector<unsigned int>&, const std::vector<unsigned int>&, unsigned int);

class FillTOOperation {
   TThreadedObject<Hist_t> fTo;

public:
   FillTOOperation(std::shared_ptr<Hist_t> h, unsigned int nSlots);
   void Exec(double v, unsigned int slot);
   void Exec(double v, double w, unsigned int slot);

   template <typename T, typename std::enable_if<TIsContainer<T>::fgValue, int>::type = 0>
   void Exec(const T &vs, unsigned int slot)
   {
      auto thisSlotH = fTo.GetAtSlotUnchecked(slot);
      for (auto& v : vs) {
         thisSlotH->Fill(v); // TODO: Can be optimised in case T == vector<double>
      }
   }

   template <typename T, typename W,
             typename std::enable_if<TIsContainer<T>::fgValue && TIsContainer<W>::fgValue, int>::type = 0>
   void Exec(const T &vs, const W &ws, unsigned int slot)
   {
      auto thisSlotH = fTo.GetAtSlotUnchecked(slot);
      if (vs.size() != ws.size()) {
         throw std::runtime_error("Cannot fill weighted histogram with values in containers of different sizes.");
      }
      auto vsIt = std::begin(vs);
      const auto vsEnd = std::end(vs);
      auto wsIt = std::begin(ws);
      for (;vsIt!=vsEnd; vsIt++,wsIt++) {
         thisSlotH->Fill(*vsIt, *wsIt); // TODO: Can be optimised in case T == vector<double>
      }
   }

   void Finalize();
   ~FillTOOperation();
};

extern template void FillTOOperation::Exec(const std::vector<float>&, unsigned int);
extern template void FillTOOperation::Exec(const std::vector<double>&, unsigned int);
extern template void FillTOOperation::Exec(const std::vector<char>&, unsigned int);
extern template void FillTOOperation::Exec(const std::vector<int>&, unsigned int);
extern template void FillTOOperation::Exec(const std::vector<unsigned int>&, unsigned int);
extern template void FillTOOperation::Exec(const std::vector<float>&, const std::vector<float>&, unsigned int);
extern template void FillTOOperation::Exec(const std::vector<double>&, const std::vector<double>&, unsigned int);
extern template void FillTOOperation::Exec(const std::vector<char>&, const std::vector<char>&, unsigned int);
extern template void FillTOOperation::Exec(const std::vector<int>&, const std::vector<int>&, unsigned int);
extern template void FillTOOperation::Exec(const std::vector<unsigned int>&, const std::vector<unsigned int>&, unsigned int);

// note: changes to this class should probably be replicated in its partial
// specialization below
template<typename T, typename COLL>
class TakeOperation {
   std::vector<std::shared_ptr<COLL>> fColls;
public:
   TakeOperation(std::shared_ptr<COLL> resultColl, unsigned int nSlots)
   {
      fColls.emplace_back(resultColl);
      for (unsigned int i = 1; i < nSlots; ++i)
         fColls.emplace_back(std::make_shared<COLL>());
   }

   template <typename V, typename std::enable_if<!TIsContainer<V>::fgValue, int>::type = 0>
   void Exec(V v, unsigned int slot)
   {
      fColls[slot]->emplace_back(v);
   }

   template <typename V, typename std::enable_if<TIsContainer<V>::fgValue, int>::type = 0>
   void Exec(const V &vs, unsigned int slot)
   {
      auto thisColl = fColls[slot];
      thisColl.insert(std::begin(thisColl), std::begin(vs), std::begin(vs));
   }

   void Finalize()
   {
      auto rColl = fColls[0];
      for (unsigned int i = 1; i < fColls.size(); ++i) {
         auto& coll = fColls[i];
         for (T &v : *coll) {
            rColl->emplace_back(v);
         }
      }
   }
   ~TakeOperation()
   {
      Finalize();
   }
};

// note: changes to this class should probably be replicated in its unspecialized
// declaration above
template<typename T>
class TakeOperation<T, std::vector<T>> {
   std::vector<std::shared_ptr<std::vector<T>>> fColls;
public:
   TakeOperation(std::shared_ptr<std::vector<T>> resultColl, unsigned int nSlots)
   {
      fColls.emplace_back(resultColl);
      for (unsigned int i = 1; i < nSlots; ++i) {
         auto v = std::make_shared<std::vector<T>>();
         v->reserve(1024);
         fColls.emplace_back(v);
      }
   }

   template <typename V, typename std::enable_if<!TIsContainer<V>::fgValue, int>::type = 0>
   void Exec(V v, unsigned int slot)
   {
      fColls[slot]->emplace_back(v);
   }

   template <typename V, typename std::enable_if<TIsContainer<V>::fgValue, int>::type = 0>
   void Exec(const V &vs, unsigned int slot)
   {
      auto thisColl = fColls[slot];
      thisColl->insert(std::begin(thisColl), std::begin(vs), std::begin(vs));
   }

   void Finalize()
   {
      ULong64_t totSize = 0;
      for (auto& coll : fColls) totSize += coll->size();
      auto rColl = fColls[0];
      rColl->reserve(totSize);
      for (unsigned int i = 1; i < fColls.size(); ++i) {
         auto& coll = fColls[i];
         rColl->insert(rColl->end(), coll->begin(), coll->end());
      }
   }

   ~TakeOperation()
   {
      Finalize();
   }
};

class MinOperation {
   double *fResultMin;
   std::vector<double> fMins;

public:
   MinOperation(double *minVPtr, unsigned int nSlots);
   void Exec(double v, unsigned int slot);

   template <typename T, typename std::enable_if<TIsContainer<T>::fgValue, int>::type = 0>
   void Exec(const T &vs, unsigned int slot)
   {
      for (auto &&v : vs) fMins[slot] = std::min((double)v, fMins[slot]);
   }

   void Finalize();
   ~MinOperation();
};

extern template void MinOperation::Exec(const std::vector<float>&, unsigned int);
extern template void MinOperation::Exec(const std::vector<double>&, unsigned int);
extern template void MinOperation::Exec(const std::vector<char>&, unsigned int);
extern template void MinOperation::Exec(const std::vector<int>&, unsigned int);
extern template void MinOperation::Exec(const std::vector<unsigned int>&, unsigned int);

class MaxOperation {
   double *fResultMax;
   std::vector<double> fMaxs;

public:
   MaxOperation(double *maxVPtr, unsigned int nSlots);
   void Exec(double v, unsigned int slot);

   template <typename T, typename std::enable_if<TIsContainer<T>::fgValue, int>::type = 0>
   void Exec(const T &vs, unsigned int slot)
   {
      for (auto &&v : vs) fMaxs[slot] = std::max((double)v, fMaxs[slot]);
   }

   void Finalize();
   ~MaxOperation();
};

extern template void MaxOperation::Exec(const std::vector<float>&, unsigned int);
extern template void MaxOperation::Exec(const std::vector<double>&, unsigned int);
extern template void MaxOperation::Exec(const std::vector<char>&, unsigned int);
extern template void MaxOperation::Exec(const std::vector<int>&, unsigned int);
extern template void MaxOperation::Exec(const std::vector<unsigned int>&, unsigned int);


class MeanOperation {
   double *fResultMean;
   std::vector<Count_t> fCounts;
   std::vector<double> fSums;

public:
   MeanOperation(double *meanVPtr, unsigned int nSlots);
   void Exec(double v, unsigned int slot);

   template <typename T, typename std::enable_if<TIsContainer<T>::fgValue, int>::type = 0>
   void Exec(const T &vs, unsigned int slot)
   {
      for (auto &&v : vs) {
         fSums[slot] += v;
         fCounts[slot]++;
      }
   }

   void Finalize();
   ~MeanOperation();
};

extern template void MeanOperation::Exec(const std::vector<float>&, unsigned int);
extern template void MeanOperation::Exec(const std::vector<double>&, unsigned int);
extern template void MeanOperation::Exec(const std::vector<char>&, unsigned int);
extern template void MeanOperation::Exec(const std::vector<int>&, unsigned int);
extern template void MeanOperation::Exec(const std::vector<unsigned int>&, unsigned int);


} // end of NS Operations

} // end of NS Internal

} // end of NS ROOT

/// \endcond

#endif
