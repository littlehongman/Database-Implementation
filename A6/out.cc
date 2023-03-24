//
// Created by Hung-Chieh Wu on 3/13/23.
//


// ALTERNATIVE 1: Use the built-in operator to do the sorted records with predicate selection

// Get sorted left input table that satisfies left selection predicate
MyDB_RecordPtr leftRec1 = leftInput->getEmptyRecord ();
MyDB_RecordPtr leftRec2 = leftInput->getEmptyRecord ();

function <bool ()> compLeft = buildRecordComparator (leftRec1, leftRec2, equalityCheck.first);

MyDB_RecordIteratorAltPtr matchedLeftIterator = buildItertorOverSortedRuns(64, *leftInput, compLeft, leftRec1, leftRec2, leftSelectionPredicate);

// Get sorted right input table that satisfies right selection predicate
MyDB_RecordPtr rightRec1 = rightInput->getEmptyRecord ();
MyDB_RecordPtr rightRec2 = rightInput->getEmptyRecord ();

function <bool ()> compRight = buildRecordComparator (rightRec1, rightRec2, equalityCheck.second);

MyDB_RecordIteratorAltPtr matchedRightIterator = buildItertorOverSortedRuns(64, *rightInput, compRight, rightRec1, rightRec2, rightSelectionPredicate);
