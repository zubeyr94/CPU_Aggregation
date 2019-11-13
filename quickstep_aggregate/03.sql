select
	tags,
	sum(aggr_attr)
from
	data_16
group by
	tags
;
