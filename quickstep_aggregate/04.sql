select
	tags,
	sum(aggr_attr)
from
	data_32
group by
	tags
;
