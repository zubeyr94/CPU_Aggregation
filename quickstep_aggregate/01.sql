select
	tags,
	sum(aggr_attr)
from
	data_4
group by
	tags
;
