select
	tags,
	sum(aggr_attr)
from
	data_64
group by
	tags
;
