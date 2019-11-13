select
	tags,
	sum(aggr_attr)
from
	data_256
group by
	tags
;
