#pragma once

#ifndef LIB_INCLUDE_ESTIMATOR_STATE_FWD_HPP_
#define LIB_INCLUDE_ESTIMATOR_STATE_FWD_HPP_


namespace Tsetlini
{


struct TAState;
struct ClassifierStateCache;
struct RegressorStateCache;

template<typename TAStateType, typename EstimatorStateCacheType>
struct EstimatorState;

using ClassifierState = EstimatorState<TAState, ClassifierStateCache>;
using RegressorState = EstimatorState<TAState, RegressorStateCache>;


}  // namespace Tsetlini


#endif /* LIB_INCLUDE_ESTIMATOR_STATE_FWD_HPP_ */