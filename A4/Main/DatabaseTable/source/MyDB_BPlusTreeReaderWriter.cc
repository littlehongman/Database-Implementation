
#ifndef BPLUS_C
#define BPLUS_C

#include "MyDB_INRecord.h"
#include "MyDB_BPlusTreeReaderWriter.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_PageListIteratorSelfSortingAlt.h"
#include "RecordComparator.h"
#include <queue>          // std::queue

MyDB_BPlusTreeReaderWriter :: MyDB_BPlusTreeReaderWriter (string orderOnAttName, MyDB_TablePtr forMe, 
	MyDB_BufferManagerPtr myBuffer) : MyDB_TableReaderWriter (forMe, myBuffer) {

    MyDB_TableReaderWriter (forMe, myBuffer);
	// find the ordering attribute
	auto res = forMe->getSchema ()->getAttByName (orderOnAttName);

	// remember information about the ordering attribute
	orderingAttType = res.second;
	whichAttIsOrdering = res.first; // the ith attribute of each record

	// and the root location
	rootLocation = getTable ()->getRootLocation (); // Default value is -1
}

MyDB_RecordIteratorAltPtr MyDB_BPlusTreeReaderWriter :: getSortedRangeIteratorAlt (MyDB_AttValPtr low, MyDB_AttValPtr high) {
    vector<MyDB_PageReaderWriter> InRangePages;

    discoverPages(rootLocation, InRangePages, low, high);

    // recordPtrs & comparator used to get sorted pages result
    MyDB_RecordPtr lhs = getEmptyRecord(), rhs = getEmptyRecord();
    function <bool ()> comparator = buildComparator(lhs, rhs);

    // recordPtrs & comparators used to check if records is in range
    MyDB_INRecordPtr lowPtr = getINRecord(low), highPtr = getINRecord(high);
    MyDB_RecordPtr currPtr = getEmptyRecord();

    function <bool ()> lowComparator = buildComparator(currPtr, lowPtr);
    function <bool ()> highComparator = buildComparator(highPtr, currPtr);



	return make_shared<MyDB_PageListIteratorSelfSortingAlt>(InRangePages, lhs, rhs, comparator, currPtr, lowComparator, highComparator,
                                                            true);
}

MyDB_RecordIteratorAltPtr MyDB_BPlusTreeReaderWriter :: getRangeIteratorAlt (MyDB_AttValPtr low, MyDB_AttValPtr high) {
    vector<MyDB_PageReaderWriter> InRangePages;

    discoverPages(rootLocation, InRangePages, low, high);

    // recordPtrs & comparator used to get sorted pages result
    MyDB_RecordPtr lhs = getEmptyRecord(), rhs = getEmptyRecord();
    function <bool ()> comparator = buildComparator(lhs, rhs);

    // recordPtrs & comparators used to check if records is in range
    MyDB_INRecordPtr lowPtr = getINRecord(low), highPtr = getINRecord(high);
    MyDB_RecordPtr currPtr = getEmptyRecord();

    function <bool ()> lowComparator = buildComparator(currPtr, lowPtr);
    function <bool ()> highComparator = buildComparator(highPtr, currPtr);


    return make_shared<MyDB_PageListIteratorSelfSortingAlt>(InRangePages, lhs, rhs, comparator, currPtr, lowComparator, highComparator,
                                                            false);

}


bool MyDB_BPlusTreeReaderWriter :: discoverPages (int whichPage, vector <MyDB_PageReaderWriter> &list, MyDB_AttValPtr low, MyDB_AttValPtr high) {
	// Call recursively to push pages that are in the range
    MyDB_PageReaderWriter currPage = operator[](whichPage);

    if (currPage.getType() == MyDB_PageType::RegularPage){
        list.push_back(currPage);

        return true;
    }

    // Range find algorithm:
    // if currKey <= low -> go next on the iterator
    // else if currKey > high -> descend to subtree => discover(subTreeId, list, low, high)
    // else (low < currKey <= high), split the range = (1) discover(subTreeId, list, low, currKey)  (2) go next on the iterator
    bool keepTraverse = true;
    // 1. Create an iterator to iterate key value
    MyDB_INRecordPtr keyPtr = getINRecord();
    MyDB_RecordIteratorPtr currPageIter = currPage.getIterator(keyPtr);

    // 2. Create multiple recordPtrs & comparators
    MyDB_INRecordPtr lowPtr = getINRecord(low);
    MyDB_INRecordPtr highPtr = getINRecord(high);

    function <bool ()> lowComparator = buildComparator(lowPtr, keyPtr);
    function <bool ()> highComparator = buildComparator(highPtr, keyPtr);

    // lowComparator: low < key => True
    // highComparator: high < key => True

    // 3. Use while loop to find the right spot
    while (currPageIter->hasNext()) {
        currPageIter->getNext();

        if (!lowComparator()){ // key <= low
            continue;
        }
        else if (highComparator()){ // high < key
            discoverPages(keyPtr->getPtr(), list, low, high);

            return false; // We don't want to go further
        }
        else { // Split the range
            // Lower half descend
            keepTraverse = discoverPages(keyPtr->getPtr(), list, low, keyPtr->getKey());

            // Upper half keep traverse
            lowPtr->setKey(keyPtr->getKey());

        }

        if (!keepTraverse){
            return false;
        }

    }

    return false;
}

void MyDB_BPlusTreeReaderWriter :: append (MyDB_RecordPtr appendMe) {
    // rely on the append () private helper function (see below) on the root of the tree.
    // Note that if the helper function indicates that a split has happened then this method needs to handle this by creating a new root that contains pointers to both the old root and the result of the split.

    if (rootLocation == -1){ // If the rootLocation of B+- tree is -1, then the tree is empty
        // ---------- Internal Page ------------
        // Create an internal page
        MyDB_PageReaderWriter newInPage = operator[](0);
        newInPage.clear(); // always clear the page first
        newInPage.setType(MyDB_PageType::DirectoryPage);


        // Set the new root location
        rootLocation = 0;
        getTable ()->setRootLocation (0);

        // Insert an INRecord into newInPage
        MyDB_INRecordPtr newInRecordPtr = getINRecord();
        // No need to set key here, the newInRecord key is default set to inf => Easies to build tree
//        newInRecord->setKey(getKey(appendMe));

        // Pointer the new INRecord to the actual page, where corresponding data is stored
        newInRecordPtr->setPtr(1);

        newInPage.append(newInRecordPtr);

        // ------------ Leaf Page (Data node) ---------------
        // Create a leaf page
        MyDB_PageReaderWriter newLeafPage = operator[](1);
        newLeafPage.clear(); // always clear the page first
        newLeafPage.setType(MyDB_PageType::RegularPage); // first clear, then set Page

        // Insert the actual record into leaf page
        newLeafPage.append(appendMe);
    }

    else { // Start inserting records with the private methods (insert, split)
        // If the page whichPage splits due to the insertion
        // (a split can occur at an internal node level or at a leaf node level)
        // then the append () method returns a new MyDB_INRecordPtr object that points to an appropriate internal node record.
        MyDB_RecordPtr newTopPtr = append(rootLocation, appendMe);

        if (newTopPtr == nullptr){
            return;
        }

        // TODO: Creating a new root that contains pointers to both the old root and the result of the split.
        // If the root can fit new recordPtr => split the root page
        else{
            MyDB_INRecordPtr newInRecordPtr = getINRecord(); // new max key
            int newPageId = getNumPages();

            // Create a new root page
            MyDB_PageReaderWriter newRootPage = operator[](newPageId);
            newRootPage.clear();
            newRootPage.setType(MyDB_PageType::DirectoryPage);

            // assign the old root location as its ptr
            newInRecordPtr->setPtr(rootLocation);

            //  Append the two recordPtr
            newRootPage.append(newTopPtr);
            newRootPage.append(newInRecordPtr);

            // Set the new root location
            rootLocation = newPageId;
            getTable ()->setRootLocation (newPageId);
        }

    }
}

MyDB_RecordPtr MyDB_BPlusTreeReaderWriter :: split (MyDB_PageReaderWriter splitMe, MyDB_RecordPtr andMe) {
    MyDB_INRecordPtr newInRecordPtr = getINRecord();
    int newPageId = getNumPages();
    // Check the type of the page
    // (1) Create correct recordPtr type to sort
    MyDB_RecordPtr lhs, rhs;

    // (2) Create correct recordPtr type to iterate
    MyDB_RecordPtr tempRecordPtr;

    // (3) Create new Page with correct type
    MyDB_PageReaderWriter newPage = operator[](newPageId);
    newPage.clear();

    if (splitMe.getType() == MyDB_PageType::DirectoryPage){
        lhs = getINRecord();
        rhs = getINRecord();

        tempRecordPtr = getINRecord();

        newPage.setType(MyDB_PageType::DirectoryPage);
    }
    else {
        lhs = getEmptyRecord();
        rhs = getEmptyRecord();

        tempRecordPtr = getEmptyRecord();

        newPage.setType(MyDB_PageType::RegularPage);
    }

    auto comparator = buildComparator(lhs, rhs);

    // 1. Sort the page in-place
    splitMe.sortInPlace(comparator, lhs, rhs);

    // 2. Split the page into two leaves (BIG STEP)
    // (1) Count how many records is in the page, and how many is smaller than "andMe"
    unsigned int totalCount = 1, newLargerThan = 0; // totalCount = 1 because we have a new page
    MyDB_RecordIteratorPtr splitPageIter = splitMe.getIterator(tempRecordPtr);
    comparator = buildComparator(andMe, tempRecordPtr);

    // (2) Collect the positions of all records
    // --> Use the same trick in PageReaderWriter (but using iterator)
    // Read in the positions of all the records

    queue<void *> positions;

    while (splitPageIter->hasNext()){
        // Get all the memory position of the records
        positions.push(splitPageIter->getCurrentPointer());

        totalCount ++;

        // First getNext, then you can compare
        splitPageIter->getNext();

        if (!comparator()){
            newLargerThan ++;
        }
    }

    // (3) Create a new page and copy the content over
    unsigned int i = 0, median = totalCount / 2;

    // Construct the new page (smaller half without median)
    while (i < median){
        if (i == newLargerThan) { // If reach the "andMe" (as in the sorted order)
            newPage.append(andMe);

        }
        else { // else, take a record from old page
            tempRecordPtr->fromBinary(positions.front());
            positions.pop();

            newPage.append(tempRecordPtr);
        }
        i++;
    }



    // Construct the old page (larger half with median)
    auto oldPageType = splitMe.getType();

    // TODO: this will change pageType to regular page
    splitMe.clear();
    splitMe.setType(oldPageType);


    while (i < totalCount){
        if (i == newLargerThan) { // If reach the "andMe" (as in the sorted order)
            splitMe.append(andMe);
        }
        else { // else, take a record from old page
            tempRecordPtr->fromBinary(positions.front());
            positions.pop();

            splitMe.append(tempRecordPtr);
        }

        // IMPORTANT: get key of the median value
        if (i == median){
            if (i == newLargerThan)
                newInRecordPtr->setKey(getKey(andMe));
            else
                newInRecordPtr->setKey(getKey(tempRecordPtr));
        }

        i++;
    }

    // 3. Set pointer to the new InRecordPtr
    newInRecordPtr->setPtr(newPageId);


	return newInRecordPtr;
}

MyDB_RecordPtr MyDB_BPlusTreeReaderWriter :: append (int whichPage, MyDB_RecordPtr appendMe) {
    // First check what kind of page is whichPage
    // whichPage indicate the current page
    MyDB_PageReaderWriter rootPage = operator[](whichPage);

    MyDB_INRecordPtr maxPtr = getINRecord ();


    // ------------ Leaf Page ----------------
    // We can just append to the end. However, we still need to deal with potential split

    if (rootPage.getType() == MyDB_PageType::RegularPage){
        // Check if the record can fit in page
        if (rootPage.append(appendMe)){
            return nullptr;
        }
        // TODO: deal with split
        else {
            return split(rootPage, appendMe);
        }
    }

    // ------------ Internal Page ----------------
    // Called recursively to find the appropriate leaf page to insert

    // Create an empty recordPtr to iterate the key in the page
    MyDB_INRecordPtr tempRecordPtr = getINRecord();
    MyDB_RecordIteratorPtr rootPageIter = rootPage.getIterator(tempRecordPtr);

    // Create a comparator to find the right spot to insert
    // If return true, then the tempRecordPtr is smaller
    auto comparator = buildComparator(appendMe, tempRecordPtr);

    // Use while loop to find the first spot where tempRecordPtr has key larger than appendMe
    // TODO: Here might have problem
    rootPageIter->getNext();
    while (!comparator() && rootPageIter->hasNext()) {
        rootPageIter->getNext();
    }

    // Now we find the spot // TODO: or the page has no records
    // If we are in the internal page => go down
    // If we are in the leaf page => append record
    if (rootPage.getType() == MyDB_PageType::DirectoryPage){
        MyDB_RecordPtr newInRecordPtr = append(tempRecordPtr->getPtr(), appendMe);

        if (newInRecordPtr == nullptr)
            return nullptr;

        // TODO: might have recursive problem
        if (rootPage.append(newInRecordPtr)){
            comparator = buildComparator(newInRecordPtr, maxPtr);

            rootPage.sortInPlace(comparator, newInRecordPtr, maxPtr);

            return nullptr;
        }
        else{
            // call recursive to line 259
            return split(rootPage, newInRecordPtr);
        }

    }


	return nullptr;
}

MyDB_INRecordPtr MyDB_BPlusTreeReaderWriter :: getINRecord () {
	return make_shared <MyDB_INRecord> (orderingAttType->createAttMax ());
}


MyDB_INRecordPtr MyDB_BPlusTreeReaderWriter :: getINRecord (const MyDB_AttValPtr& val) {
    return make_shared <MyDB_INRecord> (val);
}


void MyDB_BPlusTreeReaderWriter :: printTree () {
    printTree (rootLocation, 0);
}


void MyDB_BPlusTreeReaderWriter::printTree(int whichPage, int depth) {
    MyDB_PageReaderWriter pageToPrint = (*this)[whichPage];
//    if (whichPage == rootLocation)
//        cout << "Root: ";
    // print out a leaf page
    if (pageToPrint.getType () == MyDB_PageType :: RegularPage) {
        MyDB_RecordPtr myRec = getEmptyRecord ();
        MyDB_RecordIteratorAltPtr temp = pageToPrint.getIteratorAlt ();
        while (temp->advance ()) {

            temp->getCurrent (myRec);
            for (int i = 0; i < depth; i++)
                cout << "\t";
            cout << myRec << "leaf " << "\n";
        }

        // print out a directory page
    } else {

        MyDB_INRecordPtr myRec = getINRecord ();
        MyDB_RecordIteratorAltPtr temp = pageToPrint.getIteratorAlt ();
        while (temp->advance ()) {

            temp->getCurrent (myRec);
            printTree (myRec->getPtr (), depth + 1);
            for (int i = 0; i < depth; i++)
                cout << "\t";
            cout << (MyDB_RecordPtr) myRec <<"not leaf" << "\n";
        }
    }
}

MyDB_AttValPtr MyDB_BPlusTreeReaderWriter :: getKey (MyDB_RecordPtr fromMe) {

	// in this case, got an IN record
	if (fromMe->getSchema () == nullptr) 
		return fromMe->getAtt (0)->getCopy ();

	// in this case, got a data record
	else 
		return fromMe->getAtt (whichAttIsOrdering)->getCopy ();
}

function <bool ()>  MyDB_BPlusTreeReaderWriter :: buildComparator (MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {

	MyDB_AttValPtr lhAtt, rhAtt;

	// in this case, the LHS is an IN record
	if (lhs->getSchema () == nullptr) {
		lhAtt = lhs->getAtt (0);	

	// here, it is a regular data record
	} else {
		lhAtt = lhs->getAtt (whichAttIsOrdering);
	}

	// in this case, the LHS is an IN record
	if (rhs->getSchema () == nullptr) {
		rhAtt = rhs->getAtt (0);	

	// here, it is a regular data record
	} else {
		rhAtt = rhs->getAtt (whichAttIsOrdering);
	}
	
	// now, build the comparison lambda and return
	if (orderingAttType->promotableToInt ()) {
		return [lhAtt, rhAtt] {return lhAtt->toInt () < rhAtt->toInt ();};
	} else if (orderingAttType->promotableToDouble ()) {
		return [lhAtt, rhAtt] {return lhAtt->toDouble () < rhAtt->toDouble ();};
	} else if (orderingAttType->promotableToString ()) {
		return [lhAtt, rhAtt] {return lhAtt->toString () < rhAtt->toString ();};
	} else {
		cout << "This is bad... cannot do anything with the >.\n";
		exit (1);
	}
}


#endif
