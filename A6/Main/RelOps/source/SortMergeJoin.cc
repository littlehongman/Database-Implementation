
#ifndef SORTMERGE_CC
#define SORTMERGE_CC

#include "Aggregate.h"
#include "MyDB_Record.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableReaderWriter.h"
#include "SortMergeJoin.h"
#include "Sorting.h"

#include "RegularSelection.h"

SortMergeJoin :: SortMergeJoin (MyDB_TableReaderWriterPtr leftInput, MyDB_TableReaderWriterPtr rightInput,
                                MyDB_TableReaderWriterPtr output, string finalSelectionPredicate,
                                vector <string> projections,
                                pair <string, string> equalityCheck, string leftSelectionPredicate,
                                string rightSelectionPredicate) {

    this->leftInput = leftInput;
    this->rightInput = rightInput;
    this->output = output;

    this->finalSelectionPredicate = finalSelectionPredicate;
    this->rightSelectionPredicate = rightSelectionPredicate;
    this->leftSelectionPredicate = leftSelectionPredicate;

    this->projections = projections;
    this->equalityCheck = equalityCheck;

    this->runSize = leftInput->getBufferMgr()->numPages / 2;
}

void SortMergeJoin :: run () {


//    // 1. Discard records from both inputs that do not satisfy selection predicate
//
//    // Run regularSection on leftInput to get the records that satisfy leftSelectionPredicate
//    MyDB_TableReaderWriterPtr matchedLeftInput = make_shared <MyDB_TableReaderWriter> (leftInput->getTable(), leftInput->getBufferMgr());
//
//    MyDB_RecordPtr leftInputRec = leftInput->getEmptyRecord();
//
//    // Create func to run left selection predicate
//    func leftPred = leftInputRec->compileComputation(leftSelectionPredicate);
//
//    // Create an iterator over left input table
//    MyDB_RecordIteratorAltPtr leftInputIter = leftInput->getIteratorAlt();
//
//    while (leftInputIter->advance()){
//        leftInputIter->getCurrent(leftInputRec);
//
//        // Check if the record satisfies the predicate
//        if(!leftPred()->toBool()){
//            continue;
//        }
//
//        // Add the record to the output table
//        matchedLeftInput->append(leftInputRec);
//    }
//
//    // Run regularSection on rightInput to get the records that satisfy rightSelectionPredicate
//    MyDB_TableReaderWriterPtr matchedRightInput = make_shared <MyDB_TableReaderWriter> (rightInput->getTable(), rightInput->getBufferMgr());
//
//    MyDB_RecordPtr rightInputRec = rightInput->getEmptyRecord();
//
//    // Create func to run right selection predicate
//    func rightPred = rightInputRec->compileComputation(rightSelectionPredicate);
//
//    // Create an iterator over right input table
//    MyDB_RecordIteratorAltPtr rightInputIter = rightInput->getIteratorAlt();
//
//    while (rightInputIter->advance()){
//        rightInputIter->getCurrent(rightInputRec);
//
//        // Check if the record satisfies the predicate
//        if(!rightPred()->toBool()){
//            continue;
//        }
//
//        // Add the record to the output table
//        matchedRightInput->append(rightInputRec);
//    }

    // ALTERNATIVE 1 & 2: Use the built-in operator to do the sorted records with predicate selection

    // Get sorted left input table that satisfies left selection predicate
    MyDB_RecordPtr leftRec1 = leftInput->getEmptyRecord ();
    MyDB_RecordPtr leftRec2 = leftInput->getEmptyRecord ();

    function <bool ()> compLeft = buildRecordComparator (leftRec1, leftRec2, equalityCheck.first);

    MyDB_RecordIteratorAltPtr matchedLeftIter = buildItertorOverSortedRuns(runSize, *leftInput, compLeft, leftRec1, leftRec2, leftSelectionPredicate);

    // Get sorted right input table that satisfies right selection predicate
    MyDB_RecordPtr rightRec1 = rightInput->getEmptyRecord ();
    MyDB_RecordPtr rightRec2 = rightInput->getEmptyRecord ();

    function <bool ()> compRight = buildRecordComparator (rightRec1, rightRec2, equalityCheck.second);

    MyDB_RecordIteratorAltPtr matchedRightIter = buildItertorOverSortedRuns(runSize, *rightInput, compRight, rightRec1, rightRec2, rightSelectionPredicate);

    // 3. Join the two tables

    // Get the schema that results from combining the left and right records
    MyDB_SchemaPtr mySchemaOut = make_shared <MyDB_Schema> ();
    for (auto &p : leftInput->getTable ()->getSchema ()->getAtts ())
        mySchemaOut->appendAtt (p);
    for (auto &p : rightInput->getTable ()->getSchema ()->getAtts ())
        mySchemaOut->appendAtt (p);

    MyDB_RecordPtr leftRec = leftInput->getEmptyRecord();
    MyDB_RecordPtr rightRec = rightInput->getEmptyRecord();

    // get the combined record
    MyDB_RecordPtr combinedRec = make_shared <MyDB_Record> (mySchemaOut);
    combinedRec->buildFrom (leftRec, rightRec);

    // Get the final predicate that run on both left and right records
    func finalPredicate = combinedRec->compileComputation (finalSelectionPredicate);

    // Get the projection
    vector <func> finalComputations;
    for (string s : projections) {
        finalComputations.push_back (combinedRec->compileComputation (s));
    }

    // Create a output recordPtr
    MyDB_RecordPtr outputRec = output->getEmptyRecord();

    // Create comparators to compare the left and right records
    func smaller = combinedRec->compileComputation("< (" + equalityCheck.first + "," + equalityCheck.second + ")");
    func larger = combinedRec->compileComputation("> (" + equalityCheck.first + "," + equalityCheck.second + ")");
    func equal = combinedRec->compileComputation("== (" + equalityCheck.first + "," + equalityCheck.second + ")");

    // If either of the input tables is empty, return
    if (!matchedLeftIter->advance() || !matchedRightIter->advance()){
        return;
    }

    // Create a helper comparator to get records that same values continuously
    function <bool ()> compLeftHelper1 = buildRecordComparator (leftRec, leftRec1, equalityCheck.first);
    function <bool ()> compLeftHelper2 = buildRecordComparator (leftRec1, leftRec, equalityCheck.first);

    // Create variables to keep track of the current records
    vector<void*> potentialMatches = {};
    bool reachEnd = false;
//    int counter = 0, sub_count = 0;

    while (!reachEnd){

        // Get the current left and right records
        matchedLeftIter->getCurrent(leftRec);
        matchedRightIter->getCurrent(rightRec);

        // Compare the left and right records
        if (smaller()->toBool()){
            // If the left record is smaller than the right record, advance the left record
            if (!matchedLeftIter->advance()){
                reachEnd = true;
                break;
            }
        } else if (larger()->toBool()){
            // If the left record is larger than the right record, advance the right record
            if (!matchedRightIter->advance()){
                reachEnd = true;
                break;
            }
        } else if (equal()->toBool()){
            // If the left record is equal to the right record on equalityCheck
            // Collect all possible records in the left iterator that has the same value on equalityCheck

            while(true){

                // Load the record into LeftRec2
                matchedLeftIter->getCurrent(leftRec1);

                // If two records equals
                if(!compLeftHelper1() && !compLeftHelper2()){
                    potentialMatches.push_back(matchedLeftIter->getCurrentPointer());
                }
                else {
                    break;
                }

                if (!matchedLeftIter->advance()){
                    reachEnd = true;
                    break;
                }

            }

            // Find all records with same key at the right table
            while(true){
                // Load the record into RightRec
                matchedRightIter->getCurrent(rightRec);

                if (equal()->toBool()){
                    for (auto &v: potentialMatches){

                        leftRec->fromBinary(v);

                        // Check if satisfy the final predicate
                        if(finalPredicate()-> toBool()){
                            int i = 0;

                            for (auto &f : finalComputations) {
                                outputRec->getAtt (i++)->set (f());
                            }

                            outputRec->recordContentHasChanged ();
                            cout << outputRec << endl;
                            output->append (outputRec);

                        }
                    }

                    if (!matchedRightIter->advance()){
                        reachEnd = true;
                        break;
                    }

                }
                else{
                    break;
                }
            }
        }

        potentialMatches.clear();
    }


}

#endif
