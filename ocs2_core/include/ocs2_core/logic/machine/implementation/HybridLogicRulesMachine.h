/******************************************************************************
Copyright (c) 2017, Farbod Farshidian. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

namespace ocs2{

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/

void HybridLogicRulesMachine::setupLogicMachine(
		const scalar_array_t& partitioningTimes,
		const scalar_t& initTime,
		const size_t& initActivePartition,
		const size_t& initSubsystemID) {

	//**********************************************//
	//****************** LogicRule *****************//
	//**********************************************//
	// delete the future event information
	size_t index = std::lower_bound(hybridLogicRules_->eventTimes().begin(), hybridLogicRules_->eventTimes().end(), initTime) -
					hybridLogicRules_->eventTimes().begin();
	hybridLogicRules_->eventTimes().erase(
			hybridLogicRules_->eventTimes().begin()+index, hybridLogicRules_->eventTimes().end());
	hybridLogicRules_->subsystemsSequence().erase(
			hybridLogicRules_->subsystemsSequence().begin()+index+1, hybridLogicRules_->subsystemsSequence().end());

	// if the current subsystem is not the same as expected.
	if (hybridLogicRules_->subsystemsSequence().empty()) {
		hybridLogicRules_->subsystemsSequence().push_back(initSubsystemID);

	} else if (hybridLogicRules_->subsystemsSequence().back()!=initSubsystemID) {
		hybridLogicRules_->eventTimes().push_back(initTime);
		hybridLogicRules_->subsystemsSequence().push_back(initSubsystemID);
	}
	// update logic rules
	hybridLogicRules_->update();

	size_t initEventCounter = hybridLogicRules_->subsystemsSequence().size()-1;

	//**********************************************//
	//**************** LogicMachine ****************//
	//**********************************************//
	const size_t numPartitions = partitioningTimes.size()-1;

	BASE::logicRulesModified_ = false;
	BASE::numPartitions_ = numPartitions;
	BASE::partitioningTimes_ = partitioningTimes;

	BASE::eventTimesStock_.resize(numPartitions);
	BASE::eventCountersStock_.resize(numPartitions);
	BASE::switchingTimesStock_.resize(numPartitions);

	// for the future partitions
	for (size_t i=initActivePartition+1; i<numPartitions; i++) {
		BASE::eventTimesStock_[i].clear();
		BASE::eventCountersStock_[i].clear();
		BASE::switchingTimesStock_[i] = scalar_array_t {partitioningTimes[i], partitioningTimes[i+1]};
	}  // end of i loop

	// for the initActivePartition partition
	if (BASE::eventTimesStock_[initActivePartition].empty()) {

		BASE::eventTimesStock_[initActivePartition].clear();
		BASE::eventCountersStock_[initActivePartition] = size_array_t {initEventCounter};
		BASE::switchingTimesStock_[initActivePartition] = scalar_array_t {partitioningTimes[initActivePartition], partitioningTimes[initActivePartition+1]};

	} else {
		size_t index = std::lower_bound(BASE::eventTimesStock_[initActivePartition].begin(), BASE::eventTimesStock_[initActivePartition].end(), initTime) -
				BASE::eventTimesStock_[initActivePartition].begin();

		BASE::eventTimesStock_[initActivePartition].erase(
				BASE::eventTimesStock_[initActivePartition].begin()+index, BASE::eventTimesStock_[initActivePartition].end());

		BASE::eventCountersStock_[initActivePartition].erase(
				BASE::eventCountersStock_[initActivePartition].begin()+index+1, BASE::eventCountersStock_[initActivePartition].end());

		BASE::switchingTimesStock_[initActivePartition].erase(
				BASE::switchingTimesStock_[initActivePartition].begin()+index+1, BASE::switchingTimesStock_[initActivePartition].end());

		if (BASE::eventCountersStock_[initActivePartition].back() != initEventCounter) {
			BASE::eventTimesStock_[initActivePartition].push_back(initTime);
			BASE::eventCountersStock_[initActivePartition].push_back(initEventCounter);
			BASE::switchingTimesStock_[initActivePartition].push_back(initTime);
		}
		BASE::switchingTimesStock_[initActivePartition].push_back(partitioningTimes[initActivePartition+1]);
	}
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/

void HybridLogicRulesMachine::initLogicMachine(
		const size_t& partitionIndex) {

	if (BASE::eventCountersStock_[partitionIndex].empty()) {
		if (partitionIndex==0) {
			throw std::runtime_error("ocs2::setupLogicMachine function should have been called before.");
		}
		if (BASE::eventCountersStock_[partitionIndex-1].empty()) {
			throw std::runtime_error("ocs2::initLogicMachine function should have been called for the previous partition first.");
		}

		BASE::eventCountersStock_[partitionIndex].push_back(hybridLogicRules_->subsystemsSequence().size()-1);
	}
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/

void HybridLogicRulesMachine::pushBack(
    const size_t &partitionIndex,
    const scalar_t &eventTime,
    const size_t &subsystemID) {

	hybridLogicRules_->eventTimes().push_back(eventTime);
	hybridLogicRules_->subsystemsSequence().push_back(subsystemID);
	hybridLogicRules_->update();

	BASE::eventTimesStock_[partitionIndex].push_back(eventTime);

	BASE::eventCountersStock_[partitionIndex].push_back(hybridLogicRules_->subsystemsSequence().size()-1);

	BASE::switchingTimesStock_[partitionIndex].push_back(BASE::switchingTimesStock_[partitionIndex].back());
	*(BASE::switchingTimesStock_[partitionIndex].end()-2) = eventTime;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/

void HybridLogicRulesMachine::display() const {

	BASE::display();

	size_t itr = 0;
	std::cerr << "Event Subsystem ID distribution for partitions:\n\t ";
	for (auto& eventCounters : BASE::eventCountersStock_) {  // printing
		std::cerr << itr << ":{";
		itr++;

		for (auto& i : eventCounters) {
			std::cerr << hybridLogicRules_->subsystemsSequence()[i] << ", ";
		}
		if (!eventCounters.empty()) {  std::cerr << "\b\b";
		}
		std::cerr << "},  ";
	}
	std::cerr << std::endl;
}

} // namespace ocs2


