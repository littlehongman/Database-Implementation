select
    ps.ps_partkey
from
    part as p,
    partsupp as ps
where
    (p.p_partkey = ps.ps_partkey)
