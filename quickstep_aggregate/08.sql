select
	tags,
	sum(aggr_attr)
from
	data_512
group by
	tags
;
