
#ifndef REG_SELECTION_C                                        
#define REG_SELECTION_C

#include "RegularSelection.h"

RegularSelection :: RegularSelection (MyDB_TableReaderWriterPtr input, MyDB_TableReaderWriterPtr output,
                string selectionPredicate, vector <string> projections) {

    this->input = input;
    this->output = output;
    this->selectionPredicate = selectionPredicate;
    this->projections = projections;
}

void RegularSelection :: run () {
    // input record
    MyDB_RecordPtr inputRec = input->getEmptyRecord ();

    // output record
    MyDB_RecordPtr outputRec = output->getEmptyRecord ();

    // Create func to run predicate
    func selectPred = inputRec->compileComputation (selectionPredicate);

    // Create an iterator over input table
    MyDB_RecordIteratorAltPtr inputIter = input->getIteratorAlt ();

    // and get the final set of computatoins that will be used to buld the output record
    vector <func> finalComputations;
    for (string s : projections) {
        finalComputations.push_back (inputRec->compileComputation (s));
    }

    // Iterate over input table
    while (inputIter->advance()){

        inputIter->getCurrent(inputRec);

        // Check if the record satisfies the predicate
        if(!selectPred()->toBool()){
            continue;
        }

        // Project the attributes to output record
        int i = 0;
        for (auto &f : finalComputations){
            outputRec->getAtt(i++)->set(f());
        }


        // Tell the output record has changed
        outputRec->recordContentHasChanged();

        // Add the record to the output table
        output->append(outputRec);

    }

}

#endif
