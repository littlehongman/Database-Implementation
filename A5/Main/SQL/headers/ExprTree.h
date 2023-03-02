
#ifndef SQL_EXPRESSIONS
#define SQL_EXPRESSIONS

#include "MyDB_AttType.h"
#include "MyDB_Catalog.h"
#include <string>
#include <vector>
#include <unordered_map>

// create a smart pointer for database tables
using namespace std;
class ExprTree;
typedef shared_ptr <ExprTree> ExprTreePtr;

// this class encapsules a parsed SQL expression (such as "this.that > 34.5 AND 4 = 5")

// class ExprTree is a pure virtual class... the various classes that implement it are below
class ExprTree {

public:
	virtual string toString () = 0;

    // This will check Make sure that all of the referenced attributes exist, and are correctly attached to the tables that are indicated in the query.
    virtual bool isValid(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap) = 0;

	virtual ~ExprTree () {}
};

class BoolLiteral : public ExprTree {

private:
	bool myVal;
public:
	
	BoolLiteral (bool fromMe) {
		myVal = fromMe;
	}

	string toString () {
		if (myVal) {
			return "bool[true]";
		} else {
			return "bool[false]";
		}
	}

    bool isValid(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        return true;
    }
};

class DoubleLiteral : public ExprTree {

private:
	double myVal;
public:

	DoubleLiteral (double fromMe) {
		myVal = fromMe;
	}

	string toString () {
		return "double[" + to_string (myVal) + "]";
	}

    bool isValid(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        return true;
    }

	~DoubleLiteral () {}
};

// this implement class ExprTree
class IntLiteral : public ExprTree {

private:
	int myVal;
public:

	IntLiteral (int fromMe) {
		myVal = fromMe;
	}

	string toString () {
		return "int[" + to_string (myVal) + "]";
	}

    bool isValid(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        return true;
    }

	~IntLiteral () {}
};

class StringLiteral : public ExprTree {

private:
	string myVal;
public:

	StringLiteral (char *fromMe) {
		fromMe[strlen (fromMe) - 1] = 0;
		myVal = string (fromMe + 1);
	}

	string toString () {
		return "string[" + myVal + "]";
	}

    bool isValid(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        return true;
    }

	~StringLiteral () {}
};

class Identifier : public ExprTree {

private:
	string tableName;
	string attName;
public:

	Identifier (char *tableNameIn, char *attNameIn) {
		tableName = string (tableNameIn);
		attName = string (attNameIn);
	}

	string toString () {
		return "[" + tableName + "_" + attName + "]";
	}

    bool isValid(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        // Check if the alias exists
        if (aliasMap.find(tableName) == aliasMap.end()){
            cout << "The alias " << tableName << " does not exist in the table" << endl;
            return false;
        }

        string key = aliasMap[tableName] + "." + attName + "." + "type";
        string temp = "";

        // Check if the attributes exist in the corresponding table
        if (!myCatalog->getString(key, temp)){
            cout << "The attribute " << attName << " does not exist in the table" << aliasMap[tableName] << endl;
            cout << "OR" << endl;
            cout << "The attribute " << attName << " is attached to the wrong table" << endl;

            return false;
        }

        return true;
    }

	~Identifier () {}
};

class MinusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	MinusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "- (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

    bool isValid(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        return lhs->isValid(myCatalog, aliasMap) && rhs->isValid(myCatalog, aliasMap);
    }

	~MinusOp () {}
};

class PlusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	PlusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "+ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

    bool isValid(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        return lhs->isValid(myCatalog, aliasMap) && rhs->isValid(myCatalog, aliasMap);
    }

	~PlusOp () {}
};

class TimesOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	TimesOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "* (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

    bool isValid(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        return lhs->isValid(myCatalog, aliasMap) && rhs->isValid(myCatalog, aliasMap);
    }

	~TimesOp () {}
};

class DivideOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	DivideOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "/ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

    bool isValid(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        return lhs->isValid(myCatalog, aliasMap) && rhs->isValid(myCatalog, aliasMap);
    }

	~DivideOp () {}
};

class GtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	GtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "> (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

    bool isValid(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        return lhs->isValid(myCatalog, aliasMap) && rhs->isValid(myCatalog, aliasMap);
    }

	~GtOp () {}
};

class LtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	LtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "< (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

    bool isValid(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        return lhs->isValid(myCatalog, aliasMap) && rhs->isValid(myCatalog, aliasMap);
    }

	~LtOp () {}
};

class NeqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	NeqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "!= (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

    bool isValid(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        return lhs->isValid(myCatalog, aliasMap) && rhs->isValid(myCatalog, aliasMap);
    }

	~NeqOp () {}
};

class OrOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	OrOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "|| (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

    bool isValid(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        return lhs->isValid(myCatalog, aliasMap) && rhs->isValid(myCatalog, aliasMap);
    }

	~OrOp () {}
};

class EqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	EqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "== (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

    bool isValid(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        return lhs->isValid(myCatalog, aliasMap) && rhs->isValid(myCatalog, aliasMap);
    }

	~EqOp () {}
};

class NotOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	NotOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "!(" + child->toString () + ")";
	}


    bool isValid(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        return child->isValid(myCatalog, aliasMap);
    }

	~NotOp () {}
};

class SumOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	SumOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "sum(" + child->toString () + ")";
	}

    bool isValid(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        return child->isValid(myCatalog, aliasMap);
    }

	~SumOp () {}
};

class AvgOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	AvgOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "avg(" + child->toString () + ")";
	}

    bool isValid(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        return child->isValid(myCatalog, aliasMap);
    }

	~AvgOp () {}
};

#endif
