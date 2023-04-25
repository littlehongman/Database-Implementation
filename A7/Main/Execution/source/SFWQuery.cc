
#ifndef SFW_QUERY_CC
#define SFW_QUERY_CC

#include "ParserTypes.h"
//#include "Aggregate.h"
	
// builds and optimizes a logical query plan for a SFW query, returning the logical query plan
// 
// note that this implementation only works for two-table queries that do not have an aggregation
// 
LogicalOpPtr SFWQuery :: buildLogicalQueryPlan (map <string, MyDB_TablePtr> &allTables, map <string, MyDB_TableReaderWriterPtr> &allTableReaderWriters) {

    // Check how many tables we currently have
    if (tablesToProcess.size() == 1) {
        return buildOneTablePlan(allTables, allTableReaderWriters);
    }

	else if (tablesToProcess.size () == 2){
        return buildTwoTablePlan(allTables, allTableReaderWriters);
	}

    return nullptr;

}

LogicalOpPtr SFWQuery :: buildOneTablePlan (map <string, MyDB_TablePtr> &allTables, map <string, MyDB_TableReaderWriterPtr> &allTableReaderWriters) {



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

    // find the two input tables
    MyDB_TablePtr topTable = allTables[tablesToProcess[0].first];

    // We do not need to build a  CNF here, because we only have one table
    // CNF == allDisjunctions

    vector<string> topExprs;
    MyDB_SchemaPtr topSchema = make_shared<MyDB_Schema>();
    MyDB_SchemaPtr totSchema = make_shared<MyDB_Schema>();

    for (auto b: topTable->getSchema ()->getAtts ()) {
        bool needInSchema = false;
        bool needInExpr = false;

        for (auto a: valuesToSelect) {

            // If a is not an Agg, then just push_back th
            if (a->referencesAtt (tablesToProcess[0].second, b.first)) {
                needInSchema = true;
                needInExpr = true;
            }
        }

//        for (auto a: allDisjunctions){
//            if (a->referencesAtt (tablesToProcess[0].second, b.first)) {
//                needInSchema = true;
//            }
//        }

        // We also want to include grouping clauses
        for (auto a: groupingClauses){
            if (a->referencesAtt (tablesToProcess[0].second, b.first)) {
                needInSchema = true;
                needInExpr = true;
            }
        }

        if (needInSchema){
            topSchema->getAtts ().push_back (make_pair (tablesToProcess[0].second + "_" + b.first, b.second));
            cout <<  tablesToProcess[0].second + "_" + b.first << endl;
        }

        if (needInExpr) {
            topExprs.push_back ("[" + b.first + "]");
        }

        //totSchema->getAtts ().push_back (make_pair (tablesToProcess[0].second + "_" + b.first, b.second));
    }


    MyDB_Record myRec (topSchema);

    // and get all of the attributes for the output
    MyDB_SchemaPtr outputSchema = make_shared <MyDB_Schema> ();
    int i = 0;
    for (auto a: valuesToSelect) {
        outputSchema->getAtts ().push_back (make_pair ("att_" + to_string (i++), myRec.getType (a->toString ())));
    }

    if (!areAggs){
        LogicalOpPtr returnVal = make_shared <LogicalTableScan> (allTableReaderWriters[tablesToProcess[0].first],
                                                                 make_shared <MyDB_Table> ("topTable", "topStorageLoc", outputSchema),
                                                                 make_shared <MyDB_Stats> (topTable, tablesToProcess[0].second), allDisjunctions, topExprs, tablesToProcess[0].second);

        return returnVal;
    }
    else {
        LogicalOpPtr tableSelectionPtr = make_shared <LogicalTableScan> (allTableReaderWriters[tablesToProcess[0].first],
                                                                 make_shared <MyDB_Table> ("tempTable", "tempStorageLoc", topSchema),
                                                                 make_shared <MyDB_Stats> (topTable, tablesToProcess[0].second), allDisjunctions, topExprs, tablesToProcess[0].second);


        LogicalOpPtr returnVal = make_shared <LogicalAggregate> (tableSelectionPtr, make_shared <MyDB_Table> ("topTable", "topStorageLoc", outputSchema), valuesToSelect, groupingClauses);

        return returnVal;
    }


}

LogicalOpPtr SFWQuery :: buildTwoTablePlan (map <string, MyDB_TablePtr> &allTables, map <string, MyDB_TableReaderWriterPtr> &allTableReaderWriters) {


    // also, make sure that there are no aggregates in here
    bool areAggs = false;
    for (auto a : valuesToSelect) {
        if (a->hasAgg ()) {
            areAggs = true;
        }
    }
    if (groupingClauses.size () != 0 || areAggs) {
        cout << "Sorry, we can't handle aggs or groupings yet!\n";
        return nullptr;
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

    // and get all of the attributes for the output
    MyDB_SchemaPtr topSchema = make_shared <MyDB_Schema> ();
    int i = 0;
    for (auto a: valuesToSelect) {
        topSchema->getAtts ().push_back (make_pair ("att_" + to_string (i++), myRec.getType (a->toString ())));
    }
    cout << "tot schema: " << totSchema << "\n";
    cout << "top schema: " << topSchema << "\n";

    // Build a hash map that store table alias and its corresponding tableReaderWriter
    map<string, MyDB_TableReaderWriterPtr> usedTableReaderWriters;

    usedTableReaderWriters[tablesToProcess[0].second] = allTableReaderWriters[tablesToProcess[0].first];
    usedTableReaderWriters[tablesToProcess[1].second] = allTableReaderWriters[tablesToProcess[1].first];


    // and it's time to build the query plan
    LogicalOpPtr leftTableScan = make_shared <LogicalTableScan> (allTableReaderWriters[tablesToProcess[0].first],
                                                                 make_shared <MyDB_Table> ("leftTable", "leftStorageLoc", leftSchema),
                                                                 make_shared <MyDB_Stats> (leftTable, tablesToProcess[0].second), leftCNF, leftExprs, tablesToProcess[0].second);
    LogicalOpPtr rightTableScan = make_shared <LogicalTableScan> (allTableReaderWriters[tablesToProcess[1].first],
                                                                  make_shared <MyDB_Table> ("rightTable", "rightStorageLoc", rightSchema),
                                                                  make_shared <MyDB_Stats> (rightTable, tablesToProcess[1].second), rightCNF, rightExprs, tablesToProcess[1].second);
    LogicalOpPtr returnVal = make_shared <LogicalJoin> (leftTableScan, rightTableScan,
                                                        make_shared <MyDB_Table> ("topTable", "topStorageLoc", topSchema), topCNF, valuesToSelect, usedTableReaderWriters);

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
