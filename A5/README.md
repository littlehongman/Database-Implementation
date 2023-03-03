### Group Member
#### mh121 Ming-Hsuan Hsieh
#### hw75 Hung-Chieh Wu



### Possible mismatch
#### 1. Compare Type (eq, neq, gt, lt)
1. int, string
2. double, string 

#### 2. Operation Type (minus, plus, times, div)

#### 3. Aggregation Type (sum, avg)


### Aggregate Limitation
1. Cannot use aggregate function in WHERE clause, GROUP BY clause.
2. In an aggregate query, the selected attributes (not aggregate functions) must be in the GROUP BY clause.


### Possible wrong
1. Cannot only literals in WHERE clause, GROUP BY clause.
