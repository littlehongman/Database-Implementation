SELECT
    n.n_name
FROM
    nation AS n,
    region AS r
WHERE
  (n.n_regionkey = r.r_regionkey)
  AND (r.r_name = "region");


SELECT
    n.n_name
FROM
    nation AS n;