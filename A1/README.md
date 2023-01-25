## Unsolved
1. How to update LRU by page pointer <br>
   (if not pinned, how to update LRU order)


## UnFinished
1. Destructor of BufferManager
2. How to deal with anonymous page
   1. tempFile
   2. Use cases


## LRU
1. unordered_map<Page*, Node*>
2. update(Page* pagePtr) -> void <br>
   1. if pagePtr is not in LRU, insert it <br>
   2. if pagePtr is in LRU, move it to the MRU <br>
   
3. getEvictPage() -> return Page* <br> 
   1. return LRU, and remove it from LRU <br>

4. removeNode(Page* pagePtr) -> void <br> (If page is pinned, then remove it from LRU) <br>
   1. remove pagePtr from LRU <br>