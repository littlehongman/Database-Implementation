
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
}

void SortMergeJoin :: run () {


    // Get sorted left input table that satisfies left selection predicate
    MyDB_RecordPtr leftRec1 = leftInput->getEmptyRecord ();
    MyDB_RecordPtr leftRec2 = leftInput->getEmptyRecord ();

    function <bool ()> compLeft = buildRecordComparator (leftRec1, leftRec2, equalityCheck.first);

    MyDB_RecordIteratorAltPtr matchedLeftIter = buildItertorOverSortedRuns(leftInput->getBufferMgr()->numPages / 2, *leftInput, compLeft, leftRec1, leftRec2, leftSelectionPredicate);

    // Get sorted right input table that satisfies right selection predicate
    MyDB_RecordPtr rightRec1 = rightInput->getEmptyRecord ();
    MyDB_RecordPtr rightRec2 = rightInput->getEmptyRecord ();

    function <bool ()> compRight = buildRecordComparator (rightRec1, rightRec2, equalityCheck.second);

    MyDB_RecordIteratorAltPtr matchedRightIter = buildItertorOverSortedRuns(leftInput->getBufferMgr()->numPages / 2, *rightInput, compRight, rightRec1, rightRec2, rightSelectionPredicate);

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

    // Create a helper comparator to get records that same values
    // If (!compareLeftHelper1 && !compareLeftHelper2) => Two records equal
    function <bool ()> compLeftHelper1 = buildRecordComparator (leftRec, leftRec1, equalityCheck.first);
    function <bool ()> compLeftHelper2 = buildRecordComparator (leftRec1, leftRec, equalityCheck.first);


    // Create variables to keep track of the current records
    // ATTENTION: This method will not work because the whole vector is stored in RAM,
    // HOWEVER, the number of potentialMatches may exceed the RAM size
    // vector<void*> potentialMatches = {};

    // Alternative way to store the potential match records
    vector<MyDB_PageReaderWriter> potentalMatchesPages = {};
    MyDB_PageReaderWriter currPage(true, *output->getBufferMgr());
    potentalMatchesPages.push_back(currPage);

    bool reachEnd = false;

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

                    if (!currPage.append(leftRec1)){
                        MyDB_PageReaderWriter newPage(true, *output->getBufferMgr());
                        potentalMatchesPages.push_back(newPage);

                        currPage = newPage;

                        currPage.append(leftRec1);
                    }

//                    potentialMatches.push_back(matchedLeftIter->getCurrentPointer());
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
                    MyDB_RecordIteratorAltPtr potentialMatchIter = getIteratorAlt(potentalMatchesPages);

                    while (potentialMatchIter->advance()){

                        potentialMatchIter->getCurrent(leftRec);
//                        leftRec->fromBinary(v);

                        // Check if satisfy the final predicate
                        if(finalPredicate()-> toBool()){
                            int i = 0;

                            for (auto &f : finalComputations) {
                                outputRec->getAtt (i++)->set (f());
                            }

                            outputRec->recordContentHasChanged ();
//                            cout << outputRec << endl;
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

            // After matching
            potentalMatchesPages.clear();
            currPage.clear();

            potentalMatchesPages.push_back(currPage);
        }


    }



}

#endif
