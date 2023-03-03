
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

    // This will return the type of the expression
    virtual string getType(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap) = 0;

    virtual bool hasAggFunc() {
        return false;
    }

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

    string getType(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        return "bool";
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

    string getType(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        return "double";
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

    string getType(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        return "int";
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

    string getType(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        return "string";
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
            cout << "Error: The alias " << tableName << " does not exist in the table" << endl;
            return false;
        }

        string key = aliasMap[tableName] + "." + attName + "." + "type";
        string temp = "";

        // Check if the attributes exist in the corresponding table
        if (!myCatalog->getString(key, temp)){
            cout << "Error: The attribute " << attName << " does not exist in the table " << aliasMap[tableName] << endl;
            cout << "       OR" << endl;
            cout << "       The attribute " << attName << " is attached to the wrong table" << endl;

            return false;
        }

        return true;
    }

    string getType(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        string key = aliasMap[tableName] + "." + attName + "." + "type";
        string temp = "";

        myCatalog->getString(key, temp);

        return temp;
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

    string getType(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        string lhsType = lhs->getType(myCatalog, aliasMap);
        string rhsType = rhs->getType(myCatalog, aliasMap);

        if (lhsType == "int" && rhsType == "int"){
            return "int";
        }
        else if (lhsType == "double" && rhsType == "double"){
            return "double";
        }
        else if ((lhsType == "double" && rhsType == "int") || (lhsType == "int" && rhsType == "double")){
            return "double";
        }
        else {
            cout << "Error: Attempted to subtract values of type " << lhsType << " and type " << rhsType << endl;
            return "NULL";
        }
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

    string getType(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        string lhsType = lhs->getType(myCatalog, aliasMap);
        string rhsType = rhs->getType(myCatalog, aliasMap);

        if (lhsType == "int" && rhsType == "int"){
            return "int";
        }
        else if (lhsType == "double" && rhsType == "double"){
            return "double";
        }
        else if ((lhsType == "double" && rhsType == "int") || (lhsType == "int" && rhsType == "double")){
            return "double";
        }
        else if (lhsType == "string" && rhsType == "string"){ // String can be concatenated with string
            return "string";
        }
        else {
            cout << "Error: Attempted to add values of type " << lhsType << " and type " << rhsType << endl;
            return "NULL";
        }
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

    string getType(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        string lhsType = lhs->getType(myCatalog, aliasMap);
        string rhsType = rhs->getType(myCatalog, aliasMap);

        if (lhsType == "int" && rhsType == "int"){
            return "int";
        }
        else if (lhsType == "double" && rhsType == "double"){
            return "double";
        }
        else if ((lhsType == "double" && rhsType == "int") || (lhsType == "int" && rhsType == "double")){
            return "double";
        }
        else {
            cout << "Error: Attempted to multiply values of type " << lhsType << " and type " << rhsType << endl;
            return "NULL";
        }
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

    string getType(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        string lhsType = lhs->getType(myCatalog, aliasMap);
        string rhsType = rhs->getType(myCatalog, aliasMap);

        if (lhsType == "int" && rhsType == "int"){
            return "int";
        }
        else if (lhsType == "double" && rhsType == "double"){
            return "double";
        }
        else if ((lhsType == "double" && rhsType == "int") || (lhsType == "int" && rhsType == "double")){
            return "double";
        }
        else {
            cout << "Error: Attempted to divide values of type " << lhsType << " and type " << rhsType << endl;
            return "NULL";
        }
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

    string getType(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        string lhsType = lhs->getType(myCatalog, aliasMap);
        string rhsType = rhs->getType(myCatalog, aliasMap);

        if ((lhsType == "int" && rhsType == "int") || (lhsType == "double" && rhsType == "double") ||
            (lhsType == "double" && rhsType == "int") || (lhsType == "int" && rhsType == "double") ||
            (lhsType == "string" && rhsType == "string")) {
            return "gt";
        }
        else {
            cout << "Error: Attempted to perform 'greater than' comparison on values of type " << lhsType << " and type " << rhsType << endl;
            return "NULL";
        }
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

    string getType(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        string lhsType = lhs->getType(myCatalog, aliasMap);
        string rhsType = rhs->getType(myCatalog, aliasMap);

        if ((lhsType == "int" && rhsType == "int") || (lhsType == "double" && rhsType == "double") ||
            (lhsType == "double" && rhsType == "int") || (lhsType == "int" && rhsType == "double") ||
            (lhsType == "string" && rhsType == "string")) {
            return "lt";
        }
        else {
            cout << "Error: Attempted to perform 'less than' comparison on values of type " << lhsType << " and type " << rhsType << endl;
            return "NULL";
        }
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

    string getType(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        string lhsType = lhs->getType(myCatalog, aliasMap);
        string rhsType = rhs->getType(myCatalog, aliasMap);

        if ((lhsType == "int" && rhsType == "int") || (lhsType == "double" && rhsType == "double") ||
            (lhsType == "double" && rhsType == "int") || (lhsType == "int" && rhsType == "double") ||
            (lhsType == "string" && rhsType == "string")) {
            return "neq";
        }
        else {
            cout << "Error: Attempted to perform 'not equal' comparison on values of type " << lhsType << " and type " << rhsType << endl;
            return "NULL";
        }
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

    string getType(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        string lhsType = lhs->getType(myCatalog, aliasMap);
        string rhsType = rhs->getType(myCatalog, aliasMap);

        if (lhsType == "NULL" || rhsType == "NULL"){
            return "NULL";
        }
        else {
            return "or";
        }
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

    string getType(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        string lhsType = lhs->getType(myCatalog, aliasMap);
        string rhsType = rhs->getType(myCatalog, aliasMap);

        if ((lhsType == "int" && rhsType == "int") || (lhsType == "double" && rhsType == "double") ||
            (lhsType == "double" && rhsType == "int") || (lhsType == "int" && rhsType == "double") ||
            (lhsType == "string" && rhsType == "string")) {
            return "eq";
        }
        else {
            cout << "Error: Attempted to perform 'equal' comparison on values of type " << lhsType << " and type " << rhsType << endl;
            return "NULL";
        }
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

    string getType(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        string childType = child->getType(myCatalog, aliasMap);

        if (childType == "NULL"){
            return "NULL";
        }
        else {
            return "or";
        }
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

    string getType(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        string childType = child->getType(myCatalog, aliasMap);

        if (childType == "NULL"){
            return "NULL";
        }
        else {
            return "sum";
        }
    }

    bool hasAggFunc() override {
        return true;
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

    string getType(MyDB_CatalogPtr myCatalog, unordered_map<string, string> &aliasMap){
        string childType = child->getType(myCatalog, aliasMap);

        if (childType == "NULL"){
            return "NULL";
        }
        else {
            return "avg";
        }
    }

    bool hasAggFunc() override {
        return true;
    }

	~AvgOp () {}
};

#endif
