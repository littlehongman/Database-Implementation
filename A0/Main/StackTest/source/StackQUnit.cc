
#ifndef STACK_TEST_CC
#define STACK_TEST_CC

#include "QUnit.h"
#include "Stack.h"
#include <memory>
#include <iostream>
#include <vector>

int main () {

	QUnit::UnitTest qunit(std :: cerr, QUnit :: verbose);
	
	// UNIT TEST 1
	// just make sure we can create an empty stack, and that it is empty
	{
		
		Stack <int> myStack;
		QUNIT_IS_EQUAL (myStack.isEmpty (), true);

	}

	// UNIT TEST 2
	// make sure we can push/pop a bunch of items
	{

		Stack <int> myStack;
		for (int i = 0; i < 10; i++) {
			myStack.push (i);
		}

		// now, pop everything off
		for (int i = 0; i < 10; i++) {
			QUNIT_IS_EQUAL (myStack.pop (), 9 - i);
		}
	}

	// UNIT TEST 3
	// make sure that the destructor works correctly... unless the stack correctly deallocates
	// all of the items it contains, this test will fail
	{
		static int temp = 0;
		struct Tester {
			Tester () {temp++;}
			~Tester () {temp--;}
		};

		{
			Stack <std :: shared_ptr <Tester>> myStack;
			std :: shared_ptr <Tester> myGuy = std :: make_shared <Tester> ();
			for (int i = 0; i < 10; i++) {
				myStack.push (myGuy);
			}
		}

		QUNIT_IS_EQUAL (temp, 0);
	}

	// UNIT TEST 4
	// make sure we can push/pop a bunch of items and then do it again
	{

		Stack <int> myStack;
		for (int i = 0; i < 10; i++) {
			myStack.push (i);
		}

		for (int i = 0; i < 5; i++) {
			QUNIT_IS_EQUAL (myStack.pop (), 9 - i);
		}

		for (int i = 5; i < 20; i++) {
			myStack.push (i);
		}

		for (int i = 0; i < 20; i++) {
			QUNIT_IS_EQUAL (myStack.pop (), 19 - i);
		}
		
		QUNIT_IS_EQUAL (myStack.isEmpty (), true);
	}
    // UNIT TEST 5
    // test with other data type
    {
        Stack <std :: string> myStack;
        std :: string items[] = {"test0", "test1", "test2"};

        for (int i = 0; i < 3; i++) {
            myStack.push(items[i]);
        }

        for (int i = 0; i < 3; i++) {
            std :: string output = "test" + std ::to_string(2 - i);

            QUNIT_IS_EQUAL (myStack.pop(), output);
        }

    }

    {
        Stack <double > myStack;
        double nums[] = {0.1, 1.1, 2.1};

        for (int i = 0; i < 3; i++) {
            myStack.push(nums[i]);
        }

        for (int i = 0; i < 2; i++) {
            double output = 0.1 + (double) (2 - i);

            QUNIT_IS_EQUAL (myStack.pop(), output);
        }

        QUNIT_IS_EQUAL (myStack.isEmpty(), false);
    }

    // UNIT TEST 6
    // Test with pointers

    {
        Stack <int*> myStack;
        int a = 3, b = 2;
        int *p_a = &a, *p_b = &b;


        myStack.push(p_a);
        myStack.push(p_b);

        QUNIT_IS_EQUAL (*myStack.pop(), 2);
        QUNIT_IS_EQUAL (*myStack.pop(), 3);


        QUNIT_IS_EQUAL (myStack.isEmpty(), true);
    }

    // UNIT TEST 7
    // Test with vectors

    {
        Stack <std :: vector <int>> myStack;
        std :: vector <int> a = {3,1,2}, b = {1,2,3};

        myStack.push(a);
        myStack.push(b);

        QUNIT_IS_EQUAL (myStack.pop()[0], 1);
        QUNIT_IS_EQUAL (myStack.pop()[2], 2);


        QUNIT_IS_EQUAL (myStack.isEmpty(), true);
    }
}

#endif

