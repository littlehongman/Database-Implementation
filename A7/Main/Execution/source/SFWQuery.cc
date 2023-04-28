
#ifndef SFW_QUERY_CC
#define SFW_QUERY_CC

#include "ParserTypes.h"
//#include "Aggregate.h"
	
// builds and optimizes a logical query plan for a SFW query, returning the logical query plan
// 
// note that this implementation only works for two-table queries that do not have an aggregation
// 
LogicalOpPtr SFWQuery :: buildLogicalQueryPlan (map <string, MyDB_TablePtr> &allTables, map <string, MyDB_TableReaderWriterPtr> &allTableReaderWriters, map <string, MyDB_BPlusTreeReaderWriterPtr> &allBPlusReaderWriters) {

    // Place nullptr if BPlusTree is not applicable to the table
    for ( const auto &pair : allTableReaderWriters ) {
        if (allBPlusReaderWriters.find(pair.first) == allBPlusReaderWriters.end()){
            allBPlusReaderWriters[pair.first] = nullptr;
        }
    }

    // Check how many tables we currently have
    if (tablesToProcess.size() == 1) {
        return buildOneTablePlan(allTables, allTableReaderWriters, allBPlusReaderWriters);
    }

	else if (tablesToProcess.size () == 2){
        return buildTwoTablePlan(allTables, allTableReaderWriters, allBPlusReaderWriters);
	}

    return nullptr;

}

LogicalOpPtr SFWQuery :: buildOneTablePlan (map <string, MyDB_TablePtr> &allTables, map <string, MyDB_TableReaderWriterPtr> &allTableReaderWriters, map <string, MyDB_BPlusTreeReaderWriterPtr> &allBPlusReaderWriters) {


    // Check if aggregate elements exist
    bool areAggs = false;
    for (auto a : valuesToSelect) {
        if (a->hasAgg ()) {
            areAggs = true;
        }
    }
    if (groupingClauses.size () != 0) {
        areAggs = true;
    }

    // find the input tables
    MyDB_TablePtr inputTable = allTables[tablesToProcess[0].first];

    // We do not need to build a  CNF here, because we only have one table
    // CNF == allDisjunctions

    if (!areAggs){
        vector<string> topExprs;
        MyDB_SchemaPtr topSchema = make_shared<MyDB_Schema>();

        for (auto a: valuesToSelect) {
            for (auto b: inputTable->getSchema ()->getAtts ()) {
                if (a->referencesAtt (tablesToProcess[0].second, b.first)) {
                    topExprs.push_back ("[" + b.first + "]");
                    topSchema->getAtts ().push_back (make_pair (tablesToProcess[0].second + "_" + b.first, b.second));
                }
            }
        }

        MyDB_Record myRec (topSchema);

        // Get schema for output
        MyDB_SchemaPtr outputSchema = make_shared <MyDB_Schema> ();
        int i = 0;
        for (auto a: valuesToSelect) {
            outputSchema->getAtts ().push_back (make_pair ("att_" + to_string (i++), myRec.getType (a->toString ())));
        }

        LogicalOpPtr returnVal = nullptr;


        returnVal = make_shared <LogicalTableScan> (allTableReaderWriters[tablesToProcess[0].first],
                                                        make_shared <MyDB_Table> ("topTable", "topStorageLoc", outputSchema),
                                                        make_shared <MyDB_Stats> (inputTable, tablesToProcess[0].second), allDisjunctions, topExprs, tablesToProcess[0].second, allBPlusReaderWriters[tablesToProcess[0].first]);


        return returnVal;
    }
    else {
        vector<string> scanExprs;
        MyDB_SchemaPtr scanSchema = make_shared<MyDB_Schema>();

        for (auto a: valuesToSelect) {
            for (auto b: inputTable->getSchema ()->getAtts ()) {
                if (a->referencesAtt (tablesToProcess[0].second, b.first)) {
                    string valueStr = a->toString();
                    string typeStr = valueStr.substr(0, 3);

                    if (typeStr == "avg" || typeStr == "sum" || typeStr == "cnt"){
                        scanExprs.push_back ("[" + b.first + "]");
                    }
                    else{
                        scanExprs.push_back (a->toString());
                    }

                    scanSchema->getAtts ().push_back (make_pair (tablesToProcess[0].second + "_" + b.first, b.second));
                }
            }
        }

        MyDB_Record myRec (scanSchema);
//        vector<string> aggExprs;
        MyDB_SchemaPtr aggSchema = make_shared<MyDB_Schema>();

        int i = 0, j = 0;
        for (auto a: groupingClauses){
            aggSchema->getAtts().push_back(make_pair("group_" + to_string (i++), myRec.getType(a->toString())));
        }

        i = 0;
        for (auto a: valuesToSelect) {
            if (a->hasAgg()) {
                string exprStr = a->toString();
                string typeStr = exprStr.substr(0, 3);
                string columnStr = exprStr.substr(4, exprStr.length() - 1);

                if (typeStr == "avg" || typeStr == "sum" | typeStr == "cnt"){
                    aggSchema->getAtts().push_back(make_pair("agg_" + to_string (i++), myRec.getType(a->toString())));
                }

            }
        }


        // Get schema for output
        MyDB_SchemaPtr outputSchema = make_shared <MyDB_Schema> ();
        vector<string> outputExprs = {};
        i = 0;
        for (auto a: valuesToSelect) {
            if (!a->hasAgg()) {
                outputSchema->getAtts ().push_back (make_pair ("group_" + to_string (i), myRec.getType (a->toString ())));
                outputExprs.push_back("[group_" + to_string (i++) + ']');
            }
            else {
                outputSchema->getAtts ().push_back (make_pair ("agg_" + to_string (j), myRec.getType (a->toString ())));
                outputExprs.push_back("[agg_" + to_string (j++) + ']');
            }

        }

        LogicalOpPtr tableSelectionPtr = nullptr;


        tableSelectionPtr = make_shared <LogicalTableScan> (allTableReaderWriters[tablesToProcess[0].first],
                                                                make_shared <MyDB_Table> ("tempTable", "tempStorageLoc", scanSchema),
                                                                make_shared <MyDB_Stats> (inputTable, tablesToProcess[0].second), allDisjunctions, scanExprs, tablesToProcess[0].second, allBPlusReaderWriters[tablesToProcess[0].first]);


        LogicalOpPtr returnVal = make_shared <LogicalAggregate> (tableSelectionPtr, make_shared <MyDB_Table> ("aggTable", "aggStorageLoc", aggSchema), valuesToSelect, groupingClauses, make_shared <MyDB_Table> ("topTable", "topStorageLoc", outputSchema), outputExprs);

        return returnVal;
    }


}

LogicalOpPtr SFWQuery :: buildTwoTablePlan (map <string, MyDB_TablePtr> &allTables, map <string, MyDB_TableReaderWriterPtr> &allTableReaderWriters, map <string, MyDB_BPlusTreeReaderWriterPtr> &allBPlusReaderWriters) {


    // Check if agg exists
    bool areAggs = false;
    for (auto a : valuesToSelect) {
        if (a->hasAgg ()) {
            areAggs = true;
        }
    }
    if (groupingClauses.size () != 0 ) {
        areAggs = true;
    }

    // find the two input tables
    MyDB_TablePtr leftTable = allTables[tablesToProcess[0].first];
    MyDB_TablePtr rightTable = allTables[tablesToProcess[1].first];

    // find the various parts of the CNF
    //  leftCNF = all clauses in C referring only to attrs in left
    //  rightCNF = all clauses in C referring only to attrs in right
    //  topCNF = the rest of the clauses in C (those we cannot push down)
    vector <ExprTreePtr> leftCNF;
    vector <ExprTreePtr> rightCNF;
    vector <ExprTreePtr> topCNF;

    // loop through all of the disjunctions and break them apart
    for (auto a: allDisjunctions) {
        // Check the if the table is involved in the disjunction
        // (Here we only consider joining two tables), so we use index to tableToProcess
        bool inLeft = a->referencesTable (tablesToProcess[0].second);
        bool inRight = a->referencesTable (tablesToProcess[1].second);

        if (inLeft && inRight) {
            cout << "top " << a->toString () << "\n";
            topCNF.push_back (a);
        } else if (inLeft) {
            cout << "left: " << a->toString () << "\n";
            leftCNF.push_back (a);
        } else {
            cout << "right: " << a->toString () << "\n";
            rightCNF.push_back (a);
        }
    }

    // now get the left and right schemas for the two selections
    MyDB_SchemaPtr leftSchema = make_shared <MyDB_Schema> ();
    MyDB_SchemaPtr rightSchema = make_shared <MyDB_Schema> ();
    MyDB_SchemaPtr totSchema = make_shared <MyDB_Schema> ();
    vector <string> leftExprs;
    vector <string> rightExprs;

    // and see what we need from the left, and from the right
    for (auto b: leftTable->getSchema ()->getAtts ()) {
        bool needIt = false;
        for (auto a: valuesToSelect) {
            // If b in valuesToSelect
            if (a->referencesAtt (tablesToProcess[0].second, b.first)) {
                needIt = true;
            }
        }
        for (auto a: topCNF) {
            // If b in TopCNF
            if (a->referencesAtt (tablesToProcess[0].second, b.first)) {
                needIt = true;
            }
        }
        if (needIt) {
            leftSchema->getAtts ().push_back (make_pair (tablesToProcess[0].second + "_" + b.first, b.second));
            totSchema->getAtts ().push_back (make_pair (tablesToProcess[0].second + "_" + b.first, b.second));
            leftExprs.push_back ("[" + b.first + "]");
            cout << "left expr: " << ("[" + b.first + "]") << "\n";
        }
    }

    cout << "left schema: " << leftSchema << "\n";

    // and see what we need from the right, and from the right
    for (auto b: rightTable->getSchema ()->getAtts ()) {
        bool needIt = false;
        for (auto a: valuesToSelect) {
            if (a->referencesAtt (tablesToProcess[1].second, b.first)) {
                needIt = true;
            }
        }
        for (auto a: topCNF) {
            if (a->referencesAtt (tablesToProcess[1].second, b.first)) {
                needIt = true;
            }
        }
        if (needIt) {
            rightSchema->getAtts ().push_back (make_pair (tablesToProcess[1].second + "_" + b.first, b.second));
            totSchema->getAtts ().push_back (make_pair (tablesToProcess[1].second + "_" + b.first, b.second));
            rightExprs.push_back ("[" + b.first + "]");
            cout << "right expr: " << ("[" + b.first + "]") << "\n";
        }
    }
    cout << "right schema: " << rightSchema << "\n";

    // now we gotta figure out the top schema... get a record for the top
    MyDB_Record myRec (totSchema);

     //and get all of the attributes for the output
    MyDB_SchemaPtr topSchema = make_shared <MyDB_Schema> ();
    int i = 0, j= 0;
    for (auto a: valuesToSelect) {
        string attrStr = a->toString();

        topSchema->getAtts ().push_back (make_pair (attrStr.substr(1, attrStr.length() - 2), myRec.getType (a->toString ())));
    }
    cout << "tot schema: " << totSchema << "\n";
    cout << "top schema: " << topSchema << "\n";


    // Deal with Agg schema
    MyDB_SchemaPtr aggSchema = make_shared<MyDB_Schema>();

    i = 0, j = 0;
    for (auto a: groupingClauses){
        aggSchema->getAtts().push_back(make_pair("group_" + to_string (i++), myRec.getType(a->toString())));
    }

    i = 0;
    for (auto a: valuesToSelect) {
        if (a->hasAgg()) {
            string exprStr = a->toString();
            string typeStr = exprStr.substr(0, 3);
            string columnStr = exprStr.substr(4, exprStr.length() - 1);

            if (typeStr == "avg" || typeStr == "sum" | typeStr == "cnt"){
                aggSchema->getAtts().push_back(make_pair("agg_" + to_string (i++), myRec.getType(a->toString())));
            }

        }
    }

    // Get schema for output
    MyDB_SchemaPtr outputSchema = make_shared <MyDB_Schema> ();
    vector<string> outputExprs = {};
    i = 0;
    for (auto a: valuesToSelect) {
        if (!a->hasAgg()) {
            outputSchema->getAtts ().push_back (make_pair ("group_" + to_string (i), myRec.getType (a->toString ())));
            outputExprs.push_back("[group_" + to_string (i++) + ']');
        }
        else {
            outputSchema->getAtts ().push_back (make_pair ("agg_" + to_string (j), myRec.getType (a->toString ())));
            outputExprs.push_back("[agg_" + to_string (j++) + ']');
        }

    }


    // and it's time to build the query plan

    LogicalOpPtr leftTableScan = make_shared <LogicalTableScan> (allTableReaderWriters[tablesToProcess[0].first],
                                                                 make_shared <MyDB_Table> ("leftTable", "leftStorageLoc", leftSchema),
                                                                 make_shared <MyDB_Stats> (leftTable, tablesToProcess[0].second), leftCNF, leftExprs, tablesToProcess[0].second, allBPlusReaderWriters[tablesToProcess[0].first]);
    LogicalOpPtr rightTableScan = make_shared <LogicalTableScan> (allTableReaderWriters[tablesToProcess[1].first],
                                                                  make_shared <MyDB_Table> ("rightTable", "rightStorageLoc", rightSchema),
                                                                  make_shared <MyDB_Stats> (rightTable, tablesToProcess[1].second), rightCNF, rightExprs, tablesToProcess[1].second, allBPlusReaderWriters[tablesToProcess[1].first]);
    LogicalOpPtr returnVal = nullptr;

    if (!areAggs){
        returnVal = make_shared <LogicalJoin> (leftTableScan, rightTableScan,
                                                            make_shared <MyDB_Table> ("topTable", "topStorageLoc", topSchema), topCNF, valuesToSelect);
    }

    else{
        LogicalOpPtr tableJoinPtr = make_shared <LogicalJoin> (leftTableScan, rightTableScan,
                                                            make_shared <MyDB_Table> ("topTable", "topStorageLoc", topSchema), topCNF, valuesToSelect);

        returnVal = make_shared <LogicalAggregate> (tableJoinPtr, make_shared <MyDB_Table> ("aggTable", "aggStorageLoc", aggSchema), valuesToSelect, groupingClauses, make_shared <MyDB_Table> ("aggTable", "aggStorageLoc", outputSchema), outputExprs);
    }



    // done!!
    return returnVal;
}


void SFWQuery :: print () {
	cout << "Selecting the following:\n";
	for (auto a : valuesToSelect) {
		cout << "\t" << a->toString () << "\n";
	}
	cout << "From the following:\n";
	for (auto a : tablesToProcess) {
		cout << "\t" << a.first << " AS " << a.second << "\n";
	}
	cout << "Where the following are true:\n";
	for (auto a : allDisjunctions) {
		cout << "\t" << a->toString () << "\n";
	}
	cout << "Group using:\n";
	for (auto a : groupingClauses) {
		cout << "\t" << a->toString () << "\n";
	}
}


SFWQuery :: SFWQuery (struct ValueList *selectClause, struct FromList *fromClause,
        struct CNF *cnf, struct ValueList *grouping) {
        valuesToSelect = selectClause->valuesToCompute;
        tablesToProcess = fromClause->aliases;
        allDisjunctions = cnf->disjunctions;
        groupingClauses = grouping->valuesToCompute;
}

SFWQuery :: SFWQuery (struct ValueList *selectClause, struct FromList *fromClause,
        struct CNF *cnf) {
        valuesToSelect = selectClause->valuesToCompute;
        tablesToProcess = fromClause->aliases;
	allDisjunctions = cnf->disjunctions;
}

SFWQuery :: SFWQuery (struct ValueList *selectClause, struct FromList *fromClause) {
        valuesToSelect = selectClause->valuesToCompute;
        tablesToProcess = fromClause->aliases;
        allDisjunctions.push_back (make_shared <BoolLiteral> (true));
}

#endif
