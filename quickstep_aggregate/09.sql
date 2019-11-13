select
	tags,
	sum(aggr_attr)
from
	data_1024
group by
	tags
;
