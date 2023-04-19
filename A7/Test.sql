select
    ps.ps_partkey
from
    part as p,
    partsupp as ps
where
    (p.p_partkey = ps.ps_partkey);


SELECT
    l.l_extendedprice
FROM
    customer AS c,
    orders AS o
WHERE
    (c.c_custkey = o.o_custkey);
