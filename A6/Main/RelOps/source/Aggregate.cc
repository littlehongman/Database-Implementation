
#ifndef AGG_CC
#define AGG_CC

#include "MyDB_Record.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableReaderWriter.h"
#include "Aggregate.h"
#include <unordered_map>

using namespace std;

Aggregate :: Aggregate (MyDB_TableReaderWriterPtr input, MyDB_TableReaderWriterPtr output,
                vector <pair <MyDB_AggType, string>> aggsToCompute,
                vector <string> groupings, string selectionPredicate) {

    this->inputTable = input;
    this->outputTable = output;
    this->aggsToCompute = aggsToCompute;
    this->groupings = groupings;
    this->selectionPredicate = selectionPredicate;

}

void Aggregate :: run () {
    // 0. Check if the number of attributes match the input arguments
    if (outputTable->getTable ()->getSchema ()->getAtts ().size () != aggsToCompute.size () + groupings.size ()) {
        cout << "ERROR: the output schema needs to have the same number of atts as (# of aggs to compute + # groups).\n";
        return;
    }


    // 1. Create One big schema that can store all the grouping attributes and the aggregates
    // Use the combined schema so that you can easily calculate values
    MyDB_SchemaPtr aggSchema = make_shared <MyDB_Schema> (); // The schema of the output table
    MyDB_SchemaPtr combinedSchema = make_shared <MyDB_Schema> ();
    int numGroupings = groupings.size(), i = 0;

    // Get output schema
    // MyDB_group => the grouping attributes
    // MyDB_agg => the aggregate attributes

    for (auto &p : outputTable->getTable()->getSchema()->getAtts()){
        if (i < numGroupings){
            aggSchema->appendAtt(make_pair("MyDB_group_" + to_string(i + 1), p.second));
        }
        else {
            aggSchema->appendAtt(make_pair("MyDB_agg_" + to_string(i - numGroupings + 1), p.second));
        }

        i++;
    }

    // Add an extra attribute MyDB_cnt for the final calculation for AVG
    aggSchema->appendAtt(make_pair("MyDB_cnt", make_shared <MyDB_IntAttType> ()));

    // Get combined schema
    for (auto &p : inputTable->getTable()->getSchema()->getAtts()){
        combinedSchema->appendAtt(p);
    }
    for (auto &p : aggSchema->getAtts()){
        combinedSchema->appendAtt(p);
    }

    // 2. Create a combined record that consists of record from the input table and the outputSchema
    MyDB_RecordPtr inputRec = inputTable->getEmptyRecord();
    MyDB_RecordPtr aggRec = make_shared<MyDB_Record>(aggSchema);
    MyDB_RecordPtr combinedRec = make_shared <MyDB_Record> (combinedSchema);
    MyDB_RecordPtr outputRec = outputTable->getEmptyRecord();

    // Caution: Aggregate records don't have exactly the same schema as the output records, because we have MYDB_CNT

    combinedRec->buildFrom(inputRec, aggRec);

    vector<func> aggComputations = {};
    vector<func> groupComputations = {};
    vector<func> outputComputations = {};
     // Reset i

    // Store the computation for each aggregate function
    i = 1;
    for (auto &p : aggsToCompute){
        if (p.first == MyDB_AggType :: sum || p.first == MyDB_AggType :: avg) {
            aggComputations.push_back(combinedRec->compileComputation("+ (" + p.second + ", [MyDB_agg_" + to_string(i++) + "])"));
        }
        else if (p.first == MyDB_AggType :: cnt) {
            aggComputations.push_back(combinedRec->compileComputation("+ (int[1], [MyDB_agg_" + to_string(i++) + "])"));
        }
    }

    // Don't forget to add the computation for the MyDB_cnt
    aggComputations.push_back(combinedRec->compileComputation("+ (int[1], [MyDB_cnt])"));

    // Store the computation for the final calculation
    i = 1;
    for (auto &p : aggsToCompute){
        if (p.first == MyDB_AggType :: sum || p.first == MyDB_AggType :: cnt) {
            outputComputations.push_back(combinedRec->compileComputation("[MyDB_agg_" + to_string(i++) + "]"));
        }
        else if (p.first == MyDB_AggType :: avg) {
            outputComputations.push_back(combinedRec->compileComputation("/ ([MyDB_agg_" + to_string(i++) + "], [MyDB_cnt])"));
        }
    }

    // Use these to calculate the hash value
    for (auto &s : groupings){
        groupComputations.push_back(combinedRec->compileComputation(s));
    }

    // EXTRA: We need a func to make sure that the inputRec matches the one we are updating
    string matchingPredicate = "";

    if (numGroupings == 0)
        matchingPredicate += "bool[true]";
    else if (numGroupings == 1){
        matchingPredicate += " == ([MyDB_group_" + to_string(1) + "], " + groupings[0] + ")";
    }
    else{
        matchingPredicate += "&& (";

        for (int j = 0; j < numGroupings; j++){
            matchingPredicate += " == ([MyDB_group_" + to_string(j + 1) + "], " + groupings[j] + ")";

            if (j != numGroupings - 1){
                matchingPredicate += ", ";
            }
        }
        matchingPredicate += ")";
    }
    // Create func for the later match checking
    func matchPred = combinedRec->compileComputation(matchingPredicate);

    // 3. Iterate records from input and create hashMap
    // Create func to run selection predicate
    func pred = inputRec->compileComputation(selectionPredicate);

    // Create an iterator over input table
    MyDB_RecordIteratorPtr inputIter = inputTable->getIterator(inputRec);

    // Hashmap to store the records
    // The value should be a VECTOR, because the hash value can collide
    unordered_map<size_t, vector<void*>> hashMap;

    // Create vector of pages to store the records
    // Caution: Of course, we can the hash table to retrieve all record, HOWEVER the insert order will not be maintained
    vector <MyDB_PageReaderWriter> pages;

    // Create a PINNED page
    MyDB_PageReaderWriter currPage(true, *outputTable->getBufferMgr());
    pages.push_back(currPage);
    MyDB_AttValPtr defaultValue = make_shared <MyDB_IntAttVal> ();

    // Iterate over the input table
    while (inputIter->hasNext()){
        inputIter->getNext();

        // Check if the record satisfies the predicate
        if(!pred()->toBool()){
            continue;
        }

//        cout << inputRec << endl;

        // Calculate the hash value
        size_t hashVal = 0;
        for (auto &f : groupComputations){
            hashVal ^= f()->hash();
        }

        // If the hash value is not in the hashMap, create a new record
        if (hashMap.find(hashVal) == hashMap.end()){
            // Create a new record
            int idx = 0;

            // First load the grouping attributes value
            for (auto &f: groupComputations){
                aggRec->getAtt(idx++)->set(f());
            }

            // First set all value to zero, so that we can accumulate the value
            for (auto &f: aggComputations){
                aggRec->getAtt(idx++)->set(defaultValue);
            }

            idx = 0;

            for (auto &f: aggComputations){
                aggRec->getAtt(numGroupings + idx)->set(f());
                idx++;
            }

            aggRec->recordContentHasChanged();

            // Write the record to the page
            void* addr = currPage.appendAndReturnLocation(aggRec);

            if (addr == nullptr){
                // If the page is full, create a new page
                MyDB_PageReaderWriter newPage(true, *outputTable->getBufferMgr());

                // First push_back, then assign currPage to newPage
                pages.push_back(newPage);
                currPage = newPage;

                addr = currPage.appendAndReturnLocation(aggRec);
            }

            // Store the record into the hashTable
            hashMap[hashVal].push_back(addr);

        }
        // If the hash value is in the hashMap
        else{
            bool found = false;
            // First check if there exists a record that matches the grouping attributes of inputRec
            for (auto& addr: hashMap[hashVal]){
                aggRec->fromBinary(addr);

                if (!matchPred()->toBool()){
                    continue;
                }

                // If there is a match, update the record
                found = true;
                int idx = 0;

                for (auto &f: groupComputations){
                    aggRec->getAtt(idx++)->set(f());
                }

                for (auto &f: aggComputations){
                    aggRec->getAtt(idx++)->set(f());
                }

                aggRec->recordContentHasChanged();

                // Update the record in the hashMap
                aggRec->toBinary(addr);


                break;
            }

            // If there is no match, create a new record
            if (!found){
                // Create a new record
                int idx = 0;

                // First load the grouping attributes value
                for (auto &f: groupComputations){
                    aggRec->getAtt(idx++)->set(f());
                }

                // First set all value to zero, so that we can accumulate the value
                for (auto &f: aggComputations){
                    aggRec->getAtt(idx++)->set(defaultValue);
                }

                idx = 0;

                for (auto &f: aggComputations){
                    aggRec->getAtt(idx + numGroupings)->set(f());
                    idx ++;
                }

                aggRec->recordContentHasChanged();

                // Write the record to the page
                void* addr = currPage.appendAndReturnLocation(aggRec);

                if (addr == nullptr){
                    // If the page is full, create a new page
                    MyDB_PageReaderWriter newPage(true, *outputTable->getBufferMgr());

                    // First push_back, then assign currPage to newPage
                    pages.push_back(currPage);
                    currPage = newPage;

                    addr = currPage.appendAndReturnLocation(aggRec);
                }

                // Store the record into the hashTable
                hashMap[hashVal].push_back(addr);
            }

        }
    }

    // 4. Write the records to the output table
    // We can achieve this by loop through all the records in the pages
    MyDB_RecordIteratorAltPtr outputIter = getIteratorAlt(pages);
    while(outputIter->advance()){

        // Load the record from the page
        outputIter->getCurrent(aggRec);

        for (i = 0; i < numGroupings; i++){
            outputRec->getAtt(i)->set(aggRec->getAtt(i));
        }

        for (auto &f: outputComputations){
            outputRec->getAtt(i++)->set(f());
        }


        outputRec->recordContentHasChanged();
        outputTable->append(outputRec);

    }
}

#endif

