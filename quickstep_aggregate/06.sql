select
	tags,
	sum(aggr_attr)
from
	data_128
group by
	tags
;
