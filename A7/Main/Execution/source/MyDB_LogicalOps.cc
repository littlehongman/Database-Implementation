
#ifndef LOG_OP_CC
#define LOG_OP_CC

#include "MyDB_LogicalOps.h"
#include "RegularSelection.h"
#include "SortMergeJoin.h"
#include "ScanJoin.h"
#include "Aggregate.h"
#include "BPlusSelection.h"

// fill this out!  This should actually run the aggregation via an appropriate RelOp, and then it is going to
// have to unscramble the output attributes and compute exprsToCompute using an execution of the RegularSelection 
// operation (why?  Note that the aggregate always outputs all of the grouping atts followed by the agg atts.
// After, a selection is required to compute the final set of aggregate expressions)
//
// Note that after the left and right hand sides have been executed, the temporary tables associated with the two 
// sides should be deleted (via a kill to killFile () on the buffer manager)
MyDB_TableReaderWriterPtr LogicalAggregate :: execute () {

    // First run the underlying operations
    MyDB_TableReaderWriterPtr inputTablePtr = inputOp->execute();

//    // print out the results
//    MyDB_RecordPtr temp = inputTablePtr->getEmptyRecord();
//    MyDB_RecordIteratorAltPtr myIter = inputTablePtr->getIteratorAlt();
//
//    // Counter => only output the first 30 results
//    int count = 0;
//
//    while (myIter->advance()) {
//        myIter->getCurrent(temp);
//
//        cout << temp << "\n";
//        count ++;
//    }
//    cout << count << endl;
//

    // Define all the parameters needed for the JOIN
    // (1) Create an outputTable ReaderWriter
    MyDB_TableReaderWriterPtr aggTablePtr = make_shared<MyDB_TableReaderWriter>(aggSpec, inputTablePtr->getBufferMgr());

    // (2) Create agg pairs to compute
    vector<pair<MyDB_AggType, string>> aggsToCompute;

    for (auto a: exprsToCompute) {
        if (a->hasAgg()) {
            string exprStr = a->toString();
            string typeStr = exprStr.substr(0, 3);
            string columnStr = exprStr.substr(4, exprStr.length() - 1);

            if (typeStr == "avg"){
                aggsToCompute.push_back(make_pair(MyDB_AggType::avg, columnStr));
            }
            else if (typeStr == "sum"){
                aggsToCompute.push_back(make_pair(MyDB_AggType::sum, columnStr));
            }
            else if (typeStr == "cnt"){
                aggsToCompute.push_back(make_pair(MyDB_AggType::cnt, columnStr));
            }
        }
    }

    // (3) Turn Grouping ExprTrees into strings
    vector<string> groupingStrs;

    for (auto a: groupings){
        groupingStrs.push_back(a->toString());
    }

    Aggregate myOp(inputTablePtr, aggTablePtr, aggsToCompute ,groupingStrs, "bool[true]");
    myOp.run();

//    // print out the results
//    MyDB_RecordPtr temp = aggTablePtr->getEmptyRecord();
//    MyDB_RecordIteratorAltPtr myIter = aggTablePtr->getIteratorAlt();
//
//    // Counter => only output the first 30 results
//    int count = 0;
//
//    while (myIter->advance()) {
//        myIter->getCurrent(temp);
//
//        cout << temp << "\n";
//        count ++;
//    }
//    cout << count << endl;



    // Run the last RegularSection -> To get correctly ordering attributes
    MyDB_TableReaderWriterPtr outputTablePtr = make_shared<MyDB_TableReaderWriter>(outputSpec, inputTablePtr->getBufferMgr());

    RegularSelection mySelectionOp(aggTablePtr, outputTablePtr, "bool[true]", finalExprsToCompute);
    mySelectionOp.run();

	return outputTablePtr;
}

// we don't really count the cost of the aggregate, so cost its subplan and return that
pair <double, MyDB_StatsPtr> LogicalAggregate :: cost () {
	return inputOp->cost ();
}
	
// this costs the entire query plan with the join at the top, returning the compute set of statistics for
// the output.  Note that it recursively costs the left and then the right, before using the statistics from
// the left and the right to cost the join itself
pair <double, MyDB_StatsPtr> LogicalJoin :: cost () {
	auto left = leftInputOp->cost ();
	auto right = rightInputOp->cost ();
	MyDB_StatsPtr outputStats = left.second->costJoin (outputSelectionPredicate, right.second);
	return make_pair (left.first + right.first + outputStats->getTupleCount (), outputStats);
}
	
// Fill this out!  This should recursively execute the left hand side, and then the right hand side, and then
// it should heuristically choose whether to do a scan join or a sort-merge join (if it chooses a scan join, it
// should use a heuristic to choose which input is to be hashed and which is to be scanned), and execute the join.
// Note that after the left and right hand sides have been executed, the temporary tables associated with the two 
// sides should be deleted (via a kill to killFile () on the buffer manager)
MyDB_TableReaderWriterPtr LogicalJoin :: execute () {

    // Run recursively; first execute the sub-plans (subtree)
    MyDB_TableReaderWriterPtr leftTable = leftInputOp->execute();
    MyDB_TableReaderWriterPtr rightTable = rightInputOp->execute();


    // Implement simple heuristic to choose which join method
    // If the left table is smaller than the memory size => ScanJoin()
    // Else => SortMergeJoin(

    // First define all the parameters needed for the JOIN
    // (1) Create an outputTable ReaderWriter
    MyDB_TableReaderWriterPtr outputTablePtr = make_shared<MyDB_TableReaderWriter>(outputSpec, leftTable->getBufferMgr());

    // (2) Transform vector of CNF to string (predicate)
    string predicate = this->predicateToString(outputSelectionPredicate);

    // (2.5) transform CNF string to the format that can read by RelOps
//    string predicateRelOps = this->ExprTreeToRelOps(predicate);


    // (3) Get projections
    vector<string> projections;

    for (auto &a : exprsToCompute) {
        projections.push_back(a->toString());
    }

    // (4) Get hasAttrs
    vector<pair<string, string>> hashAtts;

    for (auto &a : outputSelectionPredicate) {
        string attrName1 = a->getLHS()->toString(), attrName2 = a->getRHS()->toString();


        // The attribute is the left table
        if (leftTable->getTable()->getSchema()->getAttByName(attrName1.substr(1, attrName1.length() - 2)).first != -1){

            hashAtts.push_back(make_pair(attrName1, attrName2));
        }
        else{
            hashAtts.push_back(make_pair(attrName2, attrName1));
        }
    }


    SortMergeJoin myOp(leftTable, rightTable, outputTablePtr, predicate, projections, hashAtts[0], "bool[true]", "bool[true]");
    myOp.run();

	return outputTablePtr;
}

//string LogicalJoin::ExprTreeToRelOps(string input) {
//    for (auto it = aliasToTableReaderWriters.begin(); it != aliasToTableReaderWriters.end(); ++it) {
//        std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
//
//
//        for (auto b: it->second->getTable()->getSchema()->getAtts()) {
//            string toReplace = it->first + "_" + b.first;
//            size_t pos = input.find(toReplace);
//
//            if (pos == std::string::npos){
//                continue;
//            }
//
//            input.replace(pos, toReplace.length(), b.first);
//        }
//    }
//
//    return input;
//}

// this costs the table scan returning the compute set of statistics for the output
pair <double, MyDB_StatsPtr> LogicalTableScan :: cost () {
	MyDB_StatsPtr returnVal = inputStats->costSelection (selectionPred);
	return make_pair (returnVal->getTupleCount (), returnVal);	
}

// fill this out!  This should heuristically choose whether to use a B+-Tree (if appropriate) or just a regular
// table scan, and then execute the table scan using a relational selection.  Note that a desirable optimization
// is to somehow set things up so that if a B+-Tree is NOT used, that the table scan does not actually do anything,
// and the selection predicate is handled at the level of the parent (by filtering, for example, the data that is
// input into a join)
MyDB_TableReaderWriterPtr LogicalTableScan :: execute () {
    // In this version, we first implement only regular selection

    // First define all the parameters needed for the RegularSelection
    // (1) Create an outputTable ReaderWriter
    MyDB_TableReaderWriterPtr outputTablePtr = make_shared<MyDB_TableReaderWriter>(outputSpec, inputSpec->getBufferMgr());

    // (2) Transform vector of CNF to string (predicate)
    string predicate = this->predicateToString(selectionPred);

    // (2.5) transform CNF string to the format that can read by RelOps
    for (auto b: inputSpec->getTable()->getSchema()->getAtts()) {
        string toReplace = inputTableAlias + "_" + b.first;

        size_t start_pos = 0;

        while((start_pos = predicate.find(toReplace, start_pos)) != std::string::npos) {
            predicate.replace(start_pos, toReplace.length(), b.first);
            start_pos += b.first.length(); // Handles case where 'to' is a substring of 'from'
        }

    }

    // (3) transform projection strings to the format that can read by RelOps
    for (auto b: inputSpec->getTable()->getSchema()->getAtts()) {

        for (int i = 0; i < exprsToCompute.size(); i++){
            string projection = exprsToCompute[i];

            string toReplace = inputTableAlias + "_" + b.first;
            size_t start_pos = 0;

            while((start_pos = projection.find(toReplace, start_pos)) != std::string::npos) {
                projection.replace(start_pos, toReplace.length(), b.first);
                start_pos += b.first.length(); // Handles case where 'to' is a substring of 'from'
            }

            exprsToCompute[i] = projection;
        }
    }

    if (inputSpec->getTable()->getFileType() == "heap"){
        RegularSelection myOp(inputSpec, outputTablePtr, predicate, exprsToCompute);
        myOp.run();
    }
    else if (inputSpec->getTable()->getFileType() == "bplustree"){
        string sortAtt = inputSpec->getTable()->getSortAtt();
        MyDB_StringAttValPtr low = make_shared<MyDB_StringAttVal>();
        MyDB_StringAttValPtr high = make_shared<MyDB_StringAttVal>();

        low->set("");
        high->set("~~~~~~~~~");

        for (auto a: selectionPred) {
            // Check the if the table is involved in the disjunction
            // (Here we only consider joining two tables), so we use index to tableToProcess
            if (a->referencesAtt (inputTableAlias, sortAtt)){
                if (a->isEq()){
                    string attStr = a->getRHS()->toString();
                    string att = attStr.substr(7, attStr.length()-8);

                    low->set(att);
                    high->set(att);
                }
            }
        }




        BPlusSelection myOp(BPlusInputSpec, outputTablePtr, low, high,
                            predicate,
                            exprsToCompute);

        myOp.run();
    }



	return outputTablePtr;
}

#endif
