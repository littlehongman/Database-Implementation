__Important: If using scons to build and the recordUnitTest failed from TEST 3, make sure the supplier.tbl is in the bin directory.__

Problem: 
1. PageReadWriter 每次被 init 都會重置 pageOverlay <br/>
    => hasNext() 一定會 return false