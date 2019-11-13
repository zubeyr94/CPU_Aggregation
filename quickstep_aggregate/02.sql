select
	tags,
	sum(aggr_attr)
from
	data_8
group by
	tags
;
