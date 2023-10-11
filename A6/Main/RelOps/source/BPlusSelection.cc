
#ifndef BPLUS_SELECTION_C                                        
#define BPLUS_SELECTION_C

#include "BPlusSelection.h"

BPlusSelection :: BPlusSelection (MyDB_BPlusTreeReaderWriterPtr input, MyDB_TableReaderWriterPtr output,
                MyDB_AttValPtr low, MyDB_AttValPtr high,
                string selectionPredicate, vector <string> projections) {

    this->input = input;
    this->output = output;
    this->low = low;
    this->high = high;
    this->selectionPredicate = selectionPredicate;
    this->projections = projections;
}

void BPlusSelection :: run () {
    // input record
    MyDB_RecordPtr inputRec = input->getEmptyRecord ();

    // output record
    MyDB_RecordPtr outputRec = output->getEmptyRecord ();

    // Get an iterator from the B+ tree in the range(low, high)
    MyDB_RecordIteratorAltPtr inputIter = input->getRangeIteratorAlt(low, high);

    // Create func to run predicate (Where clause)
    func selectPred = inputRec->compileComputation (selectionPredicate);

    // Get the set of computations that will be used to build the output record
    vector <func> finalComputations;
    for (string s : projections) {
        finalComputations.push_back (inputRec->compileComputation (s));
    }

    // Iterate over range table
    while (inputIter->advance()) {
        inputIter->getCurrent(inputRec);

        // Check if the record satisfies the predicate
        if (!selectPred()->toBool()) {
            continue;
        }

        // Project the attributes to output record
        int i = 0;
        for (auto &f: finalComputations) {
            outputRec->getAtt(i++)->set(f());
        }

        // Tell the output record has changed
        outputRec->recordContentHasChanged();

        // Add the record to the output table
        output->append(outputRec);

    }

}

#endif
