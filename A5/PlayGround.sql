SELECT
    n.n_nationkey / 0.0
FROM
    nation AS n,
    region AS r
WHERE
  (n.n_regionkey = r.r_regionkey)
  AND (r.r_name = "Bay");


SELECT
    n.n_name
FROM
    nation AS n;


SELECT
    n.n_name
FROM
    customer AS c,
    orders AS o,
    lineitem AS l,
    nation AS n
WHERE
    c.c_custkey = 0

GROUP BY
    SUM(l.l_extendedprice);


SELECT
    n.n_name + n.n_name, SUM(n.n_nationkey)
FROM
    customer AS c,
    orders AS o,
    lineitem AS l,
    nation AS n
WHERE
        c.c_custkey = 0

GROUP BY
    n.n_name + n.n_name;


SELECT SUM(n.n_nationkey + r.r_regionkey)
FROM
    nation AS n,
    region AS r
WHERE n.n_regionkey = r.r_regionkey
GROUP BY n.n_name;


SELECT n.n_name + n.n_regionkey
FROM
    nation AS n,
    region AS r
WHERE n.n_regionkey = r.r_regionkey;


SELECT n.n_name, n.n_comment
FROM
    nation AS n,
    region AS r
WHERE n.n_regionkey = r.r_regionkey
GROUP BY n.n_name;